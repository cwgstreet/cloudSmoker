/* ***************************************************************
 * update_test.h - bench commands for testing the update chain (debug builds only)
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Written with AI assistance (Claude). Under review by the author.
 * *************************************************************** */

#pragma once
#include <Arduino.h>

// Debug only (DEBUG_COOK_CMDS). Runs the signed update chain against a release.
// commit=false is a dry run: everything is verified, then discarded.
bool runUpdateTest(const String &manifestUrl, const String &sigUrl, bool commit);
bool runBenchUpdate(const char *tag, bool commit);
