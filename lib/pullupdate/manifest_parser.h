/* ***************************************************************
 * manifest_parser.h - cloudSmoker manifest parser for Github Over the Air updates
 *
 * C W Greenstreet, cloudSmoker2, 14Nov26
 * Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 * distribution must also share the full source code under this same licence.)
 *
 *************************************************************** */

#ifndef MANIFEST_PARSER_H
#define MANIFEST_PARSER_H

#include <cstddef>
#include <cstdint>

// Values read from a release manifest. Strings are NUL-terminated.
struct ManifestFields {
  uint32_t version;
  uint32_t imageSize;
  char versionName[17];  // up to 16 characters
  char imageName[65];    // up to 64 characters
  char imageSha256[65];  // exactly 64 lowercase hex characters
  char imageUrl[201];    // up to 200 characters
  char keyId[17];        // exactly 16 lowercase hex characters
};

// Returns true only if every line is present, in order, and valid.
// On false, out is left unchanged.
bool parseManifest(const char* text, size_t length, const char* expectedKeyId, ManifestFields& out);

#endif  // MANIFEST_PARSER_H
