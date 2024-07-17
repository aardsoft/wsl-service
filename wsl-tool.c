#include <stdio.h>
#include <windows.h>
#include <getopt.h>
#include <popt.h>
#include "wslapi.h"
#include "wsl-launcher.h"
#include "wsl-log.h"

int ctrl_count = 0;

enum WSL_ACTION{
  WSL_NONE,
  WSL_RUN,
  WSL_STOP,
  WSL_EXEC,
  WSL_SETUID,
  WSL_IMPORT
};

/*
  WSL.exe help output:
  Usage: wsl.exe [Argument] [Options...] [CommandLine]

  Arguments to run Linux binaries:

  If no command line is provided, wsl.exe launches the default shell.

  --exec, -e <CommandLine>
  Execute the specified command without using the default Linux shell.

  --
  Pass the remaining command line as is.

  Options:
  --distribution, -d <DistributionName>
  Run the specified distribution.

  --user, -u <UserName>
  Run as the specified user.

  Arguments to manage Windows Subsystem for Linux:

  --export <DistributionName> <FileName>
  Exports the distribution to a tar file.
  The filename can be - for standard output.

  --import <DistributionName> <InstallLocation> <FileName>
  Imports the specified tar file as a new distribution.
  The filename can be - for standard input.

  --list, -l [Options]
  Lists distributions.

  Options:
  --all
  List all distributions, including distributions that are currently
  being installed or uninstalled.

  --running
  List only distributions that are currently running.

  -setdefault, -s <DistributionName>
  Sets the distribution as the default.

  --terminate, -t <DistributionName>
  Terminates the distribution.

  --unregister <DistributionName>
  Unregisters the distribution.

  --upgrade <DistributionName>
  Upgrades the distribution to the WslFs file system format.

  --help
  Display usage information.

*/

int exitHandler(){
  printf("Exit handler\n");
}

BOOL WINAPI consoleHandler(DWORD signal){
  if (signal == CTRL_C_EVENT){
    printf("Caught ctrl-c\n");
    ctrl_count++;
  }
  if (signal == CTRL_BREAK_EVENT){
    printf("Caught ctrl-break\n");
  }

  if (ctrl_count > 1)
    exit(1);

  return TRUE;
}

void errDuplicateActions(){
  wprintf(L"Only a single action is allowed\n");
  exit(1);
}

int main(int argc, char** argv){
  LPWSTR serviceName;
  char ch;
  int action = WSL_NONE;
  poptContext optCon;

  int optDefaultUid;
  LPCWSTR optDistribution = NULL;
  LPCWSTR optDistributionTar = NULL;
  LPCWSTR optService = NULL;

  // distribution variables
  ULONG distVer, distUid;
  WSL_DISTRIBUTION_FLAGS distFlags;
  PSTR *distEnv;
  ULONG distEnvCnt;

  _onexit(exitHandler);

  static struct option long_options[] =
    {
      {"run", required_argument, NULL, 'r'},
      {"stop", required_argument, NULL, 's'},
      {NULL, 0, NULL, 0}
    };

  AllocConsole();
  if (!SetConsoleCtrlHandler(consoleHandler, TRUE)){
    wprintf(L"Unable to set control handler");
    return 1;
  }

  // wsl.exe options not implemented:
  // --upgrade
  // --list [--all --running]
  struct poptOption optionsTable[] = {
    { "exec", 'e', 0, 0, 'e',
      "Execute the command using the default Linux shell" },
    { "distribution", 'd', 0, 0, 'd',
      "Use the specified distribution" },
    { "user", 'u', 0, 0, 'u',
      "Run as the specified user" },
    { "import", '\0', 0, 0, 'P',
      "Import the distribution. Requires -d and -t" },
    { "export", '\0', 0, 0, 'X',
      "Export the distribution. Requires -d, -t and -p" },
    { "path", 'p', 0, 0, 'p',
      "The filesystem path to a distribution" },
    { "tar", 't', POPT_ARG_STRING, &optDistributionTar, 't',
      "The tar file for export/import" },
    { "run", 'r', POPT_ARG_STRING, &optService, 'r',
      "Run the service script" },
    { "stop", 's', POPT_ARG_STRING, &optService, 's',
      "Stop the service script" },
    { "setdefault", '\0', 0, 0, 'D',
      "Set the default distribution specified with -d" },
    { "terminate", '\0', 0, 0, 'T',
      "Terminate the distribution specified with -d" },
    { "unregister", '\0', 0, 0, 'U',
      "Unregister the distribution specified with -d" },
    { "shell", '\0', 0, 0, 'S',
      "Open a windows shell" },
    { "test-log", '\0', 0, 0, 'L',
      "Sent test events to Windows EventLog" },
    { "default-uid", '\0', POPT_ARG_INT, &optDefaultUid, 'I',
      "Set the default UID"},
    POPT_AUTOHELP
    { NULL, 0, 0, NULL, 0 }
  };

  optCon = poptGetContext(NULL, argc, (const char **)argv, optionsTable, 0);

  // -d
  if (optDistribution == NULL){
    optDistribution = defaultWslDistributionName();
  }

  // following switches just copy arguments through popt, and don't need extra
  // handling:
  // -t
  while ((ch = poptGetNextOpt(optCon)) >= 0){
    switch (ch){
      case 'e':
        if (action != WSL_NONE) errDuplicateActions();
        action = WSL_EXEC;
        break;
      case 'u':
        break;
      case 'i':
        if (action != WSL_NONE) errDuplicateActions();
        action = WSL_IMPORT;
        break;
      case 'p':
        break;
      case 't':
        break;
      case 'D':
        break;
      case 'T':
        break;
      case 'U':
        break;
      case 'I':
        if (action != WSL_SETUID) errDuplicateActions();
        break;
      case 'r':
        if (action != WSL_NONE) errDuplicateActions();
        action = WSL_RUN;
        break;
      case 's':
        if (action != WSL_NONE) errDuplicateActions();
        action = WSL_STOP;
        break;
      case 'L':
        setWslLogLevel(F_EVENTLOG, L_DEBUG);
        wslLogText(L_ERROR, L"Test wsl-service logging for errors\n");
        wslLogText(L_WARNING, L"Test wsl-service logging for warnings\n");
        wslLogText(L_INFO, L"Test wsl-service logging for information\n");
        exit(0);
        break;
    }
  }

  switch (action){
    case WSL_RUN:
      HANDLE hThread = startWslServiceThreadInteractiveA(argv[2], 0);
      while(TRUE){
        DWORD result = WaitForSingleObject(hThread, INFINITE);
        if (result == WAIT_OBJECT_0){
          DWORD exitCode;
          BOOL ret = GetExitCodeThread(hThread, &exitCode);
          if (ret) {
            wprintf(L"Thread exited: %i\n", exitCode);
            return 0;
          } else {
            // TODO, getting exit code status went wrong
          }
        }

        Sleep(1000);
      }
      break;
    case WSL_STOP:
      break;
    case WSL_SETUID:
      action = WSL_STOP;
      if (optDistribution == NULL){
        wprintf(L"No distribution specified, and no default distribution found, exiting.");
        return -1;
      }
      WslGetDistributionConfiguration(optDistribution,
                                      &distVer,
                                      &distUid,
                                      &distFlags,
                                      &distEnv,
                                      &distEnvCnt);

      wprintf(L"Setting default UID for %ls from %i to %i\n", optDistribution, distUid, optDefaultUid);
      WslConfigureDistribution(optDistribution, optDefaultUid, distFlags);
      return 0;
      break;
    case WSL_NONE:
      wprintf(L"Error: need an action\n");
      exit(1);
    default:
      wprintf(L"Error: not yet implemented\n");
      exit(1);
  }

  return 0;
}
