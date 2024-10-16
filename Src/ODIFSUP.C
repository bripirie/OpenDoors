/*
 * €€€€€€€€€€                         €€€€€€€‹
 * €€€ﬂﬂﬂﬂ€€€                         €€€ﬂﬂﬂ€€€
 * €€€    €€€ ‹‹‹‹‹‹‹ ‹‹‹‹‹‹‹ ‹‹‹‹‹‹‹ €€€   €€€ ‹‹‹‹‹‹‹ ‹‹‹‹‹‹‹ ‹‹‹‹‹‹ ‹‹‹‹‹‹‹
 * €€€    €€€ €€€ﬂ€€€ €€€ﬂ€€€ €€€ﬂ€€€ €€€   €€€ €€€ﬂ€€€ €€€ﬂ€€€ €€€ﬂﬂﬂ €€€ﬂﬂﬂﬂ
 * €€€‹‹‹‹€€€ €€€ €€€ €€€ﬂﬂﬂﬂ €€€ €€€ €€€‹‹‹€€€ €€€ €€€ €€€ €€€ €€€    ﬂﬂﬂﬂ€€€
 * €€€€€€€€€€ €€€€€€€ €€€€€€€ €€€ €€€ €€€€€€€ﬂ  €€€€€€€ €€€€€€€ €€€    €€€€€€€
 *            €€€
 *            €€€
 *            ﬂﬂﬂ                                     Door Programming Toolkit
 * ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
 *
 *      (C) Copyright 1991 - 1994 by Brian Pirie. All Rights Reserved.
 *
 *
 *
 *
 *     Filename : ODIFSUP.C
 *  Description : Contains additional routines used by the ODIFACE
 *                BBS software interface module.
 *      Version : 5.00
 */

#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <alloc.h>
#include <time.h>

#ifndef USEINLINE
#include <dos.h>
#include <dir.h>
#endif

char _preorexit = FALSE;
char _ra_status;

void _mt_init(void)
   {
#ifdef USEINLINE
   ASM       mov ah, 0x30
   ASM       int 0x21
   ASM       cmp al, 0x0a
   ASM       jl  check_dv
#else
   regs.h.ah = 0x30;
   int86( 0x21, &regs, &regs);
   if(regs.h.al < 0x0a) goto check_dv;
#endif

   _multitasker = MULTITASKER_OS2;

check_dv:
   #ifdef __TURBOC__
   #if(__TURBOC__>=0x295)
#ifdef USEINLINE
      ASM    mov cx, 0x4445
      ASM    mov dx, 0x5351
      ASM    mov ax, 0x2b01
      ASM    int 0x21

#else
      _CX=0x4445;
      _DX=0x5351;
      _AX=0x2b01;

      geninterrupt(0x21);
#endif

      if(_AL!=0xff)                         /* if DesqView is resident */
         {
         _multitasker=MULTITASKER_DV;
         }

   if(_multitasker==MULTITASKER_NONE)
      {
#ifdef USEINLINE
      ASM    push di
      ASM    push si
      ASM    mov ax, 0x1600
      ASM    int 0x2f
      ASM    pop si
      ASM    pop di
      ASM    cmp al, 0x00
      ASM    je no_windows
      ASM    cmp al, 0x80
      ASM    je no_windows
      ASM    mov _multitasker, MULTITASKER_WIN
#else
      regs.x.ax = 0x1600;
      int86( 0x2f, &regs, &regs);
      if(regs.h.al != 0x00 && regs.h.al != 0x80)
         {
         _multitasker==MULTITASKER_WIN;
         }
#endif
   }
#endif
#endif
no_windows:
   return;
   }


int _odfindfirst(const char *path, struct find_block *block, int attrib)
   {
#ifdef USEINLINE
   int to_return;

   ASM    push ds
   ASM    mov ah, 0x2f              /* Int 0x21, ah=0x2f: Get current DOS DTA */
   ASM    int 0x21                                         /* Get current DTA */
   ASM    push bx                     /* Store offset of current DTA on stack */
   ASM    push es                    /* Store segment of current DTA on stack */
   ASM    mov ah, 0x1a                  /* Int 0x21, ah=0x1a: Set new DOS DTA */
#ifdef LARGEDATA                                     /* If using far pointers */
   ASM    lds dx, block               /* Load DS:DX with far address of block */
#else                                               /* If using near pointers */
   ASM    mov dx, block                 /* Load DX with near address of block */
#endif
   ASM    int 0x21                                             /* Set DOS DTA */
   ASM    mov ah, 0x4e           /* Int 0x21, ah=0x4e: DOS findfirst function */
   ASM    mov cx, attrib                           /* Load attributes into CX */
#ifdef LARGEDATA                                     /* If using far pointers */
   ASM    lds dx, path                 /* Load DS:DX with far address in path */
#else                                               /* If using near pointers */
   ASM    mov dx, path                   /* Load DX with near address in path */
#endif
   ASM    int 0x21                                 /* Call findfirst function */
   ASM    jc error         /* If carry flag is set, then an error has ocurred */
   ASM    mov word ptr to_return, 0                  /* If no error, return 0 */
   ASM    jmp after_result
error:
   ASM    mov word ptr to_return, -1                   /* If error, return -1 */
after_result:
   ASM    mov ah, 0x1a                  /* Int 0x21, ah=0x1a: Set new DOS DTA */
   ASM    pop ds                     /* Pop original DTA segment off of stack */
   ASM    pop dx                        /* Pop original DTA offest from stack */
   ASM    int 0x21                               /* Reset DOS DTA to original */
   ASM    pop ds                     /* Restore DS stored at function startup */
   return(to_return);
#else
#ifdef __TURBOC__
   return(findfirst(path, (struct ffblk *)block, attrib));
#elif defined(_MSC_VER)
   return(_dos_findfirst(path, (void *)block, attrib));
#else
#error No findfirst known for this compiler, must add one or define USEINLINE
#endif
#endif
   }


int _odfindnext(struct find_block *block)
   {
#ifdef USEINLINE
   int to_return;

   ASM    push ds                                                  /* Save DS */
   ASM    mov ah, 0x2f              /* Int 0x21, ah=0x2f: Get current DOS DTA */
   ASM    int 0x21                                         /* Get current DTA */
   ASM    push bx                     /* Store offset of current DTA on stack */
   ASM    push es                    /* Store segment of current DTA on stack */
   ASM    mov ah, 0x1a                  /* Int 0x21, ah=0x1a: Set new DOS DTA */
#ifdef LARGEDATA                                     /* If using far pointers */
   ASM    lds dx, block               /* Load DS:DX with far address of block */
#else                                               /* If using near pointers */
   ASM    mov dx, block                 /* Load DX with near address of block */
#endif
   ASM    int 0x21                                             /* Set DOS DTA */
   ASM    mov ah, 0x4f            /* Int 0x21, ah=0x4f: DOS findnext function */
   ASM    int 0x21                                 /* Call findfirst function */
   ASM    jc error         /* If carry flag is set, then an error has ocurred */
   ASM    mov word ptr to_return, 0                  /* If no error, return 0 */
   ASM    jmp after_result
error:
   ASM    mov word ptr to_return, -1                   /* If error, return -1 */
after_result:
   ASM    mov ah, 0x1a                  /* Int 0x21, ah=0x1a: Set new DOS DTA */
   ASM    pop ds                     /* Pop original DTA segment off of stack */
   ASM    pop dx                        /* Pop original DTA offest from stack */
   ASM    int 0x21                               /* Reset DOS DTA to original */
   ASM    pop ds                     /* Restore DS stored at function startup */
   return(to_return);
#else
#ifdef __TURBOC__
   return(findnext((struct ffblk *)block));
#elif defined(_MSC_VER)
   return(_dos_findnext((void *)block));
#else
#error No findnext known for this compiler, must add one or define USEINLINE
#endif
#endif
   }

void od_exit(int errorlevel,char term_call) /* function to finish door operation */
   {
   register char counter;
   FILE *fp;
   time_t max_time;
   time_t door_end;
   int winwidth;
   int x1;
   void *under = NULL;
   unsigned long active_minutes;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_exit()");

   /* If user called od_exit() before doing anything else, then we first */
   /* initialize. */
   if(!inited) od_init();

   /* Update remaining time */
   od_control.user_timelimit+=od_control.od_maxtime_deduction;

   /* Calculate deducted time */
   time(&door_end);
   _ulongdiv(&active_minutes, NULL, door_end-door_start, 60L);
   od_control.user_time_used+=((start_remaining-od_control.user_timelimit) - (int)active_minutes);

   /* Reset to original bps rate that was stored in drop file */
   od_control.baud = _file_bps;

   if(od_control.od_before_exit!=NULL) /* if function hook is defined */
      (*od_control.od_before_exit)();  /* call it! */

   if(term_call && od_control.od_hanging_up != NULL)
      {
      winwidth = strlen(od_control.od_hanging_up) + 4;
      x1 = 40 - (winwidth / 2);
      if((under = phys_createwin(x1,10,x1+(winwidth - 1), 14, 0x19, "", 0x19)) != NULL)
         {
         phys_gotoxy((42 - (winwidth / 2)), 12);
         phys_cputs(od_control.od_hanging_up);
         restore_cursor();
         }
      }

   if(_final_dir!=NULL)
      {
      _chdir(_final_dir);
      free(_final_dir);
      _final_dir=NULL;
      }

   if(od_control.od_extended_info)     /* Update EXITINFO.BBS, if applicable */
      {
      if((fp=fopen(makepath(exitpath, "EXITINFO.BBS"),"r+b")) != NULL)
         {
         switch(od_control.od_info_type)
            {
            case RA2EXITINFO:
               ra2exitinfo->baud=(unsigned int)od_control.baud;
               ra2exitinfo->num_calls=od_control.system_calls;
               c2pasc(ra2exitinfo->last_caller,35,od_control.system_last_caller);
               c2pasc(ra2exitinfo->start_date,8,od_control.timelog_start_date);
               memcpy(&ra2exitinfo->busyperhour,&od_control.timelog_busyperhour,62);
               c2pasc(ra2exitinfo->name,35,od_control.user_name);
               c2pasc(ra2exitinfo->location,25,od_control.user_location);
               c2pasc(ra2exitinfo->organisation,50,od_control.user_org);
               for(counter=0;counter<3;++counter)
                  c2pasc(ra2exitinfo->address[counter],50,od_control.user_address[counter]);
               c2pasc(ra2exitinfo->handle,35,od_control.user_handle);
               c2pasc(ra2exitinfo->comment,80,od_control.user_comment);
               ra2exitinfo->password_crc=od_control.user_pwd_crc;
               c2pasc(ra2exitinfo->dataphone,15,od_control.user_dataphone);
               c2pasc(ra2exitinfo->homephone,15,od_control.user_homephone);
               c2pasc(ra2exitinfo->lasttime,5,od_control.user_lasttime);
               c2pasc(ra2exitinfo->lastdate,8,od_control.user_lastdate);
               ra2exitinfo->attrib=od_control.user_attribute;
               ra2exitinfo->attrib2=od_control.user_attrib2;
               memcpy(&ra2exitinfo->flags,&od_control.user_flags,14);
               ra2exitinfo->sec=od_control.user_security;
               ra2exitinfo->lastread=od_control.user_lastread;
               memcpy(&ra2exitinfo->nocalls,&od_control.user_numcalls,29);
               ra2exitinfo->group=od_control.user_group;
               memcpy(&ra2exitinfo->combinedrecord,&od_control.user_combinedrecord,200);
               c2pasc(ra2exitinfo->firstcall,8,od_control.user_firstcall);
               c2pasc(ra2exitinfo->birthday,8,od_control.user_birthday);
               c2pasc(ra2exitinfo->subdate,8,od_control.user_subdate);
               ra2exitinfo->screenwidth=od_control.user_screenwidth;
               ra2exitinfo->language=od_control.user_language;
               ra2exitinfo->dateformat=od_control.user_date_format;
               c2pasc(ra2exitinfo->forwardto,35,od_control.user_forward_to);
               memcpy(&ra2exitinfo->msgarea,&od_control.user_msg_area,15);
               ra2exitinfo->status=od_control.event_status;
               c2pasc(ra2exitinfo->starttime,5,od_control.event_starttime);
               memcpy(&ra2exitinfo->errorlevel,&od_control.event_errorlevel,3);
               c2pasc(ra2exitinfo->lasttimerun,8,od_control.event_last_run);
               memcpy(&ra2exitinfo->netmailentered,&od_control.user_netmailentered,2);
               c2pasc(ra2exitinfo->logintime,5,od_control.user_logintime);
               c2pasc(ra2exitinfo->logindate,8,od_control.user_logindate);
               memcpy(&ra2exitinfo->timelimit,&od_control.user_timelimit,6);
               memcpy(&ra2exitinfo->userrecord,&od_control.user_num,8);
               c2pasc(ra2exitinfo->timeofcreation,5,od_control.user_timeofcreation);
               ra2exitinfo->logonpasswordcrc=od_control.user_logon_pwd_crc;
               ra2exitinfo->wantchat=od_control.user_wantchat;
               ra2exitinfo->deducted_time=od_control.user_deducted_time;
               for(counter=0;counter<50;++counter)
                  c2pasc(ra2exitinfo->menustack[counter],8,od_control.user_menustack[counter]);
               ra2exitinfo->menustackpointer=od_control.user_menustackpointer;
               memcpy(&ra2exitinfo->error_free,&od_control.user_error_free,3);
               c2pasc(ra2exitinfo->emsi_crtdef,40,od_control.user_emsi_crtdef);
               c2pasc(ra2exitinfo->emsi_protocols,40,od_control.user_emsi_protocols);
               c2pasc(ra2exitinfo->emsi_capabilities,40,od_control.user_emsi_capabilities);
               c2pasc(ra2exitinfo->emsi_requests,40,od_control.user_emsi_requests);
               c2pasc(ra2exitinfo->emsi_software,40,od_control.user_emsi_software);
               memcpy(&ra2exitinfo->hold_attr1,&od_control.user_hold_attr1,3);
               c2pasc(ra2exitinfo->page_reason,77,od_control.user_reasonforchat);
               if(_ra_status)
                  {
                  ra2exitinfo->status_line=status_line+1;
                  }

               c2pasc(ra2exitinfo->last_cost_menu,9,od_control.user_last_cost_menu);
               ra2exitinfo->menu_cost_per_min=od_control.user_menu_cost;
               ra2exitinfo->has_rip=od_control.user_rip;

               fwrite(ra2exitinfo,1,sizeof(struct _ra2exitinfo),fp);
               free(ra2exitinfo);
               break;

            case EXITINFO:
               c2pasc(exitinfo->bbs.ra.timeofcreation,5,od_control.caller_timeofcreation);
               c2pasc(exitinfo->bbs.ra.logonpassword,15,od_control.caller_logonpassword);
               exitinfo->bbs.ra.wantchat=od_control.caller_wantchat;

               write_exitinfo_primitive(fp,476);
               break;


            case RA1EXITINFO:
               ext_exitinfo->deducted_time=od_control.ra_deducted_time;

               for(counter=0;counter<50;++counter)
                  {
                  c2pasc(ext_exitinfo->menustack[counter],8,od_control.ra_menustack[counter]);
                  }

               ext_exitinfo->menustackpointer=od_control.ra_menustackpointer;
               c2pasc(ext_exitinfo->userhandle,35,od_control.ra_userhandle);
               c2pasc(ext_exitinfo->comment,80,od_control.ra_comment);
               c2pasc(ext_exitinfo->firstcall,8,od_control.ra_firstcall);
               memcpy(ext_exitinfo->combinedrecord,od_control.ra_combinedrecord,25);
               c2pasc(ext_exitinfo->birthday,8,od_control.ra_birthday);
               c2pasc(ext_exitinfo->subdate,8,od_control.ra_subdate);
               ext_exitinfo->screenwidth=od_control.user_screenwidth;
               ext_exitinfo->msgarea=od_control.user_msg_area;
               ext_exitinfo->filearea=od_control.user_file_area;
               ext_exitinfo->language=od_control.user_language;
               ext_exitinfo->dateformat=od_control.user_date_format;
               c2pasc(ext_exitinfo->forwardto,35,od_control.ra_forward_to);
               memcpy(&ext_exitinfo->error_free,&od_control.ra_error_free,3);
               c2pasc(ext_exitinfo->emsi_crtdef,40,od_control.ra_emsi_crtdef);
               c2pasc(ext_exitinfo->emsi_protocols,40,od_control.ra_emsi_protocols);
               c2pasc(ext_exitinfo->emsi_capabilities,40,od_control.ra_emsi_capabilities);
               c2pasc(ext_exitinfo->emsi_requests,40,od_control.ra_emsi_requests);
               c2pasc(ext_exitinfo->emsi_software,40,od_control.ra_emsi_software);
               memcpy(&ext_exitinfo->hold_attr1,&od_control.ra_hold_attr1,3);

               c2pasc(exitinfo->bbs.ra.timeofcreation,5,od_control.caller_timeofcreation);
               c2pasc(exitinfo->bbs.ra.logonpassword,15,od_control.caller_logonpassword);
               exitinfo->bbs.ra.wantchat=od_control.caller_wantchat;

               write_exitinfo_primitive(fp,476);
               fwrite(ext_exitinfo,1,1017,fp);
               free(ext_exitinfo);
               break;


            case QBBS275EXITINFO:
               exitinfo->elapsed=_initial_elapsed;
               exitinfo->bbs.qbbs.qwantchat=od_control.caller_wantchat;
               exitinfo->bbs.qbbs.gosublevel=od_control.ra_menustackpointer;
               for(counter=0;counter<exitinfo->bbs.qbbs.gosublevel;++counter)
                  {
                  c2pasc(exitinfo->bbs.qbbs.menustack[counter],8,od_control.ra_menustack[counter]);
                  }
               c2pasc(exitinfo->bbs.qbbs.menu,8,od_control.ra_menustack[od_control.ra_menustackpointer]);

               write_exitinfo_primitive(fp,644);
            }

         fclose(fp);
         }
      }


   switch(od_control.od_info_type)
      {
      case DOORSYS_GAP:
      case DOORSYS_WILDCAT:
         fp=fopen(ipath,"w");
         if(od_control.baud==0L)
            {
            fprintf(fp,"COM0:\n");
            }
         else
            {
            fprintf(fp,"COM%d:\n",od_control.port+1);
            }
         fprintf(fp,"%s",storestr[0]);
         fprintf(fp,"%s",storestr[1]);
         fprintf(fp,"%u\n",od_control.od_node);
         switch(doorsys_lock)
            {          
            case 0:
               fprintf(fp,"%lu\n",od_control.baud);
               break;
            case 1:
               fprintf(fp,"N\n");
               break;
            case 2:
               fprintf(fp,"Y\n");
            }
         fprintf(fp,"%s",storestr[3]);
         fprintf(fp,"%s",storestr[4]);
         fprintf(fp,"%s",storestr[5]);
         fprintf(fp,"%s",storestr[22]);
         strupr(od_control.user_name);
         fprintf(fp,"%s\n",od_control.user_name);
         fprintf(fp,"%s\n",od_control.user_location);
         fprintf(fp,"%s\n",od_control.user_homephone);
         fprintf(fp,"%s\n",od_control.user_dataphone);
         fprintf(fp,"%s\n",od_control.user_password);
         fprintf(fp,"%u\n",od_control.user_security);
         fprintf(fp,"%d\n",od_control.user_numcalls);
         fprintf(fp,"%s\n",od_control.user_lastdate);
         fprintf(fp,"%u\n",(signed int)od_control.caller_timelimit*60);
         fprintf(fp,"%d\n",od_control.caller_timelimit);
         if(od_control.user_rip)
            {
            fprintf(fp,"RIP\n");
            }
         else if(od_control.user_ansi)
            {
            fprintf(fp,"GR\n");
            }
         else
            {
            fprintf(fp,"NG\n");
            }
         fprintf(fp,"%d\n",od_control.user_screen_length);
         fprintf(fp,"%s",storestr[8]);
         fprintf(fp,"%s",storestr[9]);
         fprintf(fp,"%s",storestr[10]);
         fprintf(fp,"%s\n",od_control.ra_subdate);
         fprintf(fp,"%u\n",od_control.user_num);
         fprintf(fp,"%s",storestr[6]);
         fprintf(fp,"%u\n",od_control.user_uploads);
         fprintf(fp,"%u\n",od_control.user_downloads);
         fprintf(fp,"%u\n",od_control.user_todayk);
         fprintf(fp,"%s",storestr[21]);


         if(od_control.od_info_type==DOORSYS_WILDCAT)
            {
            fprintf(fp,"%s\n",od_control.user_birthday);
            fprintf(fp,"%s",storestr[11]);
            fprintf(fp,"%s",storestr[12]);
            fprintf(fp,"%s\n",od_control.sysop_name);
            strupr(od_control.user_handle);
            fprintf(fp,"%s\n",od_control.user_handle);
            fprintf(fp,"%s\n",od_control.event_starttime);
            if(od_control.user_error_free)
               fprintf(fp,"Y\n");
            else
               fprintf(fp,"N\n");
            fprintf(fp,"%s",storestr[7]);
            fprintf(fp,"%s",storestr[13]);
            fprintf(fp,"%s",storestr[14]);
            fprintf(fp,"%s",storestr[15]);
            fprintf(fp,"%s",storestr[16]);
            fprintf(fp,"%s\n",od_control.user_logintime);
            fprintf(fp,"%s\n",od_control.user_lasttime);
            fprintf(fp,"%s",storestr[18]);
            fprintf(fp,"%s",storestr[19]);
            fprintf(fp,"%u\n",od_control.user_upk);
            fprintf(fp,"%u\n",od_control.user_downk);
            fprintf(fp,"%s\n",od_control.user_comment);
            fprintf(fp,"%s",storestr[20]);
            fprintf(fp,"%u\n",od_control.user_messages);
            }

         fclose(fp);
         break;


      case DOORSYS_DRWY:
         fp=fopen(ipath,"w");
         fprintf(fp,"%s\n",od_control.user_name);

         if(od_control.baud==0L)
            {
            fprintf(fp,"-1\n");
            }
         else
            {
            fprintf(fp,"%d\n",od_control.port+1);
            }

         fprintf(fp,"%lu\n",od_control.baud);

         fprintf(fp,"%d\n",od_control.caller_timelimit);

         if(od_control.caller_ansi)
            {
            fprintf(fp,"G\n");
            }
         else
            {
            fprintf(fp,"M\n");
            }

         fclose(fp);
         break;


      case SFDOORSDAT:
         fp=fopen(ipath,"w");

         fprintf(fp,"%u\n",od_control.user_num);
         fprintf(fp,"%s\n",od_control.user_name);
         fprintf(fp,"%s\n",od_control.user_password);
         fprintf(fp,"%s",storestr[0]);
         fprintf(fp,"%lu\n",od_control.baud);
         fprintf(fp,"%d\n",od_control.port+1);
         fprintf(fp,"%d\n",od_control.caller_timelimit);
         fprintf(fp,"%s",storestr[13]);
         fprintf(fp,"%s",storestr[14]);
         if(od_control.caller_ansi)
            {
            fprintf(fp,"TRUE\n");
            }
         else
            {
            fprintf(fp,"FALSE\n");
            }
         fprintf(fp,"%u\n",od_control.user_security);
         fprintf(fp,"%u\n",od_control.user_uploads);
         fprintf(fp,"%u\n",od_control.user_downloads);
         fprintf(fp,"%s",storestr[1]);
         fprintf(fp,"%s",storestr[2]);
         fprintf(fp,"%s",storestr[3]);
         if(od_control.ra_sysop_next)
            {
            fprintf(fp,"TRUE\n");
            }
         else
            {
            fprintf(fp,"FALSE\n");
            }
         fprintf(fp,"%s",storestr[4]);
         fprintf(fp,"%s",storestr[5]);
         fprintf(fp,"%s",storestr[6]);
         if(od_control.ra_error_free)
            {
            fprintf(fp,"TRUE\n");
            }
         else
            {
            fprintf(fp,"FALSE\n");
            }

         fprintf(fp,"%u\n",od_control.user_msg_area);
         fprintf(fp,"%u\n",od_control.user_file_area);
         fprintf(fp,"%u\n",od_control.od_node);

         fprintf(fp,"%s",storestr[10]);
         fprintf(fp,"%s",storestr[11]);
         fprintf(fp,"%s",storestr[12]);
         fprintf(fp,"%u\n",od_control.user_todayk);
         fprintf(fp,"%u\n",od_control.user_upk);
         fprintf(fp,"%u\n",od_control.user_downk);
         fprintf(fp,"%s\n",od_control.user_homephone);
         fprintf(fp,"%s\n",od_control.user_location);
         if(storestr[15][0]!='\0') fprintf(fp,"%s",storestr[15]);
         fclose(fp);
         break;


        case CHAINTXT:
           fp=fopen(ipath,"w");
           fprintf(fp,"%d\n",od_control.user_num);
           fprintf(fp,"%s\n",od_control.ra_userhandle);
           fprintf(fp,"%s\n",od_control.user_name);
           fprintf(fp,"%s\n",od_control.caller_callsign);
           fprintf(fp,"%s",storestr[0]);
           fprintf(fp,"%c\n",od_control.caller_sex);
           fprintf(fp,"%s",storestr[1]);
           fprintf(fp,"%s\n",od_control.user_lastdate);
           fprintf(fp,"%d\n",od_control.ra_screenwidth);
           fprintf(fp,"%d\n",od_control.user_screen_length);
           fprintf(fp,"%d\n",od_control.user_security);
           fprintf(fp,"%d\n",is_sysop);
           fprintf(fp,"%d\n",is_cosys);
           fprintf(fp,"%d\n",od_control.caller_ansi);
           if(od_control.baud==0L)
              {
              fprintf(fp,"0\n");
              }
           else
              {
              fprintf(fp,"1\n");
              }
           fprintf(fp,"    %d.00\n",od_control.caller_timelimit*60);
           fprintf(fp,"%s",storestr[3]);
           fprintf(fp,"%s",storestr[4]);
           fprintf(fp,"%s",storestr[5]);
           if(od_control.baud==0L)
              {
              fprintf(fp,"KB\n");
              }
           else
              {
              fprintf(fp,"%lu\n",od_control.baud);
              }
           fprintf(fp,"%d\n",od_control.port+1);
           fprintf(fp,"%s",storestr[6]);
           fprintf(fp,"%s\n",od_control.user_password);
           fprintf(fp,"%s",storestr[2]);
           fprintf(fp,"%s",storestr[7]);
           fprintf(fp,"%s",storestr[8]);
           fprintf(fp,"%s",storestr[9]);
           fprintf(fp,"%s",storestr[10]);
           fprintf(fp,"%s",storestr[11]);
           fprintf(fp,"%s",storestr[12]);
           fclose(fp);
	  }


   for(counter=0;counter<25;++counter)
      {
      free(storestr[counter]);
      }


   if(_log_close!=NULL)                /* If logfile system is active */
      {                                /* Then close the logfile */
      (*_log_close)(errorlevel);
      }

		 
   if(od_control.baud && term_call)
      {
      _waitdrain(192);         /* Wait up to ten seconds for bufffer to drain */

                                    /* Wait up to five seconds for no carrier */
      _com_dtr(FALSE);
      max_time=time(NULL)+5L;

      while(_com_carrier() && time(NULL) <= max_time);
		 
      _com_dtr(TRUE);                                   /* Raise DTR again */
      }

   if(under)
      {
      phys_delwin(x1, 10, x1+(winwidth - 1), 14, under);
      }

   phys_window(1,1,80,25);              /* reset text window */
   phys_setattrib(0x07);                /* reset text color */
   if(od_control.od_clear_on_exit)
      {
      phys_clrscr();                    /* clear screen if neccesary */
      }
   else
      {
      phys_gotoxy(1,1);
      }

   _com_close();                                         /* close serial port */

   inited = FALSE;                           /* OpenDoors is no longer active */

   if(od_control.od_noexit) return;

   if(_preorexit) return;

   exit(errorlevel);                      /* exit with appropriate errorlevel */
   }


                                       /* Intelligent door information file */
                                       /* locating routine. Searches for */
                                       /* the info file first in the path */
                                       /* specified by the info_path variable */
                                       /* (as set by the configuration file). */
                                       /* If not found, check the current */
                                       /* directory */
                                       /* If not found, check directory */
                                       /* specified by a BBS path environment */
                                       /* variable */
#define NUM_ENV_VARS 4
char search_for_infofile(char **filenames, int num_filenames, char *found, char *dir)
   {
   register char counter;
   char *setting;
   static char *env_vars[NUM_ENV_VARS]={"RA","QUICK","PCB","BBS"};
   char result;

   if(strlen(od_control.info_path)!=0)
      if((result=search_in_dir(filenames,num_filenames,found,od_control.info_path))!=-1)
         {
         if(dir!=NULL) strcpy(dir,od_control.info_path);
         return(result);
         }

   if((result=search_in_dir(filenames,num_filenames,found,".\\"))!=-1)
      {
      if(dir!=NULL) strcpy(dir,".\\");
      return(result);
      }

   for(counter=0;counter<NUM_ENV_VARS;++counter)
      if((setting=(char *)getenv(env_vars[counter]))!=NULL)
         if((result=search_in_dir(filenames,num_filenames,found,setting))!=-1)
            {
            if(dir!=NULL) strcpy(dir,setting);
            return(result);
            }

   return(-1);
   }


long int _fsize(FILE *stream)
   {
   long int original;
   long int to_return;

   original = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   to_return = ftell(stream);
   fseek(stream, original, SEEK_SET);
   return(to_return);
   }


                                       /* Helper function for search_for_info.. */
                                       /* Looks for door info files in */
                                       /* specified directory */
char search_in_dir(char **filenames, int num_filenames, char *found, char *dir)
   {
   register char counter;
   register char *fullname;
   register char found_yet=-1;
   unsigned int latest_date=0;
   unsigned int latest_time=0;

   for(counter=0;counter<num_filenames;++counter)
       {
       /* Do not consider DORINFO1.DEF if a DORINFOx.DEF for this node has */
       /* been found. */
       if(counter == 1 && found_yet != -1)
          {
          continue;
          }

       fullname=makepath(dir,(char *)filenames[counter]);
       if(_odfindfirst(fullname,&_fblk,0x20)==0)
          {
          if(found_yet==-1 || latest_date<_fblk.ff_fdate || (latest_date==_fblk.ff_fdate && latest_time<_fblk.ff_ftime))
             {
             if(_accessmode(fullname,4)==0)
                {
                found_yet=counter;
                latest_time=_fblk.ff_ftime;
                latest_date=_fblk.ff_fdate;
                }
             }
          }
       }
   if(found_yet!=-1)
      {
      strcpy(found,makepath(dir,(char *)filenames[found_yet]));
      }

   return(found_yet);
   }


int read_exitinfo_primitive(FILE *fp,int count)
   {
   if((exitinfo=malloc(sizeof(struct _exitinfo)))==NULL) return(FALSE);

   if(fread(exitinfo,1,count,fp)!=count) return(FALSE);


                                          /* now we read all the data from the */
                                          /* EXITINFO structure to the OpenDoors */
                                          /* control structure. This may look */
                                          /* a bit messy, but it gets the job */
                                          /* done, and allows the programmer */
                                          /* to access all the strings in C */
                                          /* format instead of Pascal */
   od_control.baud=exitinfo->baud;
   od_control.system_calls=exitinfo->num_calls;
   pasc2c(od_control.system_last_caller,exitinfo->last_caller,35);
   pasc2c(od_control.timelog_start_date,exitinfo->start_date,8);
   memcpy(&od_control.timelog_busyperhour,&exitinfo->busyperhour,62);
   pasc2c(od_control.user_name,exitinfo->uname,35);
   pasc2c(od_control.user_location,exitinfo->uloc,25);
   pasc2c(od_control.user_password,exitinfo->password,15);
   pasc2c(od_control.user_dataphone,exitinfo->dataphone,12);
   pasc2c(od_control.user_homephone,exitinfo->homephone,12);
   pasc2c(od_control.user_lasttime,exitinfo->lasttime,5);
   pasc2c(od_control.user_lastdate,exitinfo->lastdate,8);
   memcpy(&od_control.user_attribute,&exitinfo->attrib,5);
   od_control.user_net_credit=exitinfo->credit;
   od_control.user_pending=exitinfo->pending;
   od_control.user_messages=exitinfo->posted;
   od_control.user_lastread=exitinfo->lastread;
   od_control.user_security=exitinfo->sec;
   od_control.user_numcalls=exitinfo->nocalls;
   od_control.user_uploads=exitinfo->ups;
   od_control.user_downloads=exitinfo->downs;
   od_control.user_upk=exitinfo->upk;
   od_control.user_downk=exitinfo->downk;
   od_control.user_todayk=exitinfo->todayk;
   memcpy(&od_control.user_time_used,&exitinfo->elapsed,6);
   od_control.user_group=exitinfo->group;
   od_control.user_xi_record=exitinfo->xirecord;
   od_control.event_status=exitinfo->status;
   pasc2c(od_control.event_starttime,exitinfo->starttime,5);
   memcpy(&od_control.event_errorlevel,&exitinfo->errorlevel,3);
   pasc2c(od_control.event_last_run,exitinfo->lasttimerun,8);
   memcpy(&od_control.caller_netmailentered,&exitinfo->netmailentered,2);
   pasc2c(od_control.caller_logintime,exitinfo->logintime,5);
   pasc2c(od_control.caller_logindate,exitinfo->logindate,8);

   /* Note that the timelimit field is skipped here. This value has already */
   /* been read from the DORINFOx.DEF file, and is not consistently written */
   /* to the EXITINFO.BBS file by various BBS packages.                     */

   memcpy(&od_control.user_loginsec,&exitinfo->loginsec,16);
   od_control.caller_ansi=od_control.user_attribute&8;
   od_control.od_avatar=od_control.user_attrib2&2;

   return(TRUE);
   }



int write_exitinfo_primitive(FILE *fp,int count)
   {
   int to_return;

   exitinfo->num_calls=od_control.system_calls;
   c2pasc(exitinfo->last_caller,35,od_control.system_last_caller);
   c2pasc(exitinfo->start_date,8,od_control.timelog_start_date);
   memcpy(&exitinfo->busyperhour,&od_control.timelog_busyperhour,31);
   c2pasc(exitinfo->uname,35,od_control.user_name);
   c2pasc(exitinfo->uloc,25,od_control.user_location);
   c2pasc(exitinfo->password,15,od_control.user_password);
   c2pasc(exitinfo->dataphone,12,od_control.user_dataphone);
   c2pasc(exitinfo->homephone,12,od_control.user_homephone);
   c2pasc(exitinfo->lasttime,5,od_control.user_lasttime);
   c2pasc(exitinfo->lastdate,8,od_control.user_lastdate);
   memcpy(&exitinfo->attrib,&od_control.user_attribute,5);
   exitinfo->credit=(unsigned int)od_control.user_net_credit;
   exitinfo->pending=(unsigned int)od_control.user_pending;
   exitinfo->posted=(unsigned int)od_control.user_messages;
   exitinfo->lastread=(unsigned int)od_control.user_lastread;
   exitinfo->sec=(unsigned int)od_control.user_security;
   exitinfo->nocalls=(unsigned int)od_control.user_numcalls;
   exitinfo->ups=(unsigned int)od_control.user_uploads;
   exitinfo->downs=(unsigned int)od_control.user_downloads;
   exitinfo->upk=(unsigned int)od_control.user_upk;
   exitinfo->downk=(unsigned int)od_control.user_downk;
   exitinfo->todayk=(unsigned int)od_control.user_todayk;
   memcpy(&exitinfo->elapsed,&od_control.user_time_used,6);
   exitinfo->group=od_control.user_group;
   exitinfo->xirecord=(unsigned int)od_control.user_xi_record;
   exitinfo->status=od_control.event_status;
   exitinfo->status=od_control.event_status;
   c2pasc(exitinfo->starttime,5,od_control.event_starttime);
   memcpy(&exitinfo->errorlevel,&od_control.event_errorlevel,3);
   c2pasc(exitinfo->lasttimerun,8,od_control.event_last_run);
   memcpy(&exitinfo->netmailentered,&od_control.caller_netmailentered,2);
   c2pasc(exitinfo->logintime,5,od_control.caller_logintime);
   c2pasc(exitinfo->logindate,8,od_control.caller_logindate);
   memcpy(&exitinfo->timelimit,&od_control.caller_timelimit,18);

   to_return=(fwrite(exitinfo,1,count,fp)==count);
   free(exitinfo);
   return(to_return);
   }


void _preexit(void)
   {
   if(inited)
      {
      _preorexit = TRUE;
      if(od_control.od_errorlevel[0])
         od_exit(od_control.od_errorlevel[7],FALSE);
      else
         od_exit(6,FALSE);
      }
   }


/* _ulongdiv() - Divides one unsigned long by another unsigned long.        */
/*               pluQuotient and/or pluRemainder can be NULL if the caller  */
/*               does not care about their values. Returns TRUE on success, */
/*               FALSE on failure. Only fails if luDivisor is 0.            */
BOOL _ulongdiv(unsigned long * pluQuotient, unsigned long * pluRemainder,
               unsigned long luDividend, unsigned long luDivisor)
   {
   int nTimes = 0;
   unsigned long luQuotient;
   unsigned long luRemainder;

   /* Check that divisor is not zero. (An attempt to divide by zero will */
   /* put this algorithm into an infinite loop, rather than triggering   */
   /* a divide fault.)                                                   */
   if(luDivisor == 0L)
      {
      return(FALSE);
      }

   /* Initialize remainder to be entire dividend */
   luRemainder = luDividend;

   /* Initialize quotient to 0 */
   luQuotient = 0L;

   /* Determine largest required multiple of divisor */
   while(luRemainder >=  luDivisor)
      {
      luDivisor = long_shift_left(luDivisor, 1);
      ++nTimes;
      }

   /* Loop across for all multiples of divisor, beginning with the largest */
   do
      {
      luQuotient = long_shift_left(luQuotient, 1);

      /* If current remainder is >= this multiple of the divisor */
      if(luRemainder >= luDivisor)
         {
         /* Subtract the multiple of the divisor from the remainder */
         luRemainder -= luDivisor;

         /* The next bit of the quotient should be a 1 */
         luQuotient |= 1L;
         }

      /* Divide current multiple of divisor by two */
      luDivisor = long_shift_right(luDivisor, 1);

      /* Repeat for all multiples of the divisor */
      } while(nTimes--);

   /* If caller asked for quotient, then return it */
   if(pluQuotient != NULL)
      {
      *pluQuotient = luQuotient;
      }

   /* If caller asked for remainder, then return it */
   if(pluRemainder != NULL)
      {
      *pluRemainder = luRemainder;
      }

   return(TRUE);
   }


char _strmatchtail(char *string, char *tail)
   {
   int taillen = strlen(tail);
   int stringlen = strlen(string);

   if(stringlen < taillen)
      {
      return(FALSE);
      }

   return(stricmp(string + (stringlen - taillen), tail) == 0);
   }
