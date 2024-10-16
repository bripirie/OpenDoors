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
 *     Filename : ODSTAT.C
 *  Description : Module providing support functions used by various
 *                "personalities".
 *      Version : 5.00
 */


#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"

#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>



char _status_work_str[80];



void _add_key(int code)
   {
   if(od_control.od_num_keys<16) od_control.od_hot_key[od_control.od_num_keys++]=code;
   }



void _remove_key(int code)
   {
   register char counter;

   for(counter=0;counter<od_control.od_num_keys;++counter)
      if(od_control.od_hot_key[counter]==code)
         {
         if(counter!=od_control.od_num_keys-1)
            od_control.od_hot_key[counter]=od_control.od_hot_key[od_control.od_num_keys-1];
         --od_control.od_num_keys;
         return;
         }
   }



char *user_age(void)
   {
   static char age_string[7];
   unsigned char age;
   int temp;
   time_t timer;
   struct tm *tblock;

   if(od_control.od_info_type==RA1EXITINFO || od_control.od_info_type==RA2EXITINFO || od_control.od_info_type==DOORSYS_WILDCAT)
      {
      age=atoi(od_control.ra_birthday)-1;

      if(strlen(od_control.ra_birthday)==8 && age>=0 && age<=11)
         if(od_control.ra_birthday[6]>='0' && od_control.ra_birthday[6]<='9' && od_control.ra_birthday[7]>='0' && od_control.ra_birthday[7]<='9')
            if(od_control.ra_birthday[3]>='0' && od_control.ra_birthday[3]<='3' && od_control.ra_birthday[4]>='0' && od_control.ra_birthday[4]<='9')
               {
               timer=time(NULL);
               tblock=localtime(&timer);

               temp=(tblock->tm_year % 100) - atoi((char *)od_control.ra_birthday+6);
               if(temp<0) age=temp+100; else age=temp;

               temp=atoi(od_control.ra_birthday)-1;
               if(tblock->tm_mon<temp)
                  --age;
               else if(tblock->tm_mon == temp)
                  {
                  temp=atoi((char *)od_control.ra_birthday+3);

                  if(tblock->tm_mday<temp) --age;
                  }

               sprintf(age_string,"%d",age);
               return((char *)age_string);
               }
      }

   return("?");
   }
