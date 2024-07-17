#ifndef _WSL_LOG_H
#define _WSL_LOG_H

/** @file wsl-log.h
 * Definitions for logging functions
 *
 * Idea here is to have at least three outputs:
 * - windows event log
 * - text file
 * - stdout/err
 *
 * Each of those sources may be disabled in configuration.
 * There may be a different severity level for each of those sources,
 * and there's a distinction between interactive and non-interactive use
 * (i.e., there are 6 severity levels to be configured)
 */

#include <windef.h>

/** Constants to specify log severity
 * Big steps to allow adding levels in between, if necessary.
 */
enum WSL_LOGLEVELS {
  L_DISABLED = 0,
  L_UNSPEC   = 10,
  L_DEADLY   = 20,
  L_ERROR    = 30,
  L_WARNING  = 40,
  L_INFO     = 50,
  L_VERBOSE  = 60,
  L_DEBUG    = 70,
};

#define WSL_LOGFACILITY_COUNT 3

enum WSL_LOGFACILITIES {
  F_CONSOLE  = 0,
  F_FILE     = 1,
  F_EVENTLOG = 2,
};

extern int const copyBufferSize;
extern size_t copyBufferTCharSize;

/** Initialize logging for interactive or non-interactive use.
 *
 * This is expected to be called once, before any logging functions.
 * Calling it a second time, or after logging functions may have undefined
 * results.
 *
 * The default, if this function is not called, is to assume running in an
 * interactive session.
 *
 * @param logType
 */
void setWslLogInteractive(BOOL logType);
/** Set the log file for logging to disk. Note that this will only be used
 * if the logging configuration for the current session and log level configures
 * disk based logging.
 *
 * If this function gets called multiple times it'll switch the current log
 * file - there is always at most one active log file.
 *
 * @param logFile the path to a log file. The path must be writeable.
 */
void setWslLogFile(LPCWSTR logFile);
/** Change the log level for the current session.
 */
void setWslLogLevel(int facility, int level);
/** Log a simple string
 *
 * @param level loglevel for this message
 * @param logMessage the message
 */
void wslLogText(int level, LPCWSTR logMessage);
/** Log a message with a format string
 *
 * @param level loglevel for this message
 * @param logFormat the format string
 * @param ... format string arguments
 */
void wslLog(int level, LPCWSTR logFormat, ...);

#endif
