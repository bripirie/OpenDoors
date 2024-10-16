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
 *     Filename : ODPRNTF.C
 *  Description : Contains the implementation of the od_printf() function.
 *      Version : 5.00
 */




#include "opendoor.h"              /* OpenDoor global structure & prototypes */
#include "odintern.h"

#include<stdio.h>                      /* Standard header files */
#include<stdarg.h>
#include<alloc.h>
#include<string.h>



void od_printf(char *format,...)       /* printf function which outputs to */
   {                                 /*  the door terminal (local & remote) */
   va_list arg_pointer;                /* Pointer to the list of arguments */
   static char *string=NULL;                  /* String to display to user */
   register char *strptr;
   register char *start;
   register char notfound;
   register char counter;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_printf()");

   if(!inited) od_init();

   if(string==NULL)
      if ((string=malloc(512))==NULL) return;

   va_start(arg_pointer,format);       /* Copy the arguments after format */
   vsprintf(string,format,arg_pointer);/* Perform printf to string */
   va_end(arg_pointer);

   if(!od_control.od_colour_char && !od_control.od_colour_delimiter)
       goto quick_print;

   colour_check_char=od_control.od_colour_delimiter;

   notfound=TRUE;
   strptr=(char *)string;
   start=(char *)string;
   counter=0;
   while(*strptr)
      {
      if(*strptr==od_control.od_colour_delimiter)
         {
         notfound=FALSE;

         if(counter!=0)
            {
            od_disp(start,counter,TRUE);
            }

         if(!*(++strptr))
            {
            colour_check_char=0;
            return;
            }
         od_set_attrib(od_colour_config(strptr));
         if(!*(strptr=(char *)colour_end_pos))
            {
            colour_check_char=0;
            return;
            }

         if(!*(++strptr)) return;
         start=(char *)strptr;
         counter=0;
         }

      else if(*strptr==od_control.od_colour_char)
         {
         notfound=FALSE;

         if(counter!=0)
            {
            od_disp(start,counter,TRUE);
            }

         if(!*(++strptr)) return;
         od_set_attrib(*strptr);

         if(!*(++strptr)) return;
         start=(char *)strptr;
         counter=0;
         }
      else
         {
         ++counter;
         ++strptr;
         }
      }

   colour_check_char=0;

   if(notfound)
      {
quick_print:
      od_disp_str(string);             /* Display the string */
      }
   else if(counter!=0)
      {
      od_disp(start,counter,TRUE);
      }
   }
