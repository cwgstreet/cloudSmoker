#pragma once
#include <Arduino.h>

// Debug only (DEBUG_COOK_CMDS). Runs the signed update chain against a release.
// commit=false is a dry run: everything is verified, then discarded.
bool runUpdateTest(const String &manifestUrl, const String &sigUrl, bool commit);
bool runBenchUpdate(const char *tag, bool commit);
