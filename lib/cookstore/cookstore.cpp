/* ***************************************************************
 * cookstore.cpp - library to manage cook state persistence and time sync
 *       1) Cook state persistence (NVS namespace "cook") and time sync.
 *       2) Writes are kept rare: targets and flags are written on events, the charge count and
 *        3) lastSeen only every 5 min, and only when the count has moved 1 mAh or 30 min has
 * passed.
 *
 *  C W Greenstreet, Ver1, 5Nov2026
 *    GPLv3 Licence
 *
 ** ************************************************************* */
#include "cookstore.h"

#include <Preferences.h>
#include <WiFi.h>
#include <math.h>
#include <time.h>

static Preferences cookPrefs;  // namespace "cook", separate from calibration's "cal"

static const uint32_t CHECKPOINT_MS = 5UL * 60UL * 1000UL;  // checkpoint every 5 min
static const uint32_t HEARTBEAT_MS = 30UL * 60UL * 1000UL;  // refresh lastSeen at least this often
static const uint32_t CLOCK_RETRY_MS = 60UL * 1000UL;       // retry SNTP while online and unset
static const float CHARGE_STEP_mAh = 1.0f;                  // write charge after this much change
static const uint32_t STALE_S = 12UL * 3600UL;              // 12 h: cook counts as finished
static const time_t MIN_VALID_EPOCH = 1700000000;           // earlier means no SNTP time yet

static bool activeCache = false;
static float lastWrittenCharge = -1.0f;
static uint32_t lastCheckpointMs = 0;
static uint32_t lastWriteMs = 0;
static uint32_t lastClockTryMs = 0;
static uint8_t lastWrittenFlags = 0xFF;  // impossible value, so the first flag save always writes

void cookStoreBegin() {
  cookPrefs.begin("cook", false);
  activeCache = cookPrefs.getBool("active", false);
}

bool clockValid() { return time(nullptr) > MIN_VALID_EPOCH; }

static void startSntp() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // UTC: only elapsed time matters
  lastClockTryMs = millis();
}

bool timeSyncWait(uint32_t timeoutMs) {
  if (clockValid()) {
    return true;
  }
  startSntp();
  uint32_t start = millis();
  while (!clockValid() && (uint32_t)(millis() - start) < timeoutMs) {
    delay(100);
  }
  return clockValid();
}

bool cookStoreActive() { return activeCache; }



void cookSaveFlags(uint8_t flags) {
  if (!activeCache || flags == lastWrittenFlags) {
    return;
  }
  cookPrefs.putUChar("flags", flags);
  lastWrittenFlags = flags;
}

void cookCheckpoint(float charge_mAh) {
  // Keep trying to get the time while online, so lastSeen can be kept current
  if (!clockValid() && WiFi.status() == WL_CONNECTED &&
      (uint32_t)(millis() - lastClockTryMs) >= CLOCK_RETRY_MS) {
    startSntp();
  }

  if (!activeCache) {
    return;
  }
  if ((uint32_t)(millis() - lastCheckpointMs) < CHECKPOINT_MS) {
    return;
  }
  lastCheckpointMs = millis();

  bool moved = fabs(charge_mAh - lastWrittenCharge) >= CHARGE_STEP_mAh;
  bool heartbeat = (uint32_t)(millis() - lastWriteMs) >= HEARTBEAT_MS;
  if (!moved && !heartbeat) {
    return;
  }

  cookPrefs.putFloat("charge", charge_mAh);
  if (clockValid()) {
    cookPrefs.putUInt("lastSeen", (uint32_t)time(nullptr));
  }
  lastWrittenCharge = charge_mAh;
  lastWriteMs = millis();
}

void cookSaveEnd() {
  cookPrefs.putBool("active", false);
  activeCache = false;
}

CookBoot_t cookBootCheck() {
  if (!activeCache) {
    return COOK_NONE;
  }
  if (!clockValid()) {
    return COOK_RESUME_UNVERIFIED;
  }
  uint32_t lastSeen = cookPrefs.getUInt("lastSeen", 0);
  if (lastSeen == 0) {
    return COOK_RESUME;  // cook started before any time was known
  }
  uint32_t now = (uint32_t)time(nullptr);
  if (now > lastSeen && (now - lastSeen) >= STALE_S) {
    return COOK_STALE;
  }
  return COOK_RESUME;
}
void cookSaveStart(const float meatT[3], float pitMin_F, float pitMax_F, bool degC,
                   float charge_mAh) {
  cookPrefs.putFloat("meat1T", meatT[0]);
  cookPrefs.putFloat("meat2T", meatT[1]);
  cookPrefs.putFloat("meat3T", meatT[2]);
  cookPrefs.putFloat("pitMin", pitMin_F);
  cookPrefs.putFloat("pitMax", pitMax_F);
  cookPrefs.putBool("degC", degC);
  cookPrefs.putFloat("charge", charge_mAh);
  cookPrefs.putUInt("lastSeen", clockValid() ? (uint32_t)time(nullptr) : 0);
  cookPrefs.putUChar("flags", 0);
  cookPrefs.putBool("active", true);  // written last, so a cut mid-write leaves no active cook

  activeCache = true;
  lastWrittenCharge = charge_mAh;
  lastWrittenFlags = 0;
  lastCheckpointMs = millis();
  lastWriteMs = millis();
}

bool cookLoad(float meatT[3], float& pitMin_F, float& pitMax_F, bool& degC, float& charge_mAh,
              uint8_t& flags) {
  if (!cookPrefs.isKey("meat1T")) {
    return false;
  }
  meatT[0] = cookPrefs.getFloat("meat1T", 203.0f);
  meatT[1] = cookPrefs.getFloat("meat2T", 203.0f);
  meatT[2] = cookPrefs.getFloat("meat3T", 203.0f);
  pitMin_F = cookPrefs.getFloat("pitMin", 225.0f);
  pitMax_F = cookPrefs.getFloat("pitMax", 250.0f);
  degC = cookPrefs.getBool("degC", false);
  charge_mAh = cookPrefs.getFloat("charge", 0.0f);
  flags = cookPrefs.getUChar("flags", 0);
  return true;
}

// Capacity from the box, in mAh. Defaults to 10,000 until the user changes it.
float bankCapacityLoad() { return cookPrefs.getFloat("bankCap", 10000.0f); }

void bankCapacitySave(float mAh) { cookPrefs.putFloat("bankCap", mAh); }

// Usable charge at the start of this cook, in mAh at 5 V
void cookSaveBank(float usable_mAh) { cookPrefs.putFloat("bankBase", usable_mAh); }

// Defaults to 10,000 mAh x 0.74 x 100% if nothing has been stored
float cookLoadBank() { return cookPrefs.getFloat("bankBase", 7400.0f); }

// Charge % of the power bank, kept until changed. Defaults to 100.
uint8_t bankChargeLoad() { return cookPrefs.getUChar("bankPct", 100); }

void bankChargeSave(uint8_t pct) { cookPrefs.putUChar("bankPct", pct); }

// Writes the charge count now, rather than at the next 5-minute checkpoint. Used after a bank reset.
void cookForceCheckpoint(float charge_mAh) {
  if (!activeCache) {
    return;
  }
  cookPrefs.putFloat("charge", charge_mAh);
  if (clockValid()) {
    cookPrefs.putUInt("lastSeen", (uint32_t)time(nullptr));
  }
  lastWrittenCharge = charge_mAh;
  lastWriteMs = millis();
  lastCheckpointMs = millis();
}

// Hours since the last saved timestamp. Returns -1 if the clock isn't set or no time was saved.
float cookLastSeenAgeHours() {
  if (!clockValid()) {
    return -1.0f;
  }
  uint32_t lastSeen = cookPrefs.getUInt("lastSeen", 0);
  if (lastSeen == 0) {
    return -1.0f;
  }
  uint32_t now = (uint32_t)time(nullptr);
  if (now < lastSeen) {
    return 0.0f;
  }
  return (float)(now - lastSeen) / 3600.0f;
}

// Debug: moves the stored lastSeen back by the given hours, to test the 12 h stale rule.
// Needs a stored cook and a valid clock. The next checkpoint rewrites lastSeen, so reset promptly.
bool cookSetLastSeenAgeHours(float hours) {
  if (!activeCache || !clockValid() || hours < 0.0f) {
    return false;
  }
  int64_t target = (int64_t)time(nullptr) - (int64_t)(hours * 3600.0f);
  if (target < 1) {
    return false;
  }
  cookPrefs.putUInt("lastSeen", (uint32_t)target);
  return true;
}

// Cool-down state: danger-zone start time per probe (0 = none, 1 = started while time not synced)
// and the alert bits. Saved only when a state changes.
void cookSaveCooldown(const uint32_t dangerAt[3], uint16_t bits) {
  if (!activeCache) {
    return;
  }
  cookPrefs.putUInt("dng1", dangerAt[0]);
  cookPrefs.putUInt("dng2", dangerAt[1]);
  cookPrefs.putUInt("dng3", dangerAt[2]);
  cookPrefs.putUShort("cool", bits);
}

bool cookLoadCooldown(uint32_t dangerAt[3], uint16_t& bits) {
  if (!cookPrefs.isKey("cool")) {
    return false;
  }
  dangerAt[0] = cookPrefs.getUInt("dng1", 0);
  dangerAt[1] = cookPrefs.getUInt("dng2", 0);
  dangerAt[2] = cookPrefs.getUInt("dng3", 0);
  bits = cookPrefs.getUShort("cool", 0);
  return true;
}

// BLE serial setting, kept across reboots. Defaults to on.
bool bleSerialLoad() { return cookPrefs.getBool("bleOn", true); }

void bleSerialSave(bool on) { cookPrefs.putBool("bleOn", on); }
