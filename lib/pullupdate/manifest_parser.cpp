/* ***************************************************************
 * manifest_parser.cpp - parses the line-based release manifest
 *
 * C W Greenstreet, cloudSmoker2, 14Nov26
 * Licence: GPLv3 (see manifest_parser.h)
 *
 * The signature is checked on the raw bytes before this runs. The parser
 * accepts only the exact format and refuses anything else.
 *************************************************************** */

#include "manifest_parser.h"

#include <cstring>

namespace {

constexpr size_t kMaxFileBytes = 1024;
constexpr size_t kMaxLineBytes = 200;
constexpr size_t kFieldCount = 10;

// Fixed order. A missing, extra, or misplaced line makes the parse fail.
const char* const kKeys[kFieldCount] = {
    "schema", "product", "board", "version", "version_name",
    "image_name", "image_size", "image_sha256", "image_url", "key_id"};

const char kUrlPrefix[] = "https://github.com/cwgstreet/cloudSmoker/releases/download/";

bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isLowerHex(char c) { return isDigit(c) || (c >= 'a' && c <= 'f'); }
bool isLowerName(char c) { return (c >= 'a' && c <= 'z') || isDigit(c) || c == '.' || c == '-'; }
bool isVersionNameChar(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || isDigit(c) || c == '.' || c == '-';
}

// True if every one of the n characters passes the check.
bool allOf(const char* s, size_t n, bool (*ok)(char)) {
  for (size_t i = 0; i < n; i++) {
    if (!ok(s[i])) return false;
  }
  return true;
}

// True if the value is exactly the literal text.
bool matches(const char* value, size_t valueLen, const char* literal) {
  size_t n = strlen(literal);
  return valueLen == n && memcmp(value, literal, n) == 0;
}

// Reads 1 to maxDigits decimal digits. Zero is refused.
bool parseNumber(const char* value, size_t valueLen, size_t maxDigits, uint32_t& out) {
  if (valueLen == 0 || valueLen > maxDigits) return false;
  uint32_t result = 0;
  for (size_t i = 0; i < valueLen; i++) {
    if (!isDigit(value[i])) return false;
    result = result * 10 + (uint32_t)(value[i] - '0');
  }
  if (result == 0) return false;
  out = result;
  return true;
}

// Copies the value into dst, which must hold valueLen characters plus the terminator.
bool copyValue(char* dst, size_t dstSize, const char* value, size_t valueLen) {
  if (valueLen == 0 || valueLen + 1 > dstSize) return false;
  memcpy(dst, value, valueLen);
  dst[valueLen] = '\0';
  return true;
}

// Checks one "key=value" line against the field expected at this position.
bool handleLine(const char* line, size_t lineLen, size_t field,
                const char* expectedKeyId, ManifestFields& f) {
  if (memchr(line, ' ', lineLen) != nullptr) return false;  // no spaces anywhere

  const char* key = kKeys[field];
  size_t keyLen = strlen(key);
  if (lineLen < keyLen + 2) return false;  // key, '=', then at least one value character
  if (memcmp(line, key, keyLen) != 0 || line[keyLen] != '=') return false;

  const char* value = line + keyLen + 1;
  size_t valueLen = lineLen - keyLen - 1;

  switch (field) {
    case 0:  // schema
      return matches(value, valueLen, "1");
    case 1:  // product
      return matches(value, valueLen, "cloudSmoker2");
    case 2:  // board
      return matches(value, valueLen, "cloudsmoker2-pcb");
    case 3:  // version
      return parseNumber(value, valueLen, 5, f.version);
    case 4:  // version_name
      return allOf(value, valueLen, isVersionNameChar) &&
             copyValue(f.versionName, sizeof(f.versionName), value, valueLen);
    case 5:  // image_name
      return allOf(value, valueLen, isLowerName) &&
             copyValue(f.imageName, sizeof(f.imageName), value, valueLen);
    case 6:  // image_size
      return parseNumber(value, valueLen, 7, f.imageSize);
    case 7:  // image_sha256
      return valueLen == 64 && allOf(value, valueLen, isLowerHex) &&
             copyValue(f.imageSha256, sizeof(f.imageSha256), value, valueLen);
    case 8: {  // image_url
      size_t prefixLen = sizeof(kUrlPrefix) - 1;
      return valueLen > prefixLen && memcmp(value, kUrlPrefix, prefixLen) == 0 &&
             copyValue(f.imageUrl, sizeof(f.imageUrl), value, valueLen);
    }
    case 9:  // key_id
      return valueLen == 16 && allOf(value, valueLen, isLowerHex) &&
             matches(value, valueLen, expectedKeyId) &&
             copyValue(f.keyId, sizeof(f.keyId), value, valueLen);
    default:
      return false;
  }
}

}  // namespace

bool parseManifest(const char* text, size_t length, const char* expectedKeyId, ManifestFields& out) {
  if (text == nullptr || expectedKeyId == nullptr) return false;
  if (length == 0 || length > kMaxFileBytes) return false;
  if (text[length - 1] != '\n') return false;  // the last line must end with a newline

  ManifestFields fields{};  // zeroed; copied to out only if every line passes
  size_t pos = 0;
  for (size_t field = 0; field < kFieldCount; field++) {
    size_t end = pos;
    while (end < length && text[end] != '\n') {
      if (text[end] == '\r') return false;  // no CRLF line endings
      end++;
    }
    if (end >= length) return false;  // this line has no newline
    size_t lineLen = end - pos;
    if (lineLen > kMaxLineBytes) return false;
    if (!handleLine(text + pos, lineLen, field, expectedKeyId, fields)) return false;
    pos = end + 1;
  }
  if (pos != length) return false;  // extra bytes after the last expected line

  out = fields;
  return true;
}
