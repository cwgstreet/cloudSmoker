/* ***************************************************************
 * cookstore.h - library to manage cook state persistence and time sync
 *       1) Cook state persistence (NVS namespace "cook") and time sync.
 *       2) Writes are kept rare: targets and flags are written on events, the charge count and
 *        3) lastSeen only every 5 min, and only when the count has moved 1 mAh or 30 min has
 * passed.
 *
 *  C W Greenstreet, Ver1, 5Nov2026
 *    GPLv3 Licence
 *
 ** ************************************************************* */

#ifndef COOKSTORE_H
#define COOKSTORE_H

#include <stdint.h>

enum CookBoot_t { COOK_NONE, COOK_RESUME, COOK_RESUME_UNVERIFIED, COOK_STALE };

// Open the NVS namespace. Call once in setup(), after calInit().
void cookStoreBegin();

// Start SNTP and wait up to timeoutMs for a valid time (UTC). Call only when Wi-Fi is connected.
bool timeSyncWait(uint32_t timeoutMs);

// true once SNTP has set the clock
bool clockValid();

// true while a cook is recorded in NVS
bool cookStoreActive();

// Save a new cook at Start Cook. Clears the done and sent flags.

// Save the done and sent flags, packed into one byte. Writes only when they change.
void cookSaveFlags(uint8_t flags);

// Periodic checkpoint. Call once per loop() pass with the current charge count.
// Also retries SNTP once a minute while online and the clock is not set.
void cookCheckpoint(float charge_mAh);

// Mark the cook as finished. Its targets stay stored as launch pad defaults.
void cookSaveEnd();

// Read the stored cook. Returns false if none is stored.

// Boot decision for an interrupted cook: resume, resume with unverified time, or finished (stale).
CookBoot_t cookBootCheck();

void cookSaveStart(const float meatT[3], float pitMin_F, float pitMax_F, bool degC, float charge_mAh);
bool cookLoad(float meatT[3], float& pitMin_F, float& pitMax_F, bool& degC, float& charge_mAh, uint8_t& flags);

// Power bank: capacity is a device setting (kept across cooks); the usable base is stored per cook
float bankCapacityLoad();
void bankCapacitySave(float mAh);
void cookSaveBank(float usable_mAh);
float cookLoadBank();
uint8_t bankChargeLoad();
void bankChargeSave(uint8_t pct);

void cookForceCheckpoint(float charge_mAh);
float cookLastSeenAgeHours();  // hours since lastSeen, or -1 if unknown
bool cookSetLastSeenAgeHours(float hours);  // debug: move stored lastSeen back
#endif  // COOKSTORE_H