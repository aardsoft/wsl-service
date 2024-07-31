#ifndef _WSL_LAUNCHER_H
#define _WSL_LAUNCHER_H

/** @file wsl-launcher.h
 * Helper functions for interfacing with WSL
 */

#include <windef.h>

#include "wsl-templates.h"

extern PROCESS_INFORMATION processInformation;
extern STARTUPINFO startupInfo;

typedef struct _WslInstance WslInstance;

struct _WslInstance{
    LPCWSTR distributionName;
    LPCWSTR command;
    ULONG uid;
    BOOL useCurrentWorkingDirectory;
    HANDLE stdIn;
    HANDLE stdOut;
    HANDLE stdErr;
    HANDLE* process;
};

/** Start a service inside of WSL
 *
 * @param instanceData instance configuration
 * @return a handle to the process running a non-interactive WSL instance
 */
HANDLE startWslService(WslInstance *instanceData);
/** Start an interactive service inside of WSL
 *
 * Typically using an interactive call in a service should be avoided, though
 * it is useful for interactive tools.
 *
 * @param instanceData instance configuration
 * @return a handle to the thread running an interactive WSL instance
 */
HANDLE startWslServiceInteractive(WslInstance *instanceData);
/** A convenience wrapper around startWslServiceThreadInteractive, taking
 * 8-bit strings as argument, and converting it to wide.
 *
 * If any of serviceName or distributionName is not NULL it'll be converted
 * to a wide string, and stuffed into instanceData.
 *
 * @param serviceName the name of the service. May only be NULL if instanceData.command is set
 * @param distributionName the name of the distribution. May always be NULL
 * @return a handle to the thread running an interactive WSL instance
 *
 */
HANDLE startWslServiceInteractiveA(LPCSTR serviceName, LPCSTR distributionName, WslInstance *instanceData);
/** This implements the WSL service thread (i.e., a Windows thread wrapper
 * around the service launched on WSL side)
 *
 * This function probably never needs to be called directly, but is used by
 * the startWslServiceThreadInteractive functions.
 */
void wslServiceThreadInteractive(WslInstance *instanceData);
LPWSTR defaultWslDistributionName();
/** Set the default UID
 *
 * @param uid the UID to set
 * @param save save the current UID
 * @return the old UID
 */
ULONG wslSetUid(ULONG uid, BOOL save);
/** Set the default UID to 0, and save UID for later restoration
 *
 * This function tries to guard against being called multiple times
 * with the UID being 0, but can't guard against abuse with other UIDs.
 */
void wslSetRootUid();
/** Restore a UID saved by wslSetRootUid
 */
void wslRestoreUid();
/** Convert a ASCII string to wide
 *
 * It's up to you to free the strings later.
 *
 * @param input the ASCII string to convert
 * @param output a pointer to the wide string to receive the converted string
 * @return -1 on error, otherwise the length of the string
 */
int wslAtoW(LPCSTR input, LPWSTR *output);

#endif
