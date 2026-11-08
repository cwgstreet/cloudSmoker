#include "bleserial.h"

#include <NimBLEDevice.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ---------- Output ring buffer, shared by the USB tee and the BLE pump ----------
static const size_t RING_SIZE = 2048;
static uint8_t ring[RING_SIZE];
static size_t ringHead = 0;   // next write position
static size_t ringTail = 0;   // next read position
static size_t ringCount = 0;  // bytes stored
static uint32_t droppedBytes = 0;
static bool teeOn = false;
static portMUX_TYPE ringMux = portMUX_INITIALIZER_UNLOCKED;

BleTee Log;

size_t BleTee::write(uint8_t c) {
  Serial.write(c);  // USB always
  if (!teeOn) {
    return 1;
  }
  portENTER_CRITICAL(&ringMux);
  if (ringCount == RING_SIZE) {  // full: drop the oldest byte and count it
    ringTail = (ringTail + 1) % RING_SIZE;
    ringCount--;
    droppedBytes++;
  }
  ring[ringHead] = c;
  ringHead = (ringHead + 1) % RING_SIZE;
  ringCount++;
  portEXIT_CRITICAL(&ringMux);
  return 1;
}

size_t BleTee::write(const uint8_t* buf, size_t n) {
  for (size_t i = 0; i < n; i++) {
    write(buf[i]);
  }
  return n;
}

size_t BleTee::printf(const char* fmt, ...) {
  char buf[160];
  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  if (n <= 0) {
    return 0;
  }
  if ((size_t)n >= sizeof(buf)) {
    n = sizeof(buf) - 1;
  }
  return write((const uint8_t*)buf, (size_t)n);
}

void BleTee::setEnabled(bool on) {
  portENTER_CRITICAL(&ringMux);
  teeOn = on;
  if (!on) {
    ringHead = 0;
    ringTail = 0;
    ringCount = 0;
    droppedBytes = 0;
  }
  portEXIT_CRITICAL(&ringMux);
}

bool BleTee::enabled() const {
  return teeOn;
}

size_t BleTee::available() const {
  portENTER_CRITICAL(&ringMux);
  size_t n = ringCount;
  portEXIT_CRITICAL(&ringMux);
  return n;
}

size_t BleTee::take(uint8_t* out, size_t max) {
  size_t n = 0;
  portENTER_CRITICAL(&ringMux);
  while (n < max && ringCount > 0) {
    out[n++] = ring[ringTail];
    ringTail = (ringTail + 1) % RING_SIZE;
    ringCount--;
  }
  portEXIT_CRITICAL(&ringMux);
  return n;
}

uint32_t BleTee::takeDropped() {
  portENTER_CRITICAL(&ringMux);
  uint32_t d = droppedBytes;
  droppedBytes = 0;
  portEXIT_CRITICAL(&ringMux);
  return d;
}

// ---------- BLE link: Nordic UART Service, one piece in flight at a time ----------
static const char* NUS_SERVICE = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* NUS_RX      = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* NUS_TX      = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

static const size_t PIECE_BYTES = 20;            // fits the default BLE MTU of 23
static const uint32_t STATUS_TIMEOUT_MS = 2000;  // resend a piece if no status callback

static bool bleInited = false;
static NimBLECharacteristic* txChar = nullptr;
static uint8_t linkCount = 0;
static bool inFlight = false;
static uint32_t inFlightSinceMs = 0;
static uint8_t pending[PIECE_BYTES];
static size_t pendingLen = 0;

class TxCallbacks : public NimBLECharacteristicCallbacks {
  void onStatus(NimBLECharacteristic* c, int code) override {
    if (code == 0) {
      pendingLen = 0;  // piece confirmed by the stack
    }
    inFlight = false;
  }
};

class RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c, NimBLEConnInfo& info) override {
    // Phone input is handled in the next step
  }
};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* server, NimBLEConnInfo& info) override {
    linkCount++;
    if (linkCount < 2) {
      NimBLEDevice::startAdvertising();  // stay discoverable for a second app
    }
  }
  void onDisconnect(NimBLEServer* server, NimBLEConnInfo& info, int reason) override {
    if (linkCount > 0) {
      linkCount--;
    }
    if (linkCount == 0) {
      inFlight = false;
      pendingLen = 0;
    }
    if (Log.enabled()) {
      NimBLEDevice::startAdvertising();
    }
  }
};

void bleSerialStart() {
  if (bleInited) {
    return;
  }
  NimBLEDevice::init("cloudSmoker2");

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  NimBLEService* svc = server->createService(NUS_SERVICE);

  txChar = svc->createCharacteristic(NUS_TX, NIMBLE_PROPERTY::NOTIFY);
  txChar->setCallbacks(new TxCallbacks());

  NimBLECharacteristic* rxChar =
      svc->createCharacteristic(NUS_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  rxChar->setCallbacks(new RxCallbacks());

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(NUS_SERVICE);

  bleInited = true;
}

void bleSerialApply(bool on) {
  Log.setEnabled(on);
  if (on) {
    bleSerialStart();
    NimBLEDevice::startAdvertising();
  } else if (bleInited) {
    NimBLEDevice::getAdvertising()->stop();  // no new connections; an existing one ends when the phone does
  }
}

void bleSerialPump() {
  if (!bleInited || txChar == nullptr || linkCount == 0 || !Log.enabled()) {
    return;
  }

  if (inFlight) {
    if ((uint32_t)(millis() - inFlightSinceMs) < STATUS_TIMEOUT_MS) {
      return;
    }
    inFlight = false;  // no status in time: resend the same piece below
  }

  if (pendingLen == 0) {
    uint32_t dropped = Log.takeDropped();
    if (dropped > 0) {
      pendingLen = (size_t)snprintf((char*)pending, sizeof(pending), "[gap %lu]\r\n",
                                    (unsigned long)dropped);
    } else {
      pendingLen = Log.take(pending, sizeof(pending));
    }
    if (pendingLen == 0) {
      return;
    }
  }

  txChar->setValue(std::string((const char*)pending, pendingLen));
  inFlight = true;
  inFlightSinceMs = millis();
  if (!txChar->notify()) {
    inFlight = false;  // stays pending and is tried again on the next loop
  }
}
