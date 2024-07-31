/** @file wsl-log.c
 */

#include <windows.h>
#include <strsafe.h>
#include <stdarg.h>

#include "wsl-log.h"
#include "wsl-service-events.h"

#define EVENT_SOURCE L"WslService"

BOOL wslLogInitialized = FALSE;
BOOL wslLogInteractive = TRUE;

int wslLogLevels[WSL_LOGFACILITY_COUNT];

LPWSTR wslLogFile = NULL;

// just using big fixed-size buffers for string copy operations is a bit dirty,
// but makes things a bit easier. Doesn't really matter for that kind of small
// program we're doing here.
int const copyBufferSize = 40000;
size_t copyBufferTCharSize = copyBufferSize * sizeof(TCHAR);

// check if logging is initialized
// yes -> directly return
// no -> initialize and return
BOOL wslLogInit(){
  LPCWSTR regKey;

  if (wslLogInitialized == TRUE)
    return TRUE;

  if (wslLogInteractive == TRUE){
    regKey = L"SOFTWARE\\WslService\\Interactive";
  } else {
    regKey = L"SOFTWARE\\WslService\\Non-Interactive";
  }

  ///TODO: initialise keys with sane defaults if they don't exist
  LPCWSTR regValues[WSL_LOGFACILITY_COUNT];
  regValues[F_CONSOLE] = L"LogLevelConsole";
  regValues[F_FILE] = L"LogLevelFile";
  regValues[F_EVENTLOG] = L"LogLevelEventLog";

  //
  for (int cnt=0;cnt<WSL_LOGFACILITY_COUNT;cnt++){
    LSTATUS stat;
    DWORD regValueLength;
    DWORD regValue;

    // initialize facility to disabled in case something goes wrong later on
    wslLogLevels[cnt] = L_DISABLED;

    stat = RegGetValue(HKEY_LOCAL_MACHINE,
                       regKey,
                       regValues[cnt],
                       RRF_RT_REG_DWORD, NULL,
                       NULL,
                       &regValueLength);

    // something wrong with the key -> just bail out, and hope for better luck
    // with the next one
    if (stat != ERROR_SUCCESS){
      wprintf(L"Unable to determine size for %s (%d): %d\n", regValues[cnt], cnt, stat);
      continue;
    }

    stat = RegGetValue(HKEY_LOCAL_MACHINE,
                       regKey,
                       regValues[cnt],
                       RRF_RT_REG_DWORD, NULL,
                       (LPBYTE)&regValue,
                       &regValueLength);


    if (stat != ERROR_SUCCESS){
      wprintf(L"Unable to read key for %s (%d): %d\n", regValues[cnt], cnt, stat);
      continue;
    }
  }

  wslLogInitialized = TRUE;
  return wslLogInitialized;
}

void setWslLogInteractive(BOOL logType){
  wslLogInit();

  wslLogInteractive = logType;
}

void setWslLogFile(LPCWSTR logFile){
  wslLogInit();

  wslLogFile = _wcsdup(logFile);
}

void setWslLogLevel(int facility, int level){
  // would be rather pointless to set loglevels on an uninitialized logging
  // system, which would override them on init
  wslLogInit();

  wslLogLevels[facility] = level;
}

void wslLogEventLog(int level, LPCWSTR logMessage){
  HANDLE hEventSource;
  LPCWSTR lpszStrings[2];
  TCHAR Buffer[80];
  DWORD wType, wCategory;

  hEventSource = RegisterEventSource(NULL, EVENT_SOURCE);

  if (level <= L_ERROR){
    wType = EVENTLOG_ERROR_TYPE;
    wCategory = WL_ERROR;
  } else if (level == L_WARNING) {
    wType = EVENTLOG_WARNING_TYPE;
    wCategory = WL_WARNING;
  } else {
    wType = EVENTLOG_INFORMATION_TYPE;
    wCategory = WL_INFO;
  }

  if( NULL != hEventSource ){
    StringCchPrintf(Buffer, 80, TEXT("%s\nLast system error code: %d"), logMessage, GetLastError());

    lpszStrings[0] = EVENT_SOURCE;
    lpszStrings[1] = Buffer;

    ReportEvent(hEventSource,
                wType,
                0,                   // event category
                wCategory,           // event identifier
                NULL,                // no security identifier
                2,                   // size of lpszStrings array
                0,                   // no binary data
                lpszStrings,         // array of strings
                NULL);               // no binary data

    DeregisterEventSource(hEventSource);
  }
}

FILE* wslOpenLog(){
  if (wslLogFile == NULL)
    return NULL;

  FILE* log;
  log = _wfopen(wslLogFile, L"a+");
  return log;
}

void wslCloseLog(FILE* log){
  fclose(log);
}

void wslLogText(int level, LPCWSTR logMessage){
  wslLogInit();

  if (level <= wslLogLevels[F_EVENTLOG])
    wslLogEventLog(level, logMessage);

  if (level <= wslLogLevels[F_CONSOLE])
    wprintf(logMessage);

  if (level <= wslLogLevels[F_FILE]){
    FILE* fp = wslOpenLog();
    if (fp != NULL){
      fwprintf(fp, logMessage);
      wslCloseLog(fp);
    }
  }
}

void wslLog(int level, LPCWSTR logFormat, ...){
  wslLogInit();

  va_list args;
  va_start(args, logFormat);

  if (level <= wslLogLevels[F_EVENTLOG]){
    TCHAR logEntry[copyBufferSize];

    if (FAILED(StringCbPrintf(logEntry, copyBufferTCharSize, logFormat, args))){
      // hopefully that one will get through...
      wslLog(L_ERROR, L"Unable to prepare string for logging\n");
    } else {
      wslLogEventLog(level, logEntry);
    }
  }

  if (level <= wslLogLevels[F_CONSOLE])
    vwprintf(logFormat, args);

  if (level <= wslLogLevels[F_FILE]){
    FILE* fp = wslOpenLog();
    if (fp != NULL){
      vfwprintf(fp, logFormat, args);
      wslCloseLog(fp);
    }
  }
  va_end(args);
}
