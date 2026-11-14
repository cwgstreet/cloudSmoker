/* ***************************************************************
 * rollback_guard.h - keeps a new firmware image pending until it proves healthy
 *
 * C W Greenstreet, cloudSmoker2, 14Nov26
 * Licence: GPLv3 (see manifest_parser.h)
 *************************************************************** */

#ifndef ROLLBACK_GUARD_H
#define ROLLBACK_GUARD_H

// Call at the end of setup(). bleStarted is true only if BLE came up.
void rollbackGuardBegin(bool bleStarted);

// Call from loop(). Confirms the image after two minutes, or rolls back.
void rollbackGuardLoop();

#endif  // ROLLBACK_GUARD_H
