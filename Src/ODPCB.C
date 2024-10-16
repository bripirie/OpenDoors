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
 *     Filename : ODPCB.C
 *  Description : Code for PC-Board personality
 *      Version : 5.00
 */



#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"

#include <string.h>



void pdef_pcboard(unsigned char which)
   {
   static char pcb_work_str[81];
   register char info=od_control.od_info_type;


   switch(which)
      {
      case 0:                               /* Create status line 0 */
         phys_setattrib(0x70);
         phys_gotoxy(1,24);
         phys_cputs("                                                      ALT-H=Help                                                                                                ");
         phys_gotoxy(3,24);
         if(od_control.baud != 0)
            {
            phys_cprintf("(%ld) ", od_control.baud);
            }
         else
            {
            phys_cputs("(Local) ");
            }
         sprintf(pcb_work_str,"%s - %s",od_control.user_name,od_control.user_location);
         pcb_work_str[42]='\0';
         strupr(pcb_work_str);
         phys_cputs(pcb_work_str);
         phys_gotoxy(1,25);
         if(od_control.user_ansi || od_control.user_avatar || od_control.user_rip)
            {
            phys_putch('G');
            }
         else
            {
            phys_putch('A');
            }
         if(info==RA1EXITINFO || info==RA2EXITINFO || info==DOORSYS_WILDCAT)
            {
            phys_cprintf(" (%s)",od_control.user_firstcall);
            }
         phys_gotoxy(15, 25);
         phys_cprintf("Sec(0)=%u  ",od_control.user_security);
         if(od_control.od_extended_info || info==DOORSYS_GAP || info==CHAINTXT || info==DOORSYS_WILDCAT)
            {
            phys_cprintf("Times On=%u  ",od_control.user_numcalls);
            }
         if(od_control.od_extended_info || info==SFDOORSDAT || info==DOORSYS_GAP || info==DOORSYS_WILDCAT)
            {
            phys_cprintf("Up:Dn=%lu:%lu",od_control.user_uploads,od_control.user_downloads);
            }
         phys_gotoxy(70,25);
         phys_cprintf("%4d", od_control.user_timelimit);

         od_control.key_status[0]=0x0000;    /* Key to select default status line */
         od_control.key_status[1]=0x2300;    /* Key to select alternate status line 1 */
         break;

      case 1:
         phys_setattrib(0x70);
         phys_gotoxy(1,24);
         phys_cputs("  Alt-> N=Next X=DOS F1/F2=Time                                                   2=LkOut 5=SHELL 8=HngUp 10=Chat                                               ");

         od_control.key_status[0]=0x2300;    /* Key to select default status line */
         od_control.key_status[1]=0x0000;    /* Key to select alternate status line 1 */
         break;

      case 10:                               /* Update status line 0 */
         phys_setattrib(0x70);
         phys_gotoxy(70,25);
         phys_cprintf("%4d", od_control.user_timelimit);
         break;

      case 11:                               /* Update status line 1 */
         break;

      case 20:                               /* Install personality */
         od_control.key_hangup=0x4200;       /* Hangup key */
         od_control.key_drop2bbs=0x2d00;     /* Drop back to BBS key */
         od_control.key_dosshell=0x3f00;     /* Shell to DOS key */
         od_control.key_chat=0x4400;         /* Chat mode key */
         od_control.key_sysopnext=0x3100;    /* Sysop next key */
         od_control.key_lockout=0x3c00;      /* User lockout key */
         od_control.key_status[0]=0x0000;    /* Key to select default status line */
         od_control.key_status[1]=0x2300;    /* Key to select alternate status line 1 */
         od_control.key_status[2]=0x0000;    /* Key to select alternate status line 2 */
         od_control.key_status[3]=0x0000;    /* Key to select alternate status line 3 */
         od_control.key_status[4]=0x0000;    /* Key to select alternate status line 4 */
         od_control.key_status[5]=0x0000;    /* Key to select alternate status line 5 */
         od_control.key_status[6]=0x0000;    /* Key to select alternate status line 6 */
         od_control.key_status[7]=0x0000;    /* Key to select alternate status line 7 */
         od_control.key_status[8]=0x0000;    /* Key to turn off status line entirely */
         od_control.key_keyboardoff=0x2500;  /* Key to turn to disable remote input */
         od_control.key_moretime=0x0000;     /* Key to add 1 minute to user's time */
         od_control.key_lesstime=0x0000;     /* Key to subtract 1 minute from time */
         _add_key(0x6900);                   /* Key to add five minutes */
         _add_key(0x6800);                   /* key to subtract five minutes */
         od_control.od_page_statusline=0;
         break;

      case 21:
         switch(od_control.od_last_hot)
            {
            case 0x6900:                   /* Key to add five minutes */
               if(od_control.user_timelimit<=1435 &&
                  od_control.user_timelimit >= 5)
                  {
                  od_control.user_timelimit+=5;
                  _force_update=TRUE;
                  od_kernal();
                  }
               else if(od_control.user_timelimit < 5)
                  {
                  od_control.user_timelimit++;
                  _force_update=TRUE;
                  od_kernal();
                  }
               break;

            case 0x6800:                   /* key to subtract five minutes */
               if(od_control.user_timelimit > 5)
               {
                  od_control.user_timelimit-=5;
                  _force_update=TRUE;
                  od_kernal();
               }
               else if(od_control.user_timelimit > 1)
               {
                  --od_control.user_timelimit;
                  _force_update=TRUE;
                  od_kernal();
               }
               break;
            default:
               return;
            }
         od_control.od_last_hot=0;
         break;
      }
   }
