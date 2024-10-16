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
 *     Filename : ODWCAT.C
 *  Description : WildCat! Personality definition function
 *      Version : 5.00
 */



#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <time.h>

#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"


void pdef_wildcat(unsigned char which)
   {
   register char info=od_control.od_info_type;

   switch(which)
      {
      case 0:
         phys_setattrib(0x70);
         phys_gotoxy(1,24);
         phys_cputs("                                    Baud:                                       ");
         phys_gotoxy(1,25);
         phys_cputs("Phone:                               Sec:                      Time Left:      ");
         phys_puttext(80,25,80,25,block);

         phys_setattrib(0x71);
         phys_gotoxy(1,24);
         sprintf(_status_work_str,"(%s), ",od_control.user_name,od_control.user_location);
         phys_cprintf("%34.34s",_status_work_str);
         phys_gotoxy(43,24);
         phys_cprintf("%u",od_control.baud);

         phys_gotoxy(8,25);
         if(od_control.od_extended_info || info==SFDOORSDAT || info==DOORSYS_GAP || info==DOORSYS_WILDCAT)
            {
            phys_cputs(od_control.user_homephone);
            }

         if(info==RA1EXITINFO || info==RA2EXITINFO || info==DOORSYS_WILDCAT)
            {
            phys_gotoxy(28,25);
            phys_setattrib(0x70);
            phys_cputs("Age: ");
            phys_setattrib(0x71);
            phys_cputs(user_age());
            }

         phys_gotoxy(43,25);
         phys_cprintf("%u",od_control.user_security);

         if(info==RA1EXITINFO || info==RA2EXITINFO || info==DOORSYS_WILDCAT)
            {
            if(strlen(od_control.user_firstcall)==8)
               {
               phys_gotoxy(49,25);
               phys_setattrib(0x70);
               phys_cputs("Since: ");
               phys_setattrib(0x71);
               phys_putch(od_control.user_firstcall[0]);
               phys_putch(od_control.user_firstcall[1]);
               phys_putch('/');
               phys_putch(od_control.user_firstcall[6]);
               phys_putch(od_control.user_firstcall[7]);
               }
            }

      case 10:
         phys_setattrib(0x71);
         phys_gotoxy(74,25);
         if(od_control.user_timelimit<=9)
            {
            phys_cprintf("   %d",od_control.user_timelimit);
            }
         else if(od_control.user_timelimit<=99)
            {
            phys_cprintf("  %d",od_control.user_timelimit);
            }
         else if(od_control.user_timelimit<=999)
            {
            phys_cprintf(" %d",od_control.user_timelimit);
            }
         else
            {
            phys_cprintf("%d",od_control.user_timelimit);
            }

         phys_setattrib(0x70);
         phys_gotoxy(56,24);

         if(od_control.od_okaytopage==TRUE)
            phys_cputs("Page Bell ");
         else
            phys_cputs("          ");

         if(od_control.od_user_keyboard_on)
            phys_cputs("Kybd ");
         else
            phys_cputs("     ");

         if(od_control.ra_sysop_next)
            phys_cputs("Local-Next");
         else
            phys_cputs("          ");
         break;

      case 20:
         od_control.key_hangup=0x0000;
         od_control.key_drop2bbs=0x4400;     /* Drop back to BBS key */
         od_control.key_dosshell=0x2000;     /* Shell to DOS key */
         od_control.key_chat=0x4100;         /* Chat mode key */
         od_control.key_sysopnext=0x3b00;    /* Sysop next key */
         od_control.key_lockout=0x8100;      /* User lockout key */
         od_control.key_status[0]=0x0000;
         od_control.key_status[1]=0x0000;
         od_control.key_status[2]=0x0000;
         od_control.key_status[3]=0x0000;
         od_control.key_status[4]=0x0000;
         od_control.key_status[5]=0x0000;
         od_control.key_status[6]=0x0000;
         od_control.key_status[7]=0x0000;
         od_control.key_status[8]=0x0000;
         od_control.key_keyboardoff=0x2500;  /* Key to disable remote input */
         od_control.key_moretime=0x0000;
         od_control.key_lesstime=0x0000;
         od_control.od_page_statusline=-1;
         _add_key(0x4200);                   /* Key to end chat */
         _add_key(0x4800);                   /* Key to add five minutes */
         _add_key(0x5000);                   /* key to subtract five minutes */
         _add_key(0x7800);                   /* key to hangup */
         _add_key(0x7900);                   /* key to hangup */
         _add_key(0x7a00);                   /* key to hangup */
         _add_key(0x7b00);                   /* key to hangup */
         _add_key(0x7c00);                   /* key to hangup */
         _add_key(0x7d00);                   /* key to hangup */
         _add_key(0x7e00);                   /* key to hangup */
         _add_key(0x7f00);                   /* key to hangup */
         _add_key(0x8000);                   /* key to hangup */
         _add_key(0x3f00);                   /* key to toggle bell */
         _add_key(0x3e00);                   /* key to toggle bell */
         break;

      case 21:
         switch(od_control.od_last_hot)
            {
            case 0x4200:                   /* Key to end chat */
               od_control.od_chat_active=FALSE;
               break;

            case 0x4800:                   /* Key to add five minutes */
               if(od_control.user_timelimit<=1435)
                  {
                  od_control.user_timelimit+=5;
                  _force_update=TRUE;
                  od_kernal();
                  }
               break;

            case 0x5000:                   /* key to subtract five minutes */
               od_control.user_timelimit-=5;
               _force_update=TRUE;
               od_kernal();
               break;

            case 0x7800:                   /* key to hangup */
            case 0x7900:
            case 0x7a00:
            case 0x7b00:
            case 0x7c00:
            case 0x7d00:
            case 0x7e00:
            case 0x7f00:
            case 0x8000:
               od_exit(2,TRUE);
               break;

            case 0x3f00:                   /* key to toggle bell */
            case 0x3e00:
               if(od_control.od_okaytopage!=TRUE)
                  od_control.od_okaytopage=TRUE;
               else
                  od_control.od_okaytopage=FALSE;
               _force_update = TRUE;
               od_kernal();
               break;

            default:
               return;
            }
         od_control.od_last_hot=0;
         break;

      case 22:
         _remove_key(0x4200);
         _remove_key(0x4800);
         _remove_key(0x5000);
         _remove_key(0x7800);
         _remove_key(0x7900);
         _remove_key(0x7a00);
         _remove_key(0x7b00);
         _remove_key(0x7c00);
         _remove_key(0x7d00);
         _remove_key(0x7e00);
         _remove_key(0x7f00);
         _remove_key(0x8000);
         _remove_key(0x3e00);
         _remove_key(0x3f00);
      }
   }
