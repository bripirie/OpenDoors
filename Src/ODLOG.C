/*
 * лллллллллл                         лллллллм
 * лллппппллл                         лллпппллл
 * ллл    ллл ммммммм ммммммм ммммммм ллл   ллл ммммммм ммммммм мммммм ммммммм
 * ллл    ллл лллпллл лллпллл лллпллл ллл   ллл лллпллл лллпллл лллппп лллпппп
 * лллммммллл ллл ллл лллпппп ллл ллл лллмммллл ллл ллл ллл ллл ллл    ппппллл
 * лллллллллл ллллллл ллллллл ллл ллл лллллллп  ллллллл ллллллл ллл    ллллллл
 *            ллл
 *            ллл
 *            ппп                                     Door Programming Toolkit
 * ФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФ
 *
 *      (C) Copyright 1991 - 1994 by Brian Pirie. All Rights Reserved.
 *
 *
 *
 *
 *     Filename : ODLOG.C
 *  Description : Code compsing the optional log file sub-system.
 *      Version : 5.00
 */


/* Include OpenDoors header files */
#include "opendoor.h"
#include "odintern.h"

/* Include standard C library header files */
#include <stdio.h>
#include <time.h>


/* Private logfile file handle */
FILE *logfile_pointer;



/* Function to call when logfile option is included */
void option_logfile(void)
   {
   /* At this time, this function simply maps to a call to od_log_open() */
   od_log_open();
   }


/* Function to open logfile */
int od_log_open()
   {
   time_t timer;
   struct tm *tblock;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_log_open()");

   /* Initialize OpenDoors if not already done */
   if(!inited) od_init();

   /* Don't open logfile if it has been disabled in config file, etc. */
   if(od_control.od_logfile_disable) return(TRUE);

   /* Open actual logfile */
   if((logfile_pointer=fopen(od_control.od_logfile_name,"a"))==NULL)
      {
      return(FALSE);
      }

   /* Get the current time */
   timer=time(NULL);
   tblock=localtime(&timer);

   /* Print logfile tear line */
   fprintf(logfile_pointer,"\n----------  %s %02.2d %s %02.2d, %s\n",
                      od_control.od_day[tblock->tm_wday],
                      tblock->tm_mday,
                      od_control.od_month[tblock->tm_mon],
                      tblock->tm_year,
                      od_program_name);

   /* Print message of door start up */
   sprintf(globworkstr,(char *)od_control.od_logfile_messages[11],od_control.user_name);
   od_log_write(globworkstr);

   /* Set internal function hooks to enable calling of logfile features */
   /* from elsewhere in OpenDoors */
   _log_wrt=_log_write;
   _log_close=_close_logfile;

   return(TRUE);
   }


/* Internal function to write a non-formatted string from od_log_messages[] */
int _log_write(int code)
   {
   if(code < 0 || code > 11) return(FALSE);
   od_log_write((char *)od_control.od_logfile_messages[code]);

   if(code == 8)
      {
      sprintf(globworkstr,od_control.od_logfile_messages[12],od_control.user_reasonforchat);
      globworkstr[67] = '\0';
      od_log_write(globworkstr);
      }

   return(TRUE);
   }


/* Function to write line to logfile */
/* This function does not use globworkstr */
int od_log_write(char *message)
   {
   char *string;
   time_t timer;
   struct tm *tblock;

   if(!inited) od_init();              /* verify that we've been initialized */

   /* Stop if logfile has been disabled in config file, etc. */
   if(od_control.od_logfile_disable) return(TRUE);

   /* If logfile has not yet been opened, then open it */
   if(logfile_pointer==NULL)
      {
      if(!od_log_open()) return (FALSE);
      }

   /* Get the current system time */
   timer=time(NULL);
   tblock=localtime(&timer);

   /* Determine which logfile format string to use */
   if(tblock->tm_hour<10)
      {
      string=(char *)">  %1.1d:%02.2d:%02.2d  %s\n";
      }
   else
      {
      string=(char *)"> %2.2d:%02.2d:%02.2d  %s\n";
      }

   /* Write a line to the logfile */
   fprintf(logfile_pointer,string,tblock->tm_hour,tblock->tm_min,tblock->tm_sec,message);

   return(TRUE);
   }



/* Internal function used to close the logfile at door exit */
void _close_logfile(int reason)
   {
   /* Stop if logfile has been disabled in the config file, etc. */
   if(od_control.od_logfile_disable) return;

   /* If logfile has not been opened, then abort */
   if(logfile_pointer==NULL) return;

   if(_preorexit)
      {
      od_log_write((char *)od_control.od_logfile_messages[13]);
      }
   else if(_exitreason<=5 && _exitreason>=1)
      {
      od_log_write((char *)od_control.od_logfile_messages[_exitreason-1]);
      }
   else
      {
      sprintf(globworkstr,(char *)od_control.od_logfile_messages[5],reason);
      od_log_write(globworkstr);
      }

   /* Close the logfile */
   fclose(logfile_pointer);

   /* Prevent further use of logfile without first re-opening it */
   _log_wrt=NULL;
   _log_close=NULL;
   logfile_pointer=NULL;
   }
