
#pragma once

extern int log_level;
extern int logging_enable;

int SetupLogging(void);
int OpenLogFile(void);
int LogMessage(char *logmsg);
