/* ***************************************************************
 * update_test.cpp - bench wrappers around the update chain (built only with DEBUG_COOK_CMDS)
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Written with AI assistance (Claude). Under review by the author.
 * *************************************************************** */

#ifdef DEBUG_COOK_CMDS
#include "update_test.h"
#include "update_check.h"
#include "release_source.h"

// Thin wrapper over the shared chain. True only if the chain staged or verified the image.
bool runUpdateTest(const String &manifestUrl, const String &sigUrl, bool commit) {
  UpdateStatus st = runUpdateChain(manifestUrl, sigUrl, commit);
  return st == UPDATE_STAGED || st == UPDATE_VERIFIED;
}

bool runBenchUpdate(const char *tag, bool commit) {
  String base = String(RELEASE_URL_PREFIX) + tag;
  return runUpdateTest(base + "/manifest.txt", base + "/manifest.sig", commit);
}
#endif
