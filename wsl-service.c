/** @file wsl-service.c
 *
 * Currently just a quick and dirty proof of concept
 */

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>

#include "wsl-launcher.h"
#include "wsl-log.h"

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);
int InitService();

void main(){
  SERVICE_TABLE_ENTRY ServiceTable[2];
  ServiceTable[0].lpServiceName = L"WSL Service Monitor";
  ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

  ServiceTable[1].lpServiceName = NULL;
  ServiceTable[1].lpServiceProc = NULL;
  // Start the control dispatcher thread for our service
  StartServiceCtrlDispatcher(ServiceTable);
}

void ServiceMain(int argc, char** argv){
  int error;

  ServiceStatus.dwServiceType             = SERVICE_WIN32;
  ServiceStatus.dwCurrentState            = SERVICE_START_PENDING;
  ServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  ServiceStatus.dwWin32ExitCode           = 0;
  ServiceStatus.dwServiceSpecificExitCode = 0;
  ServiceStatus.dwCheckPoint              = 0;
  ServiceStatus.dwWaitHint                = 0;

  hStatus = RegisterServiceCtrlHandler(
    L"WslServiceMonitor",
    (LPHANDLER_FUNCTION)ControlHandler);
  if (hStatus == (SERVICE_STATUS_HANDLE)0){
    // Registering Control Handler failed
    return;
  }

  // Initialize Service
  error = InitService();
  if (error){
    // Initialization failed
    ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    ServiceStatus.dwWin32ExitCode      = -1;
    SetServiceStatus(hStatus, &ServiceStatus);
    return;
  }

  // We report the running status to SCM.
  ServiceStatus.dwCurrentState = SERVICE_RUNNING;
  SetServiceStatus (hStatus, &ServiceStatus);

  // TODO: check registry for additional arguments for the service name.
  //       without a registry key just start the service

  // now as long as this service is supposed to be running restart
  // the WSL process
  while (ServiceStatus.dwCurrentState == SERVICE_RUNNING){
    wslSetRootUid();
    // TODO: use non-interactive method here
    HANDLE hThread = startWslServiceThreadInteractive(L"service", 0);
    Sleep(2000);
  }
  return;
}

// Service initialization
int InitService(){
  int result;
  wslLogText(L_INFO, TEXT("Monitoring started."));
  return(result);
}


// Control handler function
void ControlHandler(DWORD request){
  switch(request){
    case SERVICE_CONTROL_STOP:
      wslLogText(L_INFO, TEXT("Monitoring stopped."));

      ServiceStatus.dwWin32ExitCode = 0;
      ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
      SetServiceStatus (hStatus, &ServiceStatus);
      return;

    default:
      break;
  }

  // Report current status
  SetServiceStatus (hStatus,  &ServiceStatus);

  return;
}
