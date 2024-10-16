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
 *     Filename : ODRA.C
 *  Description : Code for RemoteAccess personality
 *      Version : 5.00
 */



#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>

#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"



static void _ra_paging(void);                      /* static function prototypes */
static void display_date(char *string);
static void display_flags(unsigned char flags);
static void display_time(void);


char _ra_was_on=FALSE;


void pdef_ra(unsigned char which)
   {
   register char info=od_control.od_info_type;

   switch(which)
      {
      case 0:
         phys_setattrib(0x70);
         phys_gotoxy(1,24);                 /* display status line */
         phys_cputs("                                                                     (Node      ");
         phys_gotoxy(1,24);
         phys_cprintf("%s of %s at %u BPS",od_control.user_name,od_control.user_location,od_control.baud);

         if(!od_control.od_user_keyboard_on)
            {
            phys_gotoxy(49,24);
            phys_setattrib(0x99);
            phys_cputs("(Keyboard)");
            phys_setattrib(0x70);
            _ra_was_on=TRUE;
            }

         _ra_paging();

         phys_gotoxy(76,24);
         if(od_control.od_node<1000)
            {
            phys_cprintf("%u)",od_control.od_node);
            }
         else
            {
            phys_cputs("?)");
            }
         phys_gotoxy(1,25);
         phys_cputs("Security:        Time:                                               (F9)=Help ");

         phys_puttext(80,25,80,25,block);

         phys_gotoxy(11,25);
         phys_cprintf("%u",od_control.user_security);
         phys_gotoxy(24,25);
         phys_cprintf("%d mins   ",od_control.caller_timelimit);
         if(od_control.caller_ansi)
            {
            phys_gotoxy(42,25);
            phys_cputs("(ANSI)");
            }

         if(od_control.od_avatar)
            {
            phys_gotoxy(48,25);
            phys_cputs("(AVT)");
            }

         if(od_control.ra_sysop_next)
            {
            phys_gotoxy(53,25);
            phys_cputs("(SN) ");
            }

         if(od_control.caller_wantchat)
            {
            phys_gotoxy(57,25);
            phys_setattrib(0x99);
            phys_cputs("(Wants Chat)");
            phys_setattrib(0x70);
            }
         break;


      case 1:
         phys_setattrib(0x70);
         phys_puttext(80,25,80,25,block);
         phys_gotoxy(1,24);
         phys_cputs("Voice#:               Last Call   :                       First Call:           ");
         phys_gotoxy(1,25);
         phys_cputs("Data #:               Times Called:            Age:        Birthdate:          ");
         if(od_control.od_extended_info || info==SFDOORSDAT || info==DOORSYS_GAP || info==DOORSYS_WILDCAT)
            {
            phys_gotoxy(8,24);
            phys_cprintf("%13.13s",od_control.user_homephone);
            }
         if(od_control.od_extended_info || info==DOORSYS_GAP || info==DOORSYS_WILDCAT)
            {
            phys_gotoxy(8,25);
            phys_cprintf("%13.13s",od_control.user_dataphone);
            }
         if(od_control.od_extended_info || info==DOORSYS_GAP || info==CHAINTXT || info==DOORSYS_WILDCAT)
            {
            phys_gotoxy(37,24);
            display_date(od_control.user_lastdate);
            }
         if(od_control.od_extended_info || info==DOORSYS_GAP || info==DOORSYS_WILDCAT)
            {
            phys_gotoxy(37,25);
            phys_cprintf("%lu",od_control.user_numcalls);
            }
         if(info==RA1EXITINFO || info==RA2EXITINFO || info==DOORSYS_WILDCAT)
            {
            phys_gotoxy(53,25);
            phys_cputs((char *)user_age());
            phys_gotoxy(71,24);
            display_date(od_control.ra_firstcall);
            phys_gotoxy(71,25);
            display_date(od_control.ra_birthday);
            }
         break;


      case 2:
         phys_setattrib(0x70);
         phys_puttext(80,25,80,25,block);
         phys_gotoxy(1,24);
         if(od_control.od_extended_info || info==SFDOORSDAT || info==DOORSYS_GAP || info==DOORSYS_WILDCAT)
            {
            phys_cputs("Uploads:                Downloads:               Tagged: 0k (0)                 ");
            if(info==DOORSYS_GAP)
               {
               phys_gotoxy(10,24);
               phys_cprintf("%lu",od_control.user_uploads);
               phys_gotoxy(36,24);
               phys_cprintf("%lu",od_control.user_downloads);
               }
            else
               {
               phys_gotoxy(10,24);
               phys_cprintf("%luk (%lu)",od_control.user_upk,od_control.user_uploads);
               phys_gotoxy(36,24);
               phys_cprintf("%luk (%lu)",od_control.user_downk,od_control.user_downloads);
               }
            }
         else
            {
            phys_cputs("                                                                                ");
            }
         phys_gotoxy(1,25);
         if(od_control.od_extended_info)
            {
            phys_cputs("Flags: (A):--------  (B):--------  (C):--------  (D):--------                  ");
            phys_gotoxy(12,25);
            display_flags(od_control.user_flags[0]);
            phys_gotoxy(26,25);
            display_flags(od_control.user_flags[1]);
            phys_gotoxy(40,25);
            display_flags(od_control.user_flags[2]);
            phys_gotoxy(54,25);
            display_flags(od_control.user_flags[3]);
            }
         else
            {
            phys_cputs("                                                                               ");
            }
         break;


      case 3:
         phys_setattrib(0x70);
         phys_puttext(80,25,80,25,block);
         phys_gotoxy(1,24);
         phys_cputs("                                                                   (Time      ) ");
         if(od_control.od_extended_info)
            {
            phys_gotoxy(1,24);
            phys_cprintf("Last Caller: %s    Total System Calls: %lu",od_control.system_last_caller,od_control.system_calls);
            }
         display_time();

         phys_gotoxy(1,25);
         if(od_control.od_extended_info || info==DOORSYS_WILDCAT)
            {
            phys_cputs("(Printer OFF)      (Local Screen ON )        Next Event                        ");
            phys_gotoxy(57,25);
            if(od_control.event_status==ES_ENABLED || info==DOORSYS_WILDCAT)
               {
               phys_cprintf("(%s, errorlevel %u)",od_control.event_starttime,od_control.event_errorlevel);
               }
            else
               {
               phys_cputs("none, errorlevel 0");
               }
            }
         else
            {
            phys_cputs("                                                                               ");
            }
         break;


      case 4:
         phys_setattrib(0x70);
         phys_puttext(80,25,80,25,block);
         phys_gotoxy(1,24);
         if(od_control.od_extended_info || info==DOORSYS_WILDCAT)
            {
            phys_cputs("Msgs posted    :           Highread :                                Group 1    ");
            phys_gotoxy(18,24);
            phys_cprintf("%u",od_control.user_messages);
            phys_gotoxy(39,24);
            phys_cprintf("%u",od_control.user_lastread);
            if(info==RA1EXITINFO || info==RA2EXITINFO)
               {
               phys_gotoxy(76,24);
               phys_cprintf("%u",od_control.user_group);
               }
            }
         else
            {
            phys_cputs("                                                                                ");
            }

         phys_gotoxy(1,25);
         if(od_control.od_extended_info || info==CHAINTXT || info==DOORSYS_WILDCAT)
            {
            phys_cputs("Credit         :           Handle   :                                          ");
            if(info==EXITINFO || info==RA1EXITINFO || info==RA2EXITINFO)
               {
               phys_gotoxy(18,25);
               phys_cprintf("%u.00",(unsigned int)od_control.user_credit);
               }
            if(info==CHAINTXT || info==RA1EXITINFO || info==RA2EXITINFO || info==DOORSYS_WILDCAT)
               {
               phys_gotoxy(39,25);
               phys_cputs(od_control.ra_userhandle);
               }
            }
         else
            {
            phys_cputs("                                                                               ");
            }
         break;


      case 5:
         phys_setattrib(0x70);
         phys_gotoxy(1,24);
         phys_cputs("                                                                                ");
         phys_gotoxy(1,25);
         phys_cputs("                                                                               ");
         phys_puttext(80,25,80,25,block);
         if(info==RA1EXITINFO || info==RA2EXITINFO || info==DOORSYS_WILDCAT)
            {
            phys_gotoxy(1,24);
            phys_cputs(od_control.ra_comment);
            }
         if(od_control.caller_wantchat && strlen(od_control.user_reasonforchat)!=0)
            {
            phys_gotoxy(1,25);
            strcpy(_status_work_str,od_control.user_reasonforchat);
            _status_work_str[69-strlen(od_control.user_name)]='\0';
            phys_cprintf("Chat (%s): %s",od_control.user_name,_status_work_str);
            }
         break;

      case 6:
         phys_setattrib(0x70);
         phys_gotoxy(1,24);
         phys_cputs("                                                                                ");
         phys_gotoxy(1,25);
         phys_cputs("                                                                               ");
         phys_puttext(80,25,80,25,block);
         break;

      case 7:
         phys_setattrib(0x70);
         phys_puttext(80,25,80,25,block);
         phys_gotoxy(1,24);                  /* display help info */
         phys_cputs("ALT: (C)hat (H)angup (J)Shell (L)ockOut (K)eyboard (N)extOn (D)rop To BBS       ");
         phys_cputs("                                   -Inc Time -Dec Time  (F1)-(F7)=Extra Stats");
         break;

      case 10:
         phys_setattrib(0x70);               /* set color to reverse video */
         phys_gotoxy(24,25);                /* display time left */

         phys_cprintf("%d mins   ",od_control.caller_timelimit);

         phys_gotoxy(42,25);
         if(od_control.caller_ansi)    /* display ANSI mode setting */
            {
            phys_cputs("(ANSI)");
            }
         else
            {
            phys_cputs("      ");
            }

         if(od_control.od_avatar)      /* display AVATAR mode setting */
            {
            phys_cputs("(AVT)");
            }
         else
            {
            phys_cputs("     ");
            }

         if(od_control.ra_sysop_next)  /* Display sysop next setting */
            {
            phys_cputs("(SN)");
            }
         else
            {
            phys_cputs("    ");
            }

         if(od_control.caller_wantchat)
            {
            phys_setattrib(0x99);
            phys_cputs("(Wants Chat)");
            phys_setattrib(0x70);
            }
         else
            {
            phys_cputs("            ");
            }

         _ra_paging();

         if(od_control.od_user_keyboard_on && _ra_was_on)
            {
            phys_gotoxy(49,24);
            phys_cputs("          ");
            }
         if(!od_control.od_user_keyboard_on)
            {
            _ra_was_on=TRUE;
            phys_gotoxy(49,24);
            phys_setattrib(0x99);
            phys_cputs("(Keyboard)");
            phys_setattrib(0x70);
            }

         break;

      case 13:
         phys_setattrib(0x70);               /* set color to reverse video */
         display_time();
         break;

      case 20:
         _ra_status = TRUE;
         od_control.key_hangup=0x2300;       /* Hangup key */
         od_control.key_drop2bbs=0x2000;     /* Drop back to BBS key */
         od_control.key_dosshell=0x2400;     /* Shell to DOS key */
         od_control.key_chat=0x2e00;         /* Chat mode key */
         od_control.key_sysopnext=0x3100;    /* Sysop next key */
         od_control.key_lockout=0x2600;      /* User lockout key */
         od_control.key_status[0]=0x3b00;    /* Key to select default status line */
         od_control.key_status[1]=0x3c00;    /* Key to select alternate status line 1 */
         od_control.key_status[2]=0x3d00;    /* Key to select alternate status line 2 */
         od_control.key_status[3]=0x3e00;    /* Key to select alternate status line 3 */
         od_control.key_status[4]=0x3f00;    /* Key to select alternate status line 4 */
         od_control.key_status[5]=0x4000;    /* Key to select alternate status line 5 */
         od_control.key_status[6]=0x4100;    /* Key to select alternate status line 6 */
         od_control.key_status[7]=0x4300;    /* Key to select alternate status line 7 */
         od_control.key_status[8]=0x4400;    /* Key to turn off status line entirely */
         od_control.key_keyboardoff=0x2500;  /* Key to turn to disable remote input */
         od_control.key_moretime=0x4800;     /* Key to add 1 minute to user's time */
         od_control.key_lesstime=0x5000;     /* Key to subtract 1 minute from time */
         od_control.od_page_statusline=5;
      }
   }


static void _ra_paging(void)
   {
   time_t timer;
   struct tm *tblock;
   char failed=FALSE;
   int minute;

   phys_gotoxy(60,24);

   switch(od_control.od_okaytopage)
      {
      case TRUE:
         phys_setattrib(0x19);
         phys_cputs("(PAGE ON) ");
         phys_setattrib(0x70);
         break;

      case FALSE:
         phys_setattrib(0x19);
         phys_cputs("(PAGE OFF)");
         phys_setattrib(0x70);
         break;

      case MAYBE:
         timer=time(NULL);
         tblock=localtime(&timer);
         minute=(60*tblock->tm_hour)+tblock->tm_min;
         if(od_control.od_pagestartmin < od_control.od_pageendmin)
            {
            if(minute<od_control.od_pagestartmin || minute>=od_control.od_pageendmin) failed=TRUE;
            }
         else
            {
            if(minute<od_control.od_pagestartmin && minute>=od_control.od_pageendmin) failed=TRUE;
            }

         if(failed)
            {
            phys_cputs("(PAGE OFF)");
            }
         else
            {
            phys_cputs("(PAGE ON) ");
            }
      }
   }




static void display_time(void)
   {
   time_t timer;
   struct tm *tblock;

   timer=time(NULL);
   tblock=localtime(&timer);
   phys_gotoxy(74,24);
   phys_cprintf("%02.2d:%02.2d",tblock->tm_hour,tblock->tm_min);
   }



static void display_date(char *string)
   {
   register int month;
   register int temp;

   if(strlen(string)!=8) return;

   month=atoi(string)-1;
   if(month<0 || month>11) return;

   temp=atoi((char *)string+3);
   if(temp<1 || temp>31) return;

   if(string[6]<'0' || string[6]>'9' || string[7]<'0' || string[7]>'9') return;

   phys_putch(string[3]);
   phys_putch(string[4]);
   phys_putch('-');
   phys_cputs(od_control.od_month[month]);
   phys_putch('-');
   phys_putch(string[6]);
   phys_putch(string[7]);
   }



static void display_flags(unsigned char flags)
   {
   register unsigned char mask=0x01;
   register char counter;

   for(counter=0;counter<8;++counter)
      {
      if(flags&mask)
         {
         phys_putch('X');
         }
      else
         {
         phys_putch('-');
         }
      mask<<=1;
      }
   }
