/* ***************************************************************
 * signature_check.cpp - verifies the ECDSA signature on a release manifest
 *
 * C W Greenstreet, cloudSmoker2, 14Nov26
 * Licence: GPLv3 (see manifest_parser.h)
 *************************************************************** */

#include "signature_check.h"

#include <cstring>

#include "mbedtls/pk.h"
#include "mbedtls/sha256.h"

bool verifyManifestSignature(const unsigned char* manifest, size_t manifestLen,
                             const unsigned char* sig, size_t sigLen,
                             const char* publicKeyPem) {
  if (manifest == nullptr || sig == nullptr || publicKeyPem == nullptr || manifestLen == 0 ||
      sigLen == 0 || sigLen > 100) {
    return false;
  }

  // Hash the exact downloaded bytes, before anything parses them.
  unsigned char hash[32];
  if (mbedtls_sha256_ret(manifest, manifestLen, hash, 0) != 0) {
    return false;
  }

  mbedtls_pk_context ctx;
  mbedtls_pk_init(&ctx);

  bool valid = false;
  // strlen + 1 includes the terminating zero, which mbedTLS requires for PEM input.
  if (mbedtls_pk_parse_public_key(&ctx, (const unsigned char*)publicKeyPem,
                                  strlen(publicKeyPem) + 1) == 0 &&
      mbedtls_pk_verify(&ctx, MBEDTLS_MD_SHA256, hash, sizeof(hash), sig, sigLen) == 0) {
    valid = true;
  }

  mbedtls_pk_free(&ctx);  // runs on both success and failure
  return valid;
}
