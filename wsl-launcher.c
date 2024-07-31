/** @file wsl-launcher.c
 */

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <combaseapi.h>
#include "wsl-launcher.h"
#include "wsl-log.h"
#include "wslapi.h"

PROCESS_INFORMATION processInformation = {0};
STARTUPINFO startupInfo                = {0};

WSL_DISTRIBUTION_FLAGS distributionFlags = 0;
ULONG defaultUID = 0;

void wslServiceThreadInteractive(WslInstance *instanceData){
  TCHAR wslExecutable[copyBufferSize];
  size_t maxSz = STRSAFE_MAX_CCH * sizeof(TCHAR);
  LPCWSTR formatString = TEXT(BASH_START_TEMPLATE);

  DWORD exitCode;
  PCWSTR defaultDistW = defaultWslDistributionName();

  if (copyBufferTCharSize > maxSz){
    wslLog(L_ERROR, L"Size %u is bigger than maximum allowed %u\n", copyBufferTCharSize, maxSz);
    return;
  }
  if (FAILED(StringCbPrintf(wslExecutable, copyBufferTCharSize, formatString, instanceData->command))){
    wslLog(L_ERROR, L"Unable to prepare WSL executable string\n");
    return;
  }

  WslLaunchInteractive(
    defaultDistW,
    wslExecutable,
    FALSE,
    &exitCode);
}

HANDLE startWslServiceInteractive(WslInstance *instanceData){
  //LPWSTR service = _wcsdup(serviceName);
  HANDLE hThread;
  DWORD threadId;

  if (instanceData->distributionName == NULL){
    LPWSTR defaultDist = defaultWslDistributionName();
    if (defaultDist == NULL){
      wslLogText(L_ERROR, L"Unable to determine default distribution name.\n");
      wslLogText(L_ERROR, L"Is WSL initialized?\n");
      return NULL;
    } else {
      wslLog(L_DEBUG, L"Default distribution is: %s\n", defaultDist);
      instanceData->distributionName = defaultDist;
    }
  }

  if (!WslIsDistributionRegistered(instanceData->distributionName)){
    wslLog(L_ERROR, L"Distribution '%s' is not registered\n", instanceData->distributionName);
    return NULL;
  } else {
    wslLog(L_DEBUG, L"Distribution '%s' is available\n", instanceData->distributionName);
  }

  wslSetUid(instanceData->uid, TRUE);
  hThread = CreateThread(NULL,0,
                         (LPTHREAD_START_ROUTINE)wslServiceThreadInteractive,
                         instanceData,0,&threadId);
  if (hThread == NULL){
    wslLog(L_ERROR, L"CreateThread() failed, error %u\n", GetLastError());
    wslRestoreUid();
    return NULL;
  } else {
    wslLogText(L_DEBUG, L"WSL launched\n");
    ///TODO: figure out a better way to delay resetting the UID
    Sleep(2000);
    wslRestoreUid();
  }

  return hThread;
}

HANDLE startWslServiceInteractiveA(LPCSTR serviceName, LPCSTR distributionName, WslInstance *instanceData){
  LPWSTR serviceNameW = NULL;
  LPWSTR distributionNameW = NULL;

  int r = -1;

  if (serviceName != NULL){
    r = wslAtoW(serviceName, &serviceNameW);
    if (r == -1)
      return NULL;
    else
      instanceData->command = serviceNameW;
  }

  if (distributionName != NULL){
    r = wslAtoW(distributionName, &distributionNameW);
    if (r == -1)
      return NULL;
    else
      instanceData->distributionName = distributionNameW;
  }

  HANDLE handle = startWslServiceInteractive(instanceData);
  //free(serviceNameW);
  return handle;
}

BOOL terminateWslProcess(){
  ///TODO
  return FALSE;
}

LPWSTR defaultWslDistributionName(){
  LSTATUS stat;
  LPWSTR regValue;
  DWORD regValueLength;
  LPCWSTR lxssKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Lxss\\";
  DWORD defaultDistKeyLength;
  LPWSTR defaultDistKey;

  // check the length of the key
  stat = RegGetValue(HKEY_CURRENT_USER,
                     lxssKey,
                     L"DefaultDistribution",
                     RRF_RT_REG_SZ, NULL,
                     NULL,
                     &regValueLength);

  if (stat != ERROR_SUCCESS)
    return NULL;

  regValue = (LPWSTR)malloc((regValueLength+1) * sizeof(wchar_t));
  // look up the UUID for the default distribution
  stat = RegGetValue(HKEY_CURRENT_USER,
                     lxssKey,
                     L"DefaultDistribution",
                     RRF_RT_REG_SZ, NULL,
                     (PVOID)regValue,
                     &regValueLength);

  if (stat != ERROR_SUCCESS){
    free(regValue);
    return NULL;
  }

  defaultDistKeyLength = lstrlen(lxssKey) + lstrlen(regValue) + 1;
  defaultDistKey = (LPWSTR)malloc(defaultDistKeyLength * sizeof(wchar_t));
  lstrcpyn(defaultDistKey, lxssKey, defaultDistKeyLength);
  StringCbCat(defaultDistKey, defaultDistKeyLength * sizeof(wchar_t), regValue);

  free(regValue);

  stat = RegGetValue(HKEY_CURRENT_USER,
                     defaultDistKey,
                     L"DistributionName",
                     RRF_RT_REG_SZ, NULL,
                     NULL,
                     &regValueLength);

  if (stat != ERROR_SUCCESS){
    free(defaultDistKey);
    return NULL;
  }

  regValue = (LPWSTR)malloc((regValueLength+1) * sizeof(wchar_t));

  stat = RegGetValue(HKEY_CURRENT_USER,
                     defaultDistKey,
                     L"DistributionName",
                     RRF_RT_REG_SZ, NULL,
                     (PVOID)regValue,
                     &regValueLength);

  free(defaultDistKey);

  if (stat != ERROR_SUCCESS){
    free(regValue);
    return NULL;
  }

  return regValue;
}

ULONG wslSetUid(ULONG uid, BOOL save){
  ULONG _ver, _uid;
  WSL_DISTRIBUTION_FLAGS _flags;
  PSTR *_env;
  ULONG _envCnt;

  PCWSTR defaultDistW = defaultWslDistributionName();

  WslGetDistributionConfiguration(defaultDistW,
                                  &_ver,
                                  &_uid,
                                  &_flags,
                                  &_env,
                                  &_envCnt);

  //CoTaskMemFree(_env);

  WslConfigureDistribution(defaultDistW, uid, _flags);
  wslLog(L_INFO, L"Setting UID to %i\n", uid);

  if (save && _uid != 0){
    defaultUID = _uid;
    distributionFlags = _flags;
  }

  return _uid;
}

void wslSetRootUid(){
  ULONG _ver, _uid;
  WSL_DISTRIBUTION_FLAGS _flags;
  PSTR *_env;
  ULONG _envCnt;

  PCWSTR defaultDistW = defaultWslDistributionName();

  WslGetDistributionConfiguration(defaultDistW,
                                  &_ver,
                                  &_uid,
                                  &_flags,
                                  &_env,
                                  &_envCnt);

  //CoTaskMemFree(_env);

  defaultUID = _uid;
  distributionFlags = _flags;

  WslConfigureDistribution(defaultDistW, 0, _flags);
  wslLogText(L_DEBUG, L"Setting UID to 0\n");
}

void wslRestoreUid(){
  // if the saved UID is 0 don't set it to reduce risk of race conditions
  if (defaultUID == 0 || distributionFlags == 0)
    return;

  PCWSTR defaultDistW = defaultWslDistributionName();
  WslConfigureDistribution(defaultDistW, defaultUID, distributionFlags);
  wslLog(L_INFO, L"Setting UID to %i\n", defaultUID);
}

/*
  LPWSTR defaultWslDistributionNameW(){
  LPWSTR defaultDistW;
  LPTSTR defaultDist = defaultWslDistributionName();

  if (defaultDist == NULL)
  return NULL;

  // check length of buffer to hold the wide string
  int len =
  MultiByteToWideChar(CP_ACP, 0, defaultDist, -1,
  defaultDistW, 0);

  if (len == 0){
  free(defaultDist);
  return NULL;
  }

  // now convert the string
  defaultDistW = (LPWSTR)malloc(len * sizeof(wchar_t));
  len =
  MultiByteToWideChar(CP_ACP, 0, defaultDist, -1,
  defaultDistW, len);

  if (len == 0){
  free(defaultDist);
  free(defaultDistW);
  return NULL;
  }

  return defaultDistW;
  }
*/

int wslAtoW(LPCSTR input, LPWSTR *output){
  if (input == NULL)
    return -1;

  int len = MultiByteToWideChar(CP_ACP, 0,
                                input, -1,
                                NULL, 0);

  if (len == 0){
    wslLogText(L_ERROR, L"Unable to get buffer length\n");
    return -1;
  }

  *output = (LPWSTR)malloc(len * sizeof(wchar_t));

  len = MultiByteToWideChar(CP_ACP, 0,
                            input, -1,
                            *output, len);

  if (len == 0){
    wslLogText(L_ERROR, L"Unable to convert argument to unicode\n");
    return -1;
  }

  return len;
}
