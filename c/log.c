
#include <stdlib.h>
#include <stdio.h>
#include "log.h"

int log_opened = 0;

int log_msg(char* msg)
{
    // Comment the next line to enable logging.
    return 0;

    FILE* file = NULL;

    if (!log_opened)
    {
        file = fopen(LOGFILE, "w");
        log_opened = 1;
    } 
    else 
    {
        file = fopen(LOGFILE, "a");
    }

    if (file == NULL) 
    {
        printf("Could not open log file %s.\n", LOGFILE);
        log_opened = 0;
        return 0;
    }
    else
    {
        if (fprintf(file, "%s", msg) < 0)
        {
            printf("Could not write to file %s.\n", LOGFILE);
        }
        fclose(file);
    }

    return 0;
}

int log_err(char* msg) 
{
  // Comment the next line to enable logging.
  //return 0;

  char m[200];
  sprintf(m, "[ERROR]: %s", msg);
  log_msg(m);
  exit(1);
  return 0;
}
