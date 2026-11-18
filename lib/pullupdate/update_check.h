/* ***************************************************************
 * update_check.h - boot and Settings update check: fetches the stable manifest, runs the chain
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Written with AI assistance (Claude) with review by the author.
 * *************************************************************** */

#pragma once
#include <Arduino.h>

// Outcome of one update chain. UPDATE_VERIFIED is a dry run that passed and was discarded.
enum UpdateStatus {
  UPDATE_UP_TO_DATE,
  UPDATE_STAGED,
  UPDATE_VERIFIED,
  UPDATE_COOK_ACTIVE,
  UPDATE_NO_WIFI,
  UPDATE_NO_CLOCK,
  UPDATE_FAILED
};

// The full signed chain against any manifest and signature URL. commit=false verifies the
// image and discards it. commit=true refuses a release that isn't newer, then stages it.
UpdateStatus runUpdateChain(const String& manifestUrl, const String& sigUrl, bool commit);

// The official check, using the release tag this build was compiled for.
UpdateStatus runUpdateCheck();

// Boot-time check. Restarts into the new image only if one was staged.
void updateCheckAtBoot();
