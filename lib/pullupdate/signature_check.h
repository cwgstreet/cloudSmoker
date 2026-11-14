/* ***************************************************************
 * signature_check.h - verifies the ECDSA signature on a release manifest
 *
 * C W Greenstreet, cloudSmoker2, 14Nov26
 * Licence: GPLv3 (see manifest_parser.h)
 *************************************************************** */

#ifndef SIGNATURE_CHECK_H
#define SIGNATURE_CHECK_H

#include <cstddef>

// Returns true only if sig is a valid ECDSA P-256 / SHA-256 signature of the
// exact manifest bytes, made with the private key that matches publicKeyPem.
bool verifyManifestSignature(const unsigned char* manifest, size_t manifestLen,
                             const unsigned char* sig, size_t sigLen,
                             const char* publicKeyPem);

#endif  // SIGNATURE_CHECK_H
