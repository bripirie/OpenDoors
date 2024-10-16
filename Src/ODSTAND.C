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
 *     Filename : ODSTAND.C
 *  Description : Contains the code for the OpenDoors standard status line
 *                personality definition.
 *      Version : 5.00
 */



#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"


void pdef_opendoors(unsigned char which)
   {
   switch(which)
      {
      case 0:
         phys_setattrib(0x70);
         phys_gotoxy(1,24);                 /* display status line */
         phys_cputs(od_control.od_status_line[0]);
         phys_gotoxy(1,24);
         phys_cprintf(od_control.od_status_line[1],od_control.user_name,od_control.user_location,od_control.baud);
         phys_gotoxy(77,24);
         if(od_control.od_node<1000)
            {
            phys_cprintf("%d]",od_control.od_node);
            }
         else
            {
            phys_cputs("?]");
            }
         phys_gotoxy(1,25);
         phys_cputs(od_control.od_status_line[2]);

         phys_puttext(80,25,80,25,block);

         phys_gotoxy(11,25);
         phys_cprintf("%u",od_control.user_security);
         phys_gotoxy(24,25);
         phys_cprintf(od_control.od_time_left,od_control.caller_timelimit);
         if(od_control.caller_ansi)
            {
            phys_gotoxy(40,25);
            phys_cputs("[ANSI]");
            }

         if(od_control.od_avatar)
            {
            phys_gotoxy(47,25);
            phys_cputs("[AVT]");
            }

         if(od_control.ra_sysop_next)
            {
            phys_gotoxy(35,25);
            phys_cputs(od_control.od_sysop_next);
            }

         if(od_control.caller_wantchat)
            {
            phys_gotoxy(57,25);
            phys_setattrib(0xf0);
            phys_cputs(od_control.od_want_chat);
            }

         if(!od_control.od_user_keyboard_on)
            {
            phys_gotoxy(58,24);
            phys_setattrib(0xf0);
            phys_cputs(od_control.od_no_keyboard);
            }
         break;

      case 7:
         phys_setattrib(0x70);
         phys_puttext(80,25,80,25,block);
         phys_gotoxy(1,24);                  /* display help info */
         phys_cputs(od_control.od_help_text);
         phys_gotoxy(1,25);                  /* display copyright info */
         if(_cP_)
            {
            phys_cputs(od_control.od_help_text2);
            }
         else
            {
            phys_cputs("  OpenDoors 5.00  *WARNING* Unregistered Version - Limit 1 month trial period! ");
            }
         break;

      case 10:
         phys_setattrib(0x70);               /* set color to reverse video */
         phys_gotoxy(24,25);                /* display time left */

         phys_cprintf(od_control.od_time_left,od_control.caller_timelimit);

         phys_gotoxy(35,25);
         if(od_control.ra_sysop_next)  /* Display sysop next setting */
            {
            phys_cputs(od_control.od_sysop_next);
            }
         else
            {
            phys_cputs("     ");
            }

         if(od_control.caller_ansi)    /* display ANSI mode setting */
            {
            phys_cputs("[ANSI] ");
            }
         else
            {
            phys_cputs("       ");
            }

         if(od_control.od_avatar)      /* display AVATAR mode setting */
            {
            phys_cputs("[AVT] ");
            }
         else
            {
            phys_cputs("      ");
            }

         phys_gotoxy(58,24);                /* Display user keyboard setting */
         if(od_control.od_user_keyboard_on)
            {
            phys_cputs("          ");
            }
         else
            {
            phys_setattrib(0xf0);
            phys_cputs(od_control.od_no_keyboard);
            }

         phys_gotoxy(57,25);                /* display want-chat setting */
         if(od_control.caller_wantchat)
            {
            phys_setattrib(0xf0);
            phys_cputs(od_control.od_want_chat);
            }
         else
            {
            phys_cputs("           ");
            }
         break;

      case 20:
         od_control.key_hangup=0x2300;       /* Hangup key */
         od_control.key_drop2bbs=0x2000;     /* Drop back to BBS key */
         od_control.key_dosshell=0x2400;     /* Shell to DOS key */
         od_control.key_chat=0x2e00;         /* Chat mode key */
         od_control.key_sysopnext=0x3100;    /* Sysop next key */
         od_control.key_lockout=0x2600;      /* User lockout key */
         od_control.key_status[0]=0x3b00;    /* Key to select default status line */
         od_control.key_status[1]=0x0000;    /* Key to select alternate status line 1 */
         od_control.key_status[2]=0x0000;    /* Key to select alternate status line 2 */
         od_control.key_status[3]=0x0000;    /* Key to select alternate status line 3 */
         od_control.key_status[4]=0x0000;    /* Key to select alternate status line 4 */
         od_control.key_status[5]=0x0000;    /* Key to select alternate status line 5 */
         od_control.key_status[6]=0x0000;    /* Key to select alternate status line 6 */
         od_control.key_status[7]=0x4300;    /* Key to select alternate status line 7 */
         od_control.key_status[8]=0x4400;    /* Key to turn off status line entirely */
         od_control.key_keyboardoff=0x2500;  /* Key to turn to disable remote input */
         od_control.key_moretime=0x4800;     /* Key to add 1 minute to user's time */
         od_control.key_lesstime=0x5000;     /* Key to subtract 1 minute from time */
         od_control.od_page_statusline=-1;
      }
   }
