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
 *     Filename : ODBLOCK.C
 *  Description : Contains code for text block manipulation functions
 *      Version : 5.00
 */


#include <stdlib.h>
#include <string.h>

#include "opendoor.h"
#include "odintern.h"

/* Set to TRUE when od_puttext() should leave the cursor in its original */
/* position */
int _scrollaction=TRUE;



/* od_puttext() - Displays the contents of the buffer passed in blcok.     */
/*                Leaves cursor in original position, unless _scrollaction */
/*                is FALSE. Leaves colour at original value.               */
int od_puttext(int left, int top, int right, int bottom, void *block)
   {
   int row_len = right - left +1;
   int row_bytes = row_len * 2;
   char *test;
   char *memory;
   char *buffer;
   char *scrn_block;
   int block_row = 0;
   int out_row;
   int out_colour=999;
   int out_col, check_col;
   char *mem_block;
   int move_cost=(od_control.od_avatar)?4:7;
   char max_right, max_bottom;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_puttext()");

   /* Ensure that OpenDoors is initialized before proceeding */
   if(!inited) od_init();

   /* Get current display setting profile */
   phys_gettextinfo(&user_text);

   /* Calculate the maximum values for bottom and right of block */
   max_right=user_text.winright-user_text.winleft+1;
   max_bottom=user_text.winbottom-user_text.wintop+1;

   /* Check that parameters seem reasonable */
   if(left<1 || top<1 || right>max_right || bottom>max_bottom
      || top > bottom || left > right || block==NULL)
      {
      od_control.od_error = ERR_PARAMETER;
      return(FALSE);
      }

   /* Ensure that ANSI and/or AVATAR mode is available */
   if(!od_control.caller_ansi && !od_control.od_avatar)
      {
      od_control.od_error = ERR_NOGRAPHICS;
      return(FALSE);
      }

   /* If OpenDoors is operating in remote mode */
   if(od_control.baud != 0)
      {
      /* Allocate temporary buffer to store original screen contents while */
      /* buffer paste is being performed.                                  */
      if((buffer=malloc(row_bytes*(bottom-top+1)))==NULL)
         {
         od_control.od_error = ERR_MEMORY;
         return(FALSE);
         }

      /* Get current screen contents of area to be pasted into, storing */
      /* into the temporary buffer.                                     */
      if(!phys_gettext(left,top,right,bottom,buffer))
         {
         od_control.od_error = ERR_PARAMETER;
         free(buffer);
         return(FALSE);
         }
      }

   /* Display the block to be pasted on the local screen */
   if(!phys_puttext(left,top,right,bottom,block))
      {
      od_control.od_error = ERR_PARAMETER;
      free(buffer);
      return(FALSE);
      }

   /* If operating in remote mode */
   if(od_control.baud != 0)
      {
      /* Loop for each line in the buffer to be pasted */
      for(out_row=top;out_row<=bottom;++out_row,++block_row)
         {
         /* Setup pointer to beginning of line of original screen contents */
         scrn_block=(char *)buffer+(row_bytes*block_row);

         /* Setup pointer to beginning of line of block to be displayed */
         mem_block=(char *)block+(row_bytes*block_row);

         /* Loop for each column on this line */
         for(out_col=0;out_col<row_len;)
            {
            /* Loop from this character onwards, counting number of */
            /* characters that don't need to be changed: */
            check_col=out_col;
            memory=((char *)mem_block)+(check_col<<1);
            test=((char *)scrn_block)+(check_col<<1);
            for(;check_col<row_len;++check_col)
               {
               if(od_control.od_full_put) break;

               /* If both buffers have space characters */
               if(*memory==*test && (*test==' ' || *test=='\0'))
                  {
                  /* If colours differ, then stop comparison loop */
                  if((test[1]&0x70) != (memory[1]&0x70))
                     {
                     break;
                     }
                  }

               /* If both have different character and colour attributes */
               else if(*((int *)test)!=*((int *)memory))
                  {
                  /* Then stop comparison loop now */
                  break;
                  }

               /* Increment source and background pointers by two byts */
               test+=2;
               memory+=2;
               }

            /* If no futher text to change on this line */
            if(check_col==row_len)
               {
               /* Move to the next line */
               goto next_line;
               }

            /* If this is the first text to be displayed on this line */
            if(out_col == 0)
               {
               /* Move the cursor to the first text to be changed on line */
               out_col = check_col;

               /* If AVATAR mode is available */
               if(od_control.od_avatar)
                  {
                  /* Send the avatar cursor positioning command */
                  globworkstr[0]=22;
                  globworkstr[1]=8;
                  globworkstr[2]=out_row;
                  globworkstr[3]=left+out_col;
                  od_disp(globworkstr,4,FALSE);
                  }
               else
                  {
                  /* Otherwise, send the ANSI cursor positioning command */
                  sprintf(globworkstr,"x[%d;%dH",out_row,left + out_col);
                  globworkstr[0]=27;
                  od_disp(globworkstr,strlen(globworkstr),FALSE);
                  }
               }

            /* If the number of characters after current cursor position  */
            /* which must be changed, is greater than the number of       */
            /* characters required to reposition the cursor on this line, */
            /* then move the cursor to skip unchanged characters.         */
            else if((check_col-out_col)>move_cost)
               {
               out_col=check_col;
               /* If AVATAR mode is available */
               if(od_control.od_avatar)
                  {
                  /* Advance cursor appropriate number of characters */
                  /* using the AVATAR cursor position command        */
                  globworkstr[0]=22;
                  globworkstr[1]=8;
                  globworkstr[2]=out_row;
                  globworkstr[3]=left+out_col;
                  od_disp(globworkstr,4,FALSE);
                  }
               else
                  {
                  /* Otherwise, advance cursor appropriate number of      */
                  /* characters using the AVATAR cursor position command. */
                  sprintf(globworkstr,"x[%d;%dH",out_row,left + out_col);
                  globworkstr[0]=27;
                  od_disp(globworkstr,strlen(globworkstr),FALSE);
                  }
               }

            /* Output text for the number of characters found to be */
            /* different.                                           */
            test=(char *)&mem_block[out_col*2];
            for(;out_col<=check_col;++out_col)
               {
               if(test[1] != out_colour)
                  {
                  od_set_attrib(out_colour=test[1]);
                  }
               od_disp(test,1,FALSE);
               test++;
               test++;
               }
            }
next_line:
         ;
         }

      /* If not disabled, update cursor position */
      if(_scrollaction)
         {
         od_set_cursor(user_text.cury,user_text.curx);
         }

      /* Deallocate temporary buffer */
      free(buffer);
      }

   /* Restore the original display attribute */
   od_set_attrib(user_text.attribute);

   /* Return with success */
   return(TRUE);
   }
   

/* has no effect on cursor position */
int od_gettext(int left, int top, int right, int bottom, void *block)
   {
   char max_right, max_bottom;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_gettext()");

   if(!inited) od_init();

   phys_gettextinfo(&user_text);

   max_right=user_text.winright-user_text.winleft+1;
   max_bottom=user_text.winbottom-user_text.wintop+1;
   if(left<1 || top<1 || right>max_right || bottom>max_bottom || !block)
      {
      od_control.od_error = ERR_PARAMETER;
      return(FALSE);
      }

   if(!od_control.caller_ansi && !od_control.od_avatar)
      {
      od_control.od_error = ERR_NOGRAPHICS;
      return(FALSE);
      }

   return(phys_gettext(left,top,right,bottom,block));
   }



/* od_scroll() - Scrolls the specified area of the screen by the specified */
/*               number of lines, in either the up or down directions. The */
/*               cursor is left at its original locaiton, and the display  */
/*               attribute is left at its original setting. New lines are  */
/*               created in the current display colour.                    */
int od_scroll(int left, int top, int right, int bottom, int distance, unsigned int flags)
   {
   char width, height;
   char counter;
   char first, last;
   char avt_seq[7];
   void *block;
   char blankstr[81];
   char keep_height;
   char max_right;
   char max_bottom;
   struct phys_text_info info;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_scroll()");

   /* Ensure that OpenDoors has been initialized before proceeding */
   if(!inited) od_init();

   /* Get current display setting information */
   phys_gettextinfo(&info);

   /* Determine the height and width of the area to be scrolled */
   width=right-left+1;
   height=bottom-top+1;

   /* Determine the number of lines currently in the area that will still */
   /* be visible after scrolling.                                         */
   keep_height=height-((distance>=0) ? distance : -distance);

   /* Determine the maximum bottom and left coordinates of an area to be */
   /* scrolled.                                                          */
   max_right=info.winright-info.winleft+1;
   max_bottom=info.winbottom-info.wintop+1;

   /* Check that parameters are valid */
   if(left<1 || top<1 || right>max_right || bottom>max_bottom ||
      left > right || top > bottom)
      {
      od_control.od_error = ERR_PARAMETER;
      return(FALSE);
      }

   /* Check that ANSI or AVATAR graphics mode is available */
   if(!od_control.caller_ansi && !od_control.od_avatar)
      {
      od_control.od_error = ERR_NOGRAPHICS;
      return(FALSE);
      }

   /* If distance to be scrolled is 0, then we are finished already and */
   /* can return immediately.                                           */
   if(distance == 0)
      {
      return(TRUE);
      }

   /* If distance is positive, then we are moving text upwards. */
   if(distance>0)
      {
      /* Ensure that distance is not greater than size of scrolled area. */
      if(distance>height)
         {
         distance=height;
         }

      /* Calculate first and last line to be moved */
      first=bottom-(distance-1);
      last=bottom;
      }

   /* If distance is negative, then we are moving text downwards. */
   else /* if(distance<0) */
      {
      /* Ensure that distance is not greater than size of scrolled area. */
      if(distance<-height)
         {
         distance=-height;
         }

      /* Calculate first and last line to be moved */
      first=top;
      last=top-distance-1;
      }

   /* If AVATAR mode is available */
   if(od_control.user_avatar)
      {
      /* Generate AVATAR sequence which causes the remote terminal to */
      /* scroll an area of its screen.                                */
      avt_seq[0]=22;

      /* If scrolling text upwards */
      if(distance>0)
         {
         /* Specify control sequence for scrolling upwards */
         avt_seq[1]=10;
         avt_seq[2]=distance;

         /* Move text appropriate direction on local screen */
         phys_movetext(left,top+distance,right,bottom,left,top);
         }
      /* If scrolling text downwards */
      else /* if(disatnce<0) */
         {
         /* Specify control sequence for scrolling downwards */
         avt_seq[1]=11;
         avt_seq[2]=-distance;

         /* Move text appropriate direction on local screen */
         phys_movetext(left,top,right,bottom+distance,left,top-distance);
         }

      /* Specify area to be scrolled to the AVATAR terminal. */
      avt_seq[3]=top;
      avt_seq[4]=left;
      avt_seq[5]=bottom;
      avt_seq[6]=right;

      /* Send the control sequence to the AVATAR terminal. */
      od_disp(avt_seq,7,FALSE);

      /* Generate string containing a blank line of text */
      for(counter=0;counter<width;++counter) blankstr[counter]=' ';
      blankstr[counter]='\0';

      /* Blank-out lines that will no longer be visiable */
      for(;first<=last;++first)
         {
         phys_gotoxy(left,first);
         phys_cputs(blankstr);
         }

      /* Reset cursor position on local display */
      phys_gotoxy(info.curx,info.cury);
      }

   /* Otherwise, we are using ANSI mode */
   else /* if(od_control.user_ansi) */
      {
      /* If any of the original text will still be available after scrolling. */
      if(keep_height>0)
         {
         /* Allocate some temporary memory to hold text to be "got" */
         if((block=malloc(keep_height*width*2))==NULL)
            {
            /* If memory allocation failed, then scrolling fails */
            od_control.od_error = ERR_MEMORY;
            return(FALSE);
            }

         /* If we are scrolling text upwards */
         if(distance > 0)
            {
            /* Move text that will still be visible, using od_gettext() */
            /* and od_puttext().                                        */
            od_gettext(left,top+distance,right,bottom,block);
            _scrollaction=FALSE;
            od_puttext(left,top,right,bottom-distance,block);
            _scrollaction=TRUE;
            }

         /* If we are scrolling text downwards */
         else /* if(distance < 0) */
            {
            /* Move text that will still be visible, using od_gettext() */
            /* and od_puttext().                                        */
            od_gettext(left,top,right,bottom+distance,block);
            _scrollaction=FALSE;
            od_puttext(left,top-distance,right,bottom,block);
            _scrollaction=TRUE;
            }

         /* Deallocate temporary memory block */
         free(block);
         }

      /* If new area clearing has not been disabled */
      if(!(flags&SCROLL_NO_CLEAR))
         {
         /* Loop for lines that should be blank */
         for(;first<=last;++first)
            {
            /* Move cursor to the beginning of this line */
            od_set_cursor(first,left);

            /* If right boarder of area to be scrolled is the edge of the */
            /* screen, then we can use a quick control sequence to clear  */
            /* the rest of the line. Call od_clr_line() to do this.       */
            if(right == 80)
               {
               od_clr_line();
               }

            /* If right boarder of area to be scrolled is not at the edge */
            /* of the screen, then each line must be manually erased, by  */
            /* sending the appropriate number of blanks (spaces).         */
            else /* if(right != 80) */
               {
               od_repeat(' ',width);
               }
            }
         }

      /* Reset the cursor to its original position */
      od_set_cursor(info.cury,info.curx);
      }

   /* Return with success */
   return(TRUE);   
   }



/* od_save_screen() - Stores the contents of the screen into a buffer,   */
/*                    along with the current cursor location and display */
/*                    colour. Supports all display modes. Buffer must be */
/*                    at least 4004 bytes in size.                       */
int od_save_screen(void *buffer)
   {
   char height;
   struct phys_text_info info;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_save_screen()");

   /* Ensure that OpenDoors is initialized before proceeding */
   if(!inited) od_init();

   /* Check parameters and current output window size */
   phys_gettextinfo(&info);
   if(info.winleft!=1 || info.winright!=80 || !buffer)
      {
      od_control.od_error = ERR_PARAMETER;
      return(FALSE);
      }

   /* Store current cursor location in buffer */
   ((char *)buffer)[0]=info.curx;
   ((char *)buffer)[1]=info.cury;

   /* Store current display colour in buffer */
   ((char *)buffer)[2]=info.attribute;

   /* Store height of buffer stored */
   ((char *)buffer)[3]=height=info.winbottom-info.wintop+1;

   /* Store screen contents using local screen gettext() function */
   return(phys_gettext(1,1,80,height,(char *)buffer+4));
   }


int od_restore_screen(void *buffer)
   {
   register char *character;
   register char pos;
   register char last_char;
   register char *text_buf;
   char height;
   int to_return=TRUE;
   struct phys_text_info info;
   char line;
   char distance=0;
   char cursor_row;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_restore_screen()");

   /* Ensure that OpenDoors is initialized before proceeding */
   if(!inited) od_init();

   /* Check parameters and current output window size */
   phys_gettextinfo(&info);
   if(info.winleft!=1 || info.winright!=80 || !buffer)
      {
      od_control.od_error = ERR_PARAMETER;
      return(FALSE);
      }

   /* Determine current window height were text will be restored */
   height=info.winbottom-info.wintop+1;

   /* If current display window height is too small for entire buffer */
   /* to be re-displayed.                                             */
   if(height<((char *)buffer)[3])
      {
      /* Then we will always display as much of the END of the buffer */
      /* as possible.                                                 */
      distance = height - ((char *)buffer)[3];
      }
   else if(height > ((char *)buffer)[3])
      {
      /* Otherwise, ensure that we don't try to display more lines that */
      /* are in the buffer.                                             */
      height=((char *)buffer)[3];
      }

   /* Clear the remote and local screens */
   od_clr_scr();

   /* If ANSI or AVATAR modes are available */
   if(od_control.od_avatar || od_control.caller_ansi)
      {
      /* Then we can efficiently display the buffer using od_puttext() */
      _scrollaction=FALSE;
      to_return=od_puttext(1,1,80,height,(char *)buffer+(4+((int)distance*160)));
      _scrollaction=TRUE;

      /* Restore original cursor location */
      od_set_cursor(((char *)buffer)[1],((char *)buffer)[0]);

      /* Restore original display attribute */
      od_set_attrib(((char *)buffer)[2]);
      }

   /* If we are operating in ASCII mode */
   else /* if (!od_control.od_avatar && !od_control.caller_ansi) */
      {
      /* Then the buffer is displayed one line at a time, beginning  */
      /* at the top of the screen, up to the saved cusrsor location. */

      /* Set pointer to beginning of buffer to be displayed */
      text_buf=(char *)buffer+4;

      /* Get final cursor row number */
      cursor_row=((char *)buffer)[1];

      /* Loop for each line in the buffer */
      for(line=1;line<=height;++line)
         {
         /* Set pointer to last character of line */
         character=(char *)text_buf+158;

         /* Loop backwards until a non-blank character is found, or we */
         /* reach the beginning of the line.                           */
         for(last_char=80;last_char>1;)
            {
            /* If this is a blank character */
            if(*character==32 || *character==0)
               {
               /* Move to previous character */
               --last_char;
               character-=2;
               }

            /* If this is not a blank character, then stop looping */
            else
               {
               break;
               }
            }

         /* If this is the line on which the cursor resides */
         if(line==cursor_row)
            {
            /* If last non-blank character of line is at or past the final */
            /* cursor location, then we backup the last character to be    */
            /* displayed to the cursor before the final cursor position.   */
            /* This code could be improved to be able to display text on   */
            /* the entire cursor line by displaying the entire line,       */
            /* sending a C/R, and redisplaying first portion of line to    */
            /* end up with the cursor in the desired position.             */
            if(last_char>=((char *)buffer)[0])
               {
               last_char=((char *)buffer)[0]-1;
               }
            }

         /* Display all characters on this line */
         character = (char *)text_buf;
         for(pos=1;pos<=last_char;++pos)
            {
            od_putch(*character);
            character+=2;
            }

         /* If this is the row where the cursor should be left, then we */
         /* stop displaying now.                                        */
         if(line==cursor_row)
            {
            break;
            }

         /* If cursor hasn't been wrapped, then we should send a C/R - */
         /* L/F sequence.                                              */
         if(last_char != 80)
            {
            od_disp_str("\n\r");
            text_buf+=160;
            }
         }
      }

   /* Return with the appropriate success/failure status */
   return(to_return);
   }
