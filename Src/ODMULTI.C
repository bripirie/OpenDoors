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
 *     Filename : ODMULTI.C
 *  Description : Code composing the optional Multiple Personality sub-system.
 *      Version : 5.00
 */



#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <time.h>

#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"



struct personality_struct
   {
   char name[33];
   char output_top;
   char output_bottom;
   void (*function)(unsigned char which);
   } personality[MAX_PERSONALITIES]=
                              {{"STANDARD",1,23,pdef_opendoors},
                               {"REMOTEACCESS",1,23,pdef_ra},
                               {"WILDCAT",1,23,pdef_wildcat},
                               {"PCBOARD",1,23,pdef_pcboard}};

unsigned char num_personalities=5;
unsigned char current_personality=255;




void option_mps(void)
   {
   _set_personality=od_set_personality;
   }



int od_set_personality(char *name)
   {
   register unsigned char new_personality;
   char set_name[33];
   struct personality_struct *new_record;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_set_personality()");

   if(!inited) od_init();              /* verify that we've been initialized */

   if(strlen(name) == 0) return(FALSE);

   strncpy(set_name,name,32);
   set_name[32]='\0';
   strupr(set_name);

   for(new_personality=0;new_personality<num_personalities;++new_personality)
      {
      if(strcmp(set_name,personality[new_personality].name)==0)
         {
         if(new_personality!=current_personality)      /* check all code needed */
            {
            od_set_statusline(8);
            if(current_personality!=255)
               (*current_status_function)(22);
            od_control.od_page_statusline=-1;
            new_record=&personality[current_personality=new_personality];
            _ra_status = TRUE;
            (*new_record->function)(20);
            phys_window(1,new_record->output_top,80,new_record->output_bottom);
            current_status_function=new_record->function;
            status_line=-1;
            od_set_statusline(0);
            }

         return(TRUE);
         }
      }

   return(FALSE);
   }



int od_add_personality(char *name, char output_top, char output_bottom, void (*function)(unsigned char which))
   {
   if(num_personalities==MAX_PERSONALITIES)
      {
      od_control.od_error = ERR_LIMIT;
      return(FALSE);
      }

   strncpy(personality[num_personalities].name,name,32);
   personality[num_personalities].name[32]='\0';
   strupr(personality[num_personalities].name);
   personality[num_personalities].output_top=output_top;
   personality[num_personalities].output_bottom=output_bottom;
   personality[num_personalities].function=function;

   ++num_personalities;

   return(TRUE);
   }
