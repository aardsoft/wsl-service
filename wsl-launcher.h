#ifndef _WSL_LAUNCHER_H
#define _WSL_LAUNCHER_H

/** @file wsl-launcher.h
 * Helper functions for interfacing with WSL
 */

#include <windef.h>

#include "wsl-templates.h"

extern PROCESS_INFORMATION processInformation;
extern STARTUPINFO startupInfo;

/** Start a service inside of WSL
 *
 * @param serviceName the name of a service. Must be configured on Linux side
 * @param uid the UID to use for the service
 */
HANDLE startWslServiceThreadInteractive(LPWSTR serviceName, ULONG uid);
/** A convenience wrapper around startWslServiceThreadInteractive, taking an
 * 8-bit string as argument, and converting it to wide.
 */
HANDLE startWslServiceThreadInteractiveA(LPCSTR serviceName, ULONG uid);
/** This implements the WSL service thread (i.e., a Windows thread wrapper
 * around the service launched on WSL side)
 *
 * This function probably never needs to be called directly, but is used by
 * the startWslServiceThreadInteractive functions.
 */
void wslServiceThreadInteractive(WCHAR* serviceName);
LPWSTR defaultWslDistributionName();
/** Set the default UID
 *
 * @param uid the UID to set
 * @return the old UID
 */
ULONG wslSetUid(ULONG uid);
/** Set the default UID to 0, and save UID for later restoration
 *
 * This function tries to guard against being called multiple times
 * with the UID being 0, but can't guard against abuse with other UIDs.
 */
void wslSetRootUid();
/** Restore a UID saved by wslSetRootUid
 */
void wslRestoreUid();


#endif
