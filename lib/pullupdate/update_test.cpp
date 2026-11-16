#ifdef DEBUG_COOK_CMDS
#include "update_test.h"
#include "update_check.h"

// Thin wrapper over the shared chain. True only if the chain staged or verified the image.
bool runUpdateTest(const String &manifestUrl, const String &sigUrl, bool commit) {
  UpdateStatus st = runUpdateChain(manifestUrl, sigUrl, commit);
  return st == UPDATE_STAGED || st == UPDATE_VERIFIED;
}

bool runBenchUpdate(const char *tag, bool commit) {
  String base = String("https://github.com/cwgstreet/cloudSmoker/releases/download/") + tag;
  return runUpdateTest(base + "/manifest.txt", base + "/manifest.sig", commit);
}
#endif
