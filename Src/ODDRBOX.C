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
 *     Filename : ODDRBOX.C
 *  Description : Contains code for the remote/local windowing functions.
 *      Version : 5.00
 */


/* OpenDoors global header files */
#include "opendoor.h"
#include "odintern.h"


/* Function to draw a box */
int od_draw_box(char left, char top, char right, char bottom)
   {
   char line_counter;                     /* Number of current line being drawn */
   char between_size=(right-left)-1;                        /* X size of window */

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_draw_box()");

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   /* Setup od_box_chars appropriately */
   if(od_control.od_box_chars[BOX_BOTTOM]==0)
      {
      od_control.od_box_chars[BOX_BOTTOM] = od_control.od_box_chars[BOX_TOP];
      }
   if(od_control.od_box_chars[BOX_RIGHT]==0)
      {
      od_control.od_box_chars[BOX_RIGHT] = od_control.od_box_chars[BOX_LEFT];
      }

   if(!(od_control.user_ansi || od_control.user_avatar))
      {
      od_control.od_error = ERR_NOGRAPHICS;
      return(FALSE);
      }

   if(left<1 || top<1 || right>80 || bottom>25)
      {
      od_control.od_error = ERR_PARAMETER;
      return(FALSE);
      }

   od_set_cursor(top,left);                /* move to top corner, if applicable */
   od_putch(od_control.od_box_chars[BOX_UPPERLEFT]);            /* display corner character */
   od_repeat(od_control.od_box_chars[BOX_TOP],between_size);      /* display top line */
   od_putch(od_control.od_box_chars[BOX_UPPERRIGHT]);            /* display corner character */

   if(od_control.user_avatar)                    /* If AVATAR mode is available */
      {
      od_set_cursor(top+1,left);            /* Display first left verticle line */
      od_putch(od_control.od_box_chars[BOX_LEFT]);

      od_emulate(22);                               /* Fill in center of window */
      od_emulate(12);
      od_emulate(od_control.od_cur_attrib);
      od_emulate((bottom-top)-1);
      od_emulate(between_size);

      od_set_cursor(top+1,right);          /* Display first right verticle line */
      od_putch(od_control.od_box_chars[BOX_RIGHT]);

                                            /* Display remaining verticle lines */
      for(line_counter=top+2;line_counter<bottom;++line_counter)
         {
         od_set_cursor(line_counter,left);                /* move to line start */
         od_putch(od_control.od_box_chars[BOX_LEFT]);        /* display left line char */
         od_set_cursor(line_counter,right);               /* move to line start */
         od_putch(od_control.od_box_chars[BOX_RIGHT]);       /* display right line char */
         }
      }

   else                                      /* If AVATAR mode is not available */
      {                                  /* loop through middle lines of window */
      for(line_counter=top+1;line_counter<bottom;++line_counter)
         {
         od_set_cursor(line_counter,left);                /* move to line start */
         od_putch(od_control.od_box_chars[BOX_LEFT]);        /* display left line char */
         od_repeat(' ',between_size);                     /* display blank area */
         od_putch(od_control.od_box_chars[BOX_RIGHT]);       /* display right line char */
         }
      }

   od_set_cursor(bottom,left);                         /* move to bottom corner */
   od_putch(od_control.od_box_chars[BOX_LOWERLEFT]);            /* display corner character */
   od_repeat(od_control.od_box_chars[BOX_BOTTOM],between_size);   /* display bottom line */
   od_putch(od_control.od_box_chars[BOX_LOWERRIGHT]);            /* display corner character */

   return(TRUE);                                         /* Return with success */
   }
