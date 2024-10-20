/*
 * log.h - the header file which defines log_msg(); and log_err();
 */

#define LOGFILE "./log/log.txt" // All log messages will be appended to this file.
extern int log_opened; // Keeps track of whether the log file is open or not.

int log_msg(char* msg);	// Just log a message.
int log_err(char* msg);	// Log a message and exit the program.
