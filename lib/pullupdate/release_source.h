/* ***************************************************************
 * release_source.h - where the board looks for firmware releases
 *
 *   C W Greenstreet, cloudSmoker2, 17Nov2026
 *   Licence: GPLv3 (Licensed under the GNU GPLv3: Free to use and modify, but any public
 *     distribution must also share the full source code under this same license.)
 *
 *   Only this URL prefix is accepted for update manifests and images. A fork that publishes
 *   its own releases must change RELEASE_URL_PREFIX here and also change the public key in
 *   release_keys.h.
 * *************************************************************** */

#ifndef RELEASE_SOURCE_H
#define RELEASE_SOURCE_H

// Start of every release download URL. Forks change this one line.
constexpr char RELEASE_URL_PREFIX[] =
    "https://github.com/cwgstreet/cloudSmoker/releases/download/";

#endif  // end header guard
