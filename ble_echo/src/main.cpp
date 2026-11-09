#include <Arduino.h>
#include <NimBLEDevice.h>
#include "nvs.h"
#include <deque>
#include <string>

// Nordic UART Service: the phone writes to RX, the device notifies on TX
static const char* NUS_SERVICE = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* NUS_RX      = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* NUS_TX      = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

static const uint32_t STATUS_TIMEOUT_MS = 2000;  // resend a piece if no status callback arrives

static NimBLECharacteristic* txChar = nullptr;
static uint8_t linkCount = 0;                  // number of connected apps
static bool inFlight = false;                  // a piece has been notified, status not yet seen
static uint32_t inFlightSinceMs = 0;
static std::deque<std::string> outQueue;       // 20-byte pieces waiting to go out, in order
static uint32_t statusCount = 0;
static uint32_t timeoutCount = 0;

// Splits text into 20-byte pieces and queues them. Callbacks only queue; loop() sends.
static void queueOutput(const std::string& data) {
  const size_t chunk = 20;
  for (size_t i = 0; i < data.size(); i += chunk) {
    outQueue.push_back(data.substr(i, chunk));
  }
}

// Called from loop(): sends the front piece when nothing is in flight.
// inFlight is set BEFORE notify(), because the status callback can fire inside notify().
static void pumpOutput() {
  if (linkCount == 0 || txChar == nullptr) {
    return;
  }

  if (inFlight) {
    if ((uint32_t)(millis() - inFlightSinceMs) < STATUS_TIMEOUT_MS) {
      return;
    }
    // No status callback in time: clear the flag and resend the same piece
    timeoutCount++;
    inFlight = false;
    Serial.print(F("BLE status timeout, resending piece (total "));
    Serial.print(timeoutCount);
    Serial.println(F(")"));
  }

  if (outQueue.empty()) {
    return;
  }

  txChar->setValue(outQueue.front());
  inFlight = true;
  inFlightSinceMs = millis();
  if (!txChar->notify()) {
    inFlight = false;  // not sent: the piece stays at the front and is tried again next loop
  }
}

// Every status callback is logged. Any callback means the stack has finished with the front piece.
class TxCallbacks : public NimBLECharacteristicCallbacks {
  void onStatus(NimBLECharacteristic* c, int code) override {
    statusCount++;
    Serial.print(F("BLE status #"));
    Serial.print(statusCount);
    Serial.print(F(" code "));
    Serial.print(code);
    Serial.print(F(", queue "));
    Serial.println((unsigned)outQueue.size());
    if (inFlight && !outQueue.empty()) {
      outQueue.pop_front();
    }
    inFlight = false;
  }
};

class RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c, NimBLEConnInfo& info) override {
    std::string in = c->getValue();
    Serial.print(F("BLE rx: "));
    Serial.println(in.c_str());

    if (in == "burst") {
      // 40 numbered pieces of exactly 20 bytes: any gap in the phone's list is a lost piece
      for (int n = 1; n <= 40; n++) {
        char piece[21];
        snprintf(piece, sizeof(piece), "#%02d 0123456789ABCDEF", n);
        outQueue.push_back(std::string(piece, 20));
      }
      Serial.println(F("burst queued"));
      return;
    }

    queueOutput("echo: " + in + "\r\n");
  }
};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* server, NimBLEConnInfo& info) override {
    linkCount++;
    Serial.print(F("BLE phone connected, links now "));
    Serial.println(linkCount);
    if (linkCount < 2) {
      NimBLEDevice::startAdvertising();  // stay discoverable so a second app can connect
    }
  }
  void onDisconnect(NimBLEServer* server, NimBLEConnInfo& info, int reason) override {
    if (linkCount > 0) {
      linkCount--;
    }
    Serial.print(F("BLE link dropped, links now "));
    Serial.println(linkCount);
    if (linkCount == 0) {
      inFlight = false;
      outQueue.clear();  // only clear when the last app has gone
    }
    NimBLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("BLE echo test starting"));

  NimBLEDevice::init("cloudSmoker2-test");
  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
  NimBLEDevice::setSecurityPasskey(378300);
  NimBLEDevice::setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID | BLE_SM_PAIR_KEY_DIST_SIGN);
  NimBLEDevice::setSecurityRespKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID | BLE_SM_PAIR_KEY_DIST_SIGN);

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  NimBLEService* svc = server->createService(NUS_SERVICE);

  txChar = svc->createCharacteristic(NUS_TX, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_ENC);
  txChar->setCallbacks(new TxCallbacks());

  NimBLECharacteristic* rxChar =
      svc->createCharacteristic(NUS_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::WRITE_ENC);
  rxChar->setCallbacks(new RxCallbacks());

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(NUS_SERVICE);
  adv->start();
  { nvs_stats_t st; nvs_get_stats(NULL, &st); Serial.printf("NVS used %u, free %u\n", (unsigned)st.used_entries, (unsigned)st.free_entries); }

  Serial.println(F("advertising as cloudSmoker2-test"));
}

void loop() {
  pumpOutput();
  delay(2);
}
