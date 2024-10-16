/* ФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФФ
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
 *     Filename : ODAUTO.C
 *  Description : Autodetection of terminal emulation supported on remote
 *                system
 *      Version : 5.00
 */

#include <string.h>
#include <ctype.h>

#include "opendoor.h"
#include "odintern.h"

#define ANSI_TRIES 1
#define ANSI_WAIT 12L
#define RIP_TRIES 1
#define RIP_WAIT 12L

#define MATCH_LEN 3

#define MIN(x, y) ((x) > (y)) ? (y) : (x)


/* od_autodetect() - determines the display capabilities of the remote */
/*                   terminal program.                                 */
void od_autodetect(int flags)
   {
   int counter;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_autodetect()");

   /* Initialize OpenDoors if it hasn't aready been done */
   if(!inited) od_init();

   /* Temporary code that will be optimized out, which prevents a compiler  */
   /* warning from being generated for the currently unused flags parameter */
   (void)flags;

   /* If operating in local mode, turn on ANSI mode, but not RIP */
   if(od_control.baud == 0)
      {
      od_control.user_ansi = TRUE;
      return;
      }

   /* If user_ansi is not set, attempt to determine ANSI capabilities */
   if(!od_control.user_ansi)
      {
      /* Clear inbound keyboard buffer */
      od_clear_keybuffer();

      /* Try twice to test ANSI capabilities */
      for(counter = 0; counter < ANSI_TRIES; ++counter)
         {
         /* Send a string that an ANSI capable terminal will usually */
         /* respond to */
         od_disp("\x1b[6n\r    \r", 10, FALSE);

         /* Wait for response expected from an ANSI terminal, for up to */
         /* 12/18.2 second */
         if(_waitnocase("\x1b[", ANSI_WAIT))
            {
            /* If expected sequence was received, turn on ANSI mode and */
            /* exit the loop */
            od_control.user_ansi = TRUE;
            break;
            }
         }
      od_clear_keybuffer();
      }

rip:
   /* If user_rip is not set, attempt to determine RIP capabilities */
   if(!od_control.user_rip)
      {
      /* Clear inbound keyboard buffer */
      od_clear_keybuffer();

      /* Try twice to test RIP capabilities */
      for(counter = 0; counter < RIP_TRIES; ++counter)
         {
         /* Send a string that a RIP capable terminal will usually */
         /* respond to */
         od_disp("!|1K|10000$RIPVER$|#|#|#\r                        \r", 50, FALSE);

         /* Wait for response expected from a RIP terminal, for up to */
         /* 12/18.2 second */
         if(_waitnocase("RIP", RIP_WAIT))
            {
            /* If expected sequence was received, turn on RIP mode and */
            /* exit the loop */
            od_control.user_rip = TRUE;
            break;
            }
         }
      od_clear_keybuffer();
      }
   }


char _waitnocase(char *string, long length)
   {
   long until = _clock_tick();
   char instring[MATCH_LEN + 1];
   int counter;
   char inkey;
   int compare_length = MIN(MATCH_LEN, strlen(string));

   for(counter = 0; counter <= MATCH_LEN; ++counter)
      {
      instring[counter] = '\0';
      }

   do
      {
      if((inkey = od_get_key(FALSE)) != 0)
         {
         for(counter = 0; counter < MATCH_LEN - 1; ++ counter)
            {
            instring[counter] = instring[counter + 1];
            }
         instring[MATCH_LEN - 1] = inkey;

         if(strnicmp(instring + (MATCH_LEN - compare_length), string, compare_length) == 0) return(TRUE);
         }
      } while(_clock_tick() <= until + length && until <= _clock_tick());


   return(FALSE);
   }
