#pragma once
// Outcome of one update check. UPDATE_STAGED means a newer release was verified and downloaded
// into the inactive slot, and the caller should restart to apply it.
enum UpdateStatus {
  UPDATE_UP_TO_DATE,
  UPDATE_STAGED,
  UPDATE_COOK_ACTIVE,
  UPDATE_NO_WIFI,
  UPDATE_NO_CLOCK,
  UPDATE_FAILED
};

UpdateStatus runUpdateCheck();
void updateCheckAtBoot();
