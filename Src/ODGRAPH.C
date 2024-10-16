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
 *     Filename : ODGRAPH.C
 *  Description : Contains code for the non-standard ANSI/AVATAR control
 *                functions.
 *      Version : 5.00
 */



#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"

#include<stdio.h>                      /* Standard header files */
#include<stdlib.h>
#include<string.h>



void od_clr_line(void)                 /* function to clear rest of line */
   {
   register char charsleft,counter,*pos;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_clr_line()");

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   phys_gettextinfo(&user_text);       /* get cursor position */
   charsleft=80-user_text.curx;        /* calculate # columns to erase */

   if(od_control.user_avatar || od_control.user_ansi)
      {
      pos=(char *)globworkstr;
      for(counter=0;counter<=charsleft;++counter) *pos++=' ';
      *pos='\0';
      phys_setscroll(0);
      phys_cputs(globworkstr);
      phys_setscroll(1);
      phys_gotoxy(user_text.curx, user_text.cury);
      }

   if(od_control.od_avatar)            /* if in AVATAR mode */
      {
      od_disp(avatar_clear,2,FALSE);   /* send the 2-char AVATAR sequence */
      }

   else if(od_control.caller_ansi)     /* if in ANSI mode */
      {
      od_disp(ansi_clear,3,FALSE);     /* send the 3-char ANSI sequence */
      }

   else                                /* if in plain ASCII mode */
      {                                /* send the up to 160 char sequence */
      pos=(char *)globworkstr;         /* Place spaces in string */
      for(counter=0;counter<charsleft;++counter) *pos++=' ';
      for(counter=0;counter<charsleft;++counter) *pos++=8;
      *pos='\0';
      od_disp(globworkstr,strlen(globworkstr),TRUE);
      }
   }


void od_set_cursor(int row,int col)    /* function to set cursor position */
   {
   static char string[40];

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_set_cursor()");

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   /* Check validity of parameters */
   if(row < 1 || col < 1)
      {
      od_control.od_error = ERR_PARAMETER;
      }

   if(od_control.od_avatar)            /* if AVATAR mode is on */
      {
      phys_gotoxy(col,row);            /* position local cursor */
      string[0]=22;                    /* generate AVATAR sequence */
      string[1]=8;
      string[2]=row;
      string[3]=col;
      od_disp(string,4,FALSE);         /* send AVATAR sequence */
      }
   else if(od_control.caller_ansi)     /* if ANSI mode is on */
      {                                /* send ANSI control sequence */
      sprintf(string,"x[%d;%dH",row,col);
      string[0]=27;
      od_disp(string,strlen(string),FALSE);
      phys_gotoxy(col,row);            /* position cursor on local screen */
      }
   else
      {
      od_control.od_error = ERR_NOGRAPHICS;
      }
   }
