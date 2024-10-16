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
 *     Filename : ODWIN.C
 *  Description : Contains code for text block manipulation functions
 *      Version : 5.00
 */

#include <alloc.h>
#include <string.h>

#include "opendoor.h"
#include "odintern.h"


void *od_window_create(int left, int top, int right, int bottom, char *title, char boardcol, char titlecol, char insidecol, int reserved)
   {
   char line_counter;                     /* Number of current line being drawn */
   char between_size;
   void *buffer;
   char title_size;
   char remaining;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_window_create()");

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   reserved &= 0x00;

   between_size = (right - left) - 1;

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
      return(NULL);
      }

   if(left<1 || top<1 || right>80 || bottom>25 || right-left < 2 || bottom-top < 2)
      {
      od_control.od_error = ERR_PARAMETER;
      return(NULL);
      }

   if((buffer=malloc( (right-left+1)*2 + (bottom-top+1)*160 + 4)) == NULL)
      {
      od_control.od_error = ERR_MEMORY;
      return(NULL);
      }

   if(!od_gettext(left, top, right, bottom, (char *)buffer+4))
      {
      free(buffer);
      return(NULL);                    /* (od_error code set in od_gettext()) */
      }

   ((char *)buffer)[0]=left;
   ((char *)buffer)[1]=top;
   ((char *)buffer)[2]=right;
   ((char *)buffer)[3]=bottom;

   if(title==NULL)
      {
      title_size = 0;
      }
   else
      {
      if((title_size = strlen(title)) > (between_size - 4))
         {
         title_size = between_size - 4;
         }
      }

   od_set_cursor(top,left);                /* move to top corner, if applicable */
   od_set_attrib(boardcol);
   /* display corner character */
   od_putch(od_control.od_box_chars[BOX_UPPERLEFT]);
   if(title_size == 0)
      {
      od_repeat(od_control.od_box_chars[BOX_TOP],between_size);   /* display top line */
      }
   else
      {
      od_repeat(od_control.od_box_chars[BOX_TOP],remaining=((between_size-title_size-2)/2));
      od_set_attrib(titlecol);
      od_putch(' ');
      od_disp(title,title_size,TRUE);
      od_putch(' ');
      od_set_attrib(boardcol);
      od_repeat(od_control.od_box_chars[BOX_TOP],between_size-remaining-title_size-2);
      }

   od_putch(od_control.od_box_chars[BOX_UPPERRIGHT]); /* display corner character */

   if(od_control.user_avatar)                    /* If AVATAR mode is available */
      {
      od_set_cursor(top+1,left);            /* Display first left verticle line */
      od_putch(od_control.od_box_chars[BOX_LEFT]);

      od_emulate(22);                               /* Fill in center of window */
      od_emulate(12);
      od_emulate(insidecol);
      od_emulate((bottom-top)-1);
      od_emulate(between_size);

      od_set_attrib(boardcol);
      od_set_cursor(top+1,right);          /* Display first right verticle line */
      od_putch(od_control.od_box_chars[BOX_RIGHT]);

                                            /* Display remaining verticle lines */
      for(line_counter=top+2;line_counter<bottom;++line_counter)
         {
         od_set_cursor(line_counter,left);                /* move to line start */
         od_putch(od_control.od_box_chars[BOX_LEFT]); /* display left line char */
         od_set_cursor(line_counter,right);               /* move to line start */
         od_putch(od_control.od_box_chars[BOX_RIGHT]);    /* display right line */
         }
      }

   else                                      /* If AVATAR mode is not available */
      {                                  /* loop through middle lines of window */
      for(line_counter=top+1;line_counter<bottom;++line_counter)
         {
         od_set_cursor(line_counter,left);                /* move to line start */
         od_putch(od_control.od_box_chars[BOX_LEFT]); /* display left line char */
         od_set_attrib(insidecol);                         /* set window colour */
         od_repeat(' ',between_size);                     /* display blank area */
         od_set_attrib(boardcol);                         /* set boarder colour */
         od_putch(od_control.od_box_chars[BOX_RIGHT]);    /* display right line */
         }
      }

   /* Display bottom border of window */
   od_set_cursor(bottom,left);
   od_putch(od_control.od_box_chars[BOX_LOWERLEFT]);
   od_repeat(od_control.od_box_chars[BOX_BOTTOM],between_size);
   od_putch(od_control.od_box_chars[BOX_LOWERRIGHT]);

   return(buffer);                                       /* Return with success */
   }


/* Function to remove window referred to by "info" from the screen, and */
/* deallocate space used by window storage buffer */
int od_window_remove(void *info)
   {
   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_window_remove()");

   if(info == NULL) return(FALSE);

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   if(!od_puttext(((char *)info)[0], ((char *)info)[1], ((char *)info)[2], ((char *)info)[3], (char *)info + 4))
      {
      free(info);
      return(FALSE);                   /* (od_error code set in od_puttext()) */
      }

   free(info);

   return(TRUE);
   }
