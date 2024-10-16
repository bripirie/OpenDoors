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
 *     Filename : ODCORE.C
 *  Description : Contains the code of the OpenDoors core functions.
 *      Version : 5.00
 */




#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"






#include<stdio.h>                      /* Standard header files */
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<dir.h>
#include<alloc.h>
#include<time.h>
#include<errno.h>

#ifndef USEINLINE
#include<dos.h>
#endif

struct _od_control od_control;         /* The OpenDoors control structure */

char block[2]={' ',0x70};              /* Used to place character in pos 80,25 */
char blankblock[2]={' ',0x07};         /* Used to place blank in pos 80,25 */
char clearit[2]={12,0};                /* Clearscreen sequence */
char backstr[4]={8,' ',8,0};           /* Backspace sequence */
char ansi_clear[3]={27,'[','K'};       /* ANSI clear-line sequence */
char avatar_clear[2]={22,7};           /* AVATAR clear-line sequence */

struct find_block _fblk;               /* findfirst/findnext structure */

char _yet;                             /* has first sequence been done yet */

                                       /* IBM -> ANSI color conversion table */
char ibm2ansii[8]={30,34,32,36,31,35,33,37};

struct phys_text_info user_text;       /* used to store cursor, etc. settings */


union REGS regs;                       /* 8086 registers for interrupt calls */

time_t next_minute;                    /* time of next time limit update */
time_t next_statup;                    /* time of next status line update */
time_t last_activity;                  /* time of last user activity */
unsigned char *input_buffer;           /* inbound keystroke buffer */
char *input_remote;                    /* source of key (FALSE=local) */
unsigned int _in_buf_size;             /* # of bytes allocated for above */
int b_head=0;                          /* location of first char in buffer */
int b_tail=0;                          /* location of last char in buffer */
int last_inactivity_setting=0;
char colour_check_char=0;
char *colour_end_pos;
long _last_kernel;
char _doing_cs_hook;
char _sfunc_pending;
char _is_callback = FALSE;             /* Is a callback active */
char _force_update=FALSE;              /* Force status line update */


                                       /* LOGFILE HOOKS */
                                       /* ƒƒƒƒƒƒƒƒƒƒƒƒƒ */
int          (*_log_wrt)(int)=NULL;    /* function that writes to logfile */
void         (*_log_close)(int)=NULL;  /* function that closes the logfile */

                                       /* OPENDOORS STATUS VARIABLES */
                                       /* ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ */
char inited=FALSE;                     /* had opendoors been initialized */
char kernal_active=FALSE;              /* is the opendoors kernal active */
char warned=FALSE;                     /* user has been warned of inactivity */
char chatted;                          /* has the sysop chatted with user */
char last_status;                      /* last setting of status bar (on/off) */
char sysop_color,sysop_key;
char _multitasker=MULTITASKER_NONE;
char status_line=8;                   /* Status line number which is active */
char _last_control_key=0;
char globworkstr[257];

void (*current_status_function)(unsigned char which);
char _output_top=1;
char _output_bottom=23;
char _desired_personality[33]="";
int (*_set_personality)(char *name)=NULL;
char ra_status_to_set=0;




                                      /* Function to giveup current timeslice */
void giveup_slice(void)
   {
#ifdef USEINLINE
   switch(_multitasker)
      {
      case MULTITASKER_DV:
         ASM    mov ax, 0x1000
         ASM    int 0x15
         break;

      case MULTITASKER_WIN:
         ASM    mov ax, 0x1680
         ASM    int 0x2f
         break;

      case MULTITASKER_OS2:
      default:
         ASM    int 0x28
      }
#else
   switch(_multitasker)
      {
      case MULTITASKER_DV:
         regs.x.ax = 0x1000;
         int86(0x15,&regs,&regs);
         break;

      case MULTITASKER_WIN:
         regs.x.ax = 0x1680;
         int86(0x2f,&regs,&regs);
         break;

      case MULTITASKER_OS2:
         break;

      default:
         int86(0x28,&regs,&regs);
      }
#endif
   }


                                       /* Function to return the first word in */
                                       /* a string containing a series of */
                                       /* words seperated by one or more spaces */
char *first_word(char *string,char *outstr)
   {
   register char *dest=(char *)outstr;

   while(*string && *string!=' ') *dest++=*string++;
   *dest='\0';

   return(outstr);
   }


                                      /* Function to return the remaining words */
                                      /* (after the first word) of a string */
                                      /* of words seperated by spaces. (eg, */
                                      /* "I think, Therefore I am" --> */
                                      /*          "think, Therefore I am" */
char *other_words(char *string)
   {                                  /* Pointer to first space in string */
   register char *ret_dest=(char *)string;

   while(*ret_dest && *ret_dest!=' ')
      {
      ++ret_dest;
      }

   while(*ret_dest && *ret_dest==' ') /* Otherwise, search through string */
      {                               /* for first character after space */
      ++ret_dest;
      }

   return((char *)ret_dest);           /* Otherwise, return pointer to the */
    }                                   /* rest of the string */


void _waitdrain(int length)
   {
   long start;

   if(od_control.baud == 0) return;

   start = _clock_tick();

   while(_com_outbound() && start+length > _clock_tick() &&
      _clock_tick() >=start )
         ;
   }


void od_clr_scr(void)                  /* clearscreen function */
   {
   int original_attrib;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_clr_scr()");

   if(!inited) od_init();

                                       /* don't clear screen if disabled */
   if(!od_control.od_always_clear && !(od_control.user_attribute&2) && (od_control.od_extended_info || od_control.od_info_type==CUSTOM))
      {
      return;
      }

   if(od_control.user_rip)
      {
      od_disp("!|*", 3, FALSE);
      if(!od_control.od_default_rip_win)
         {
         od_disp("!|w0000270M12", 13, FALSE);
         }
      }
      
   od_disp(clearit,1,FALSE);           /* send ascii 12 to modem, no local echo */
      
   phys_clrscr();                      /* clear local window */

                                       /* get colour set prior to screen clear */
   original_attrib=od_control.od_cur_attrib;
   od_control.od_cur_attrib=-1;        /* current colour state is unknown */
   od_set_attrib(original_attrib);     /* set colour to original value */
   }



                                       /* get a string from user */
void od_input_str(char *string,int max_len,unsigned char minchar,unsigned char maxchar)
   {
   register char inkey;                /* key pressed by user */
   register int position;              /* position within string */
   char temp_str[2];                   /* character to be echoed */

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_input_str()");

   if(!inited) od_init();              /* verify that we've been initialized */

   position=0;                         /* start at beginning of string */

   if(!string)
      {
      od_control.od_error = ERR_PARAMETER;
      return;
      }

   for(;;)
      {
      inkey=od_get_key(TRUE);          /* get a key from buffer */

      if(inkey==13)                    /* if carriage return */
         {
         string[position]='\0';        /* terminate string */
         od_disp_str("\n\r");          /* send CR-LF sequence */
         return;                       /* exit */
         }

      else if(inkey==8 && position>0)  /* if backspace */
         {
         od_disp_str(backstr);         /* send backspace sequence */
         --position;                   /* move pointer one back in string */
         }
                                       /* if valid keypress */
      else if(inkey>=minchar && inkey<=maxchar && position<max_len)
         {
         temp_str[1]='\0';
         temp_str[0]=inkey;
         od_disp_str(temp_str);        /* display keypress */
         string[position++]=inkey;     /* store keypress in string */
         }
      }
   }


void od_clear_keybuffer(void)          /* function to clear inbound key buffer */
   {
   if(!inited) od_init();              /* initialize opendoors if no done */

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_clear_keybuffer()");

   b_head=0;                           /* reset buffer pointers */
   b_tail=0;

   if(od_control.baud!=0)              /* if not in local mode */
      {
      _com_clear_inbound();            /* clear the fossil buffer as well */
      }

   od_kernal();                        /* call the kernal */
   }


int od_get_key(int wait)               /* get a key from the inbound buffer */
   {
   if(!inited) od_init();              /* initialize if not done already */

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_get_key()");

   for(;;)                             /* Loop until valid char to return */
      {
      od_kernal();                     /* Call the od_kernal() function */

      if(b_head!=b_tail)               /* If there are characters in buffer */
         {
         return(get_char());           /* Return next character from buffer */
         }
                                       /* If there are no chars in buffer */
      if(!wait)                        /* If not running in "wait" mode */
         {
         return(0);                    /* Return a zero */
         }
                                       /* If we are running in "wait" mode */
      giveup_slice();                  /* Giveup the processor to other tasks */
      }                                /* Loop to check for character again */
   }


int get_char(void)                     /* get character from inbound buffer */
  {
  register int get_from;               /* location to get character */

  get_from=b_tail;

  if(b_head==b_tail) return(0);        /* if no characters in buffer, exit */
  if(++b_tail>=_in_buf_size) b_tail=0;

                                       /* store input source of retrieved char */
  od_control.od_last_input=input_remote[get_from];

  return(input_buffer[get_from]);      /* return key waiting in buffer */
  }



int od_carrier(void)
   {
   if(!inited) od_init();              /* verify that we've been initialized */

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_carrier()");

   if(!od_control.baud)
      {
      od_control.od_error = ERR_NOREMOTE;
      return(FALSE);
      }
   return(_com_carrier());
   }



void od_kernel(void)                   /* The OpenDoors kernal function */
   {
   static char temp[80];               /* internal storage for kernal function */
   register int inkey;
   register char counter;
   char *command;
   time_t now;

   if(!inited) od_init();              /* verify that we've been initialized */

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_kernel()");

   if(kernal_active) return;           /* check kernal_active semaphore */

   kernal_active=TRUE;                 /* set kernal_active semaphore */

   if(od_control.od_ker_exec != NULL)  /* call od_ker_exec function if req. */
      {
      (*od_control.od_ker_exec)();
      }

   /* All potientially time comsuming tasks have now been executed, so we */
   /* get the current time for use throughout od_kernel().                */
   now = time(NULL);

                                       /* if not operating in local mode */
   if(od_control.baud && !(od_control.od_disable&DIS_CARRIERDETECT))
      {
      if(!_com_carrier())
         {
         _exitreason = 1;
         if(od_control.od_errorlevel[0])
            od_exit(od_control.od_errorlevel[2],FALSE);
         else
            od_exit(1,FALSE);
         }
      }

still_carrier:
                                       /* if inactivity setting has changed */
   if(last_inactivity_setting!=od_control.od_inactivity)
      {
      if(last_inactivity_setting==0)   /* if it used to be disabled */
         last_activity=now;            /* prevent immediate timeout */
                                       /* store current value */
      last_inactivity_setting=od_control.od_inactivity;
      }

                                       /* check user keyboard inactivity */
   if((last_activity+od_control.od_inactivity) < now)
      {                                /* if timeout, display message */
      if(od_control.od_inactivity!=0)
         {
         if(od_control.od_time_msg_func == NULL)
            {
            od_disp_str(od_control.od_inactivity_timeout);
            }
         else
            {
            (*od_control.od_time_msg_func)(od_control.od_inactivity_timeout);
            }
         _exitreason = 4;
         if(od_control.od_errorlevel[0])
            od_exit(od_control.od_errorlevel[5],TRUE);           /* log-off user */
         else
            od_exit(4,TRUE);
         }
      }

   /* if less than 5s left of inactivity */
   else if(last_activity + od_control.od_inactivity <
      now + od_control.od_inactive_warning)
      {
      if(!warned && od_control.od_inactivity!=0)
         {                             /* warn user */
         if(od_control.od_time_msg_func == NULL)
            {
            od_disp_str(od_control.od_inactivity_warning);
            }
         else
            {
            (*od_control.od_time_msg_func)(od_control.od_inactivity_warning);
            }
         warned=TRUE;                  /* don't warn a second time */
         }
      }
   else
      {
      warned=FALSE;                    /* re-enable inactivity warning */
      }

   /* if 1 minute has passed since last time update */
   if(now >= next_minute)
      {
      /* Next time update should occur 60 seconds after this one was */
      /* scheduled */
      next_minute+=60;

      /* Force status line to be updated immediately */
      _force_update = TRUE;

      /* Was used in version 3.99: */
      /* ++od_control.user_time_used; */
                                       /* decrement time left */
      if(--od_control.caller_timelimit<=3 && !(od_control.od_disable&DIS_TIMEOUT))
         {                             /* if less than 3 mins left, tell user */
         sprintf(temp,od_control.od_time_warning,od_control.caller_timelimit);
         if(od_control.od_time_msg_func == NULL)
            {
            od_disp_str(temp);
            }
         else
            {
            (*od_control.od_time_msg_func)(temp);
            }
         }
      }

check_keyboard_again:
    if(_sfunc_pending && !_doing_cs_hook)
       {
       if(_sfunc_pending & SFP_CHAT)
          {
          _sfunc_pending &=~ SFP_CHAT;
          goto chat_pressed;
          }
       }

#ifdef USEINLINE
   ASM    mov ah, 1
   ASM    push si
   ASM    push di
   ASM    int 0x16
   ASM    jnz key_waiting
   ASM    pop di
   ASM    pop si
   ASM    jmp after_key_check
key_waiting:
   ASM    mov ah, 0
   ASM    int 0x16
   ASM    pop di
   ASM    pop si
   ASM    mov inkey, ax
#else
   regs.h.ah = 1;
   int86(0x16, &regs, &regs);
   if(!regs.x.ax) goto after_key_check;
   regs.h.ah = 0;
   int86(0x16, &regs, &regs);
   inkey = regs.x.ax;
#endif
      /* record keyboard activity time */
      last_activity=now;

      if(inkey==od_control.key_hangup) /* if ALT-H (hangup on user) */
         {
         _exitreason=2;
         if(od_control.od_errorlevel[0])
            od_exit(od_control.od_errorlevel[3],TRUE);
         else
            od_exit(2,TRUE);
         }
                                       /* if ALT-D (Drop to BBS) */
      else if(inkey==od_control.key_drop2bbs)
         {
         _exitreason=5;
         if(od_control.od_errorlevel[0])
            od_exit(od_control.od_errorlevel[6],FALSE);
         else
            od_exit(5,FALSE);
         }

      else if(inkey==od_control.key_dosshell)
         {
shell_pressed:
         if(!_doing_cs_hook)
         {
         if(_log_wrt!=NULL)
            (*_log_wrt)(6);

         if(od_control.od_cbefore_shell!=NULL) /* if function hook is defined */
            {
            _doing_cs_hook=TRUE;
            (*od_control.od_cbefore_shell)();  /* call it! */
            _doing_cs_hook=FALSE;
            }

         if(od_control.od_before_shell!=NULL)
            od_disp_str(od_control.od_before_shell);

         if((command=(char *)getenv("COMSPEC"))==NULL) command=(char *)"COMMAND.COM";
         is_shell=TRUE;
         od_spawnvpe(P_WAIT,command,NULL,NULL);
         is_shell=FALSE;

         if(od_control.od_after_shell!=NULL)
            od_disp_str(od_control.od_after_shell);
         if(od_control.od_cafter_shell!=NULL) /* if function hook is defined */
            {
            _doing_cs_hook=TRUE;
            (*od_control.od_cafter_shell)();  /* call it! */
            _doing_cs_hook=FALSE;
            }

         if(_log_wrt!=NULL)
            (*_log_wrt)(7);
         }
         }

                                       /* if ALT-C (toggle chat mode) */
      else if(inkey==od_control.key_chat)
         {
chat_pressed:
         if(!_doing_cs_hook)
         {
         if(od_control.od_chat_active) /* if chat mode is active */
            {                          /* signal exit of chat mode */
            od_control.od_chat_active=FALSE;
            }

         else                          /* if chat mode is off */
            {
            kernal_active=FALSE;       /* enable second call to kernal */
            od_chat();                 /* enter sysop chat */
            kernal_active=TRUE;        /* disable second call to kernal */
            }
         }
         else
         {
         if(_sfunc_pending & SFP_CHAT)
            {
            _sfunc_pending &= ~SFP_CHAT;
            }
         else
            {
            _sfunc_pending |= SFP_CHAT;
            }
         }
         }

                                        /* ALT-N (sysop next) */
      else if(inkey==od_control.key_sysopnext)
         {                              /* toggle sysop next setting */
         od_control.ra_sysop_next=!od_control.ra_sysop_next;
         goto statup;                   /* update status line (that's right, */
         }                              /* more gotos!) */

                                        /* if ESCape key and in chat mode */
      else if((inkey&0xff)==27 && od_control.od_chat_active)
         {                              /* signal exit from chat mode */
         od_control.od_chat_active=FALSE;
         }

                                        /* if ALT-L (lockout user) */
      else if(inkey==od_control.key_lockout)
         {
         od_control.user_security=0;    /* set secutrity level to 0 */
         _exitreason=2;
         if(od_control.od_errorlevel[0])
            od_exit(od_control.od_errorlevel[3],TRUE);
         else
            od_exit(2,TRUE);
         }


                                        /* if ALT-K (toggle keyboard off) */
      else if(inkey==od_control.key_keyboardoff)
         {                              /* toggle user keyboard settings */
         od_control.od_user_keyboard_on=!od_control.od_user_keyboard_on;
         goto statup;                   /* update status line */
         }
                                        /* if up arrow (increase time) */
      else if(inkey==od_control.key_moretime)
         {
         if(od_control.caller_timelimit<1440)
            ++od_control.caller_timelimit; /* increase time left online */
         goto statup;                   /* update status line */
         }
                                        /* if down arrow (decrease time) */
      else if(inkey==od_control.key_lesstime)
         {
         if(od_control.caller_timelimit > 0)
            --od_control.caller_timelimit;/* decrease user's timelimit */
         goto statup;                  /* update the status line */
         }

      else
         {
         for(counter=0;counter<9;++counter)
            {
            if(inkey==od_control.key_status[counter])
               {
               if(status_line!=counter && od_control.od_status_on) od_set_statusline(counter);
               goto check_keyboard_again;
               }
            }
                                  /* look for user-defined hotkeys */
         for(counter=0;counter<od_control.od_num_keys;++counter)
            {                          /* if it matches */
            if(inkey==od_control.od_hot_key[counter])
               {                       /* record keypress */
               od_control.od_last_hot=inkey;

                                       /* Notify the current personality */
               (*current_status_function)(21);

                                       /* Check for a hotkey function */
               if(od_control.od_hot_function[counter] != NULL)
                  {                    /* Call it if it exists */
                  (*od_control.od_hot_function[counter])();
                  }
               break;                  /* stop searching */
               }
            }

         /* if no hotkeys found */
         if(counter>=od_control.od_num_keys)
            {
            /* Pass key on to od_local_input, if it is defined */
            if(od_control.od_local_input != NULL)
               {
               (*od_control.od_local_input)(inkey);
               }

            /* If local keyboard input by sysop has not been disabled */
            if(!(od_control.od_disable & DIS_LOCAL_INPUT))
               {
               if((inkey & 0xff) == 0)
                  {
                  add_to_buffer(0,1);
                  add_to_buffer(inkey>>8,1);
                  }
               else
                  {
                  add_to_buffer(inkey,1);

                  switch((char)inkey)
                     {
                     case 's':
                     case 'S':
                     case 3:
                     case 11:
                     case 0x18:
                        _last_control_key='s';
                        break;
                     case 'p':
                     case 'P':
                        _last_control_key='p';
                     }
                  }

               /* record that sysop was last to type */
               sysop_key=TRUE;
               }
            }
         }
   goto check_keyboard_again;

after_key_check:
                                       /* if status line has been turned on */
                                       /* since last call to kernal */
   if(last_status!=od_control.od_status_on)
      {
      od_set_statusline(0);            /* generate the status line */
      }

   last_status=od_control.od_status_on;/* record that status has been turned on */

   if(od_control.od_update_status_now)
      {
      od_set_statusline(status_line);
      od_control.od_update_status_now = FALSE;
      }
                                       /* update status line when needed */
   if(next_statup < now || _force_update)
      {
statup:
      next_statup=now+STATUS_FREQUENCY;

      /* Turn off status line update force flag */
      _force_update = FALSE;

      if(od_control.od_status_on && status_line!=8)
         {
         store_cursor();               /* store console settings */
         phys_window(1,1,80,25);       /* enable writes to whole screen */
         phys_cursor(FALSE);
         (*current_status_function)(10+status_line);
         restore_cursor();
         phys_cursor(TRUE);
         }
      }


   if(od_control.baud)                 /* if not in local mode */
      {
check_more:
      if(_com_inbound())
         {                                    /* store time of last activity */
         last_activity=now;

         counter = _com_getchar();               /* Get character from modem */

                                         /* if keyboard is on, add to buffer */
         if(od_control.od_user_keyboard_on)
            {
            add_to_buffer(counter,0);

            switch(counter)
               {
               case 's':
               case 'S':
               case 3:
               case 11:
               case 0x18:
                  _last_control_key='s';
                  break;
               case 'p':
               case 'P':
                  _last_control_key='p';
               }

            sysop_key=FALSE;              /* last keypress was by user */
            }
                                       /* check for another key */
         goto check_more;              /* (that's right, more gotos!) */
         }
      }

                                       /* if user has used up time */
   if(od_control.caller_timelimit<=0 && !(od_control.od_disable&DIS_TIMEOUT))
      {                                /* tell him/her */
      if(od_control.od_time_msg_func == NULL)
         {
         od_disp_str(od_control.od_no_time);
         }
      else
         {
         (*od_control.od_time_msg_func)(od_control.od_no_time);
         }
      _exitreason=3;
      if(od_control.od_errorlevel[0])
         od_exit(od_control.od_errorlevel[4],FALSE);             /* exit to BBS */
      else
         od_exit(3,FALSE);
      }

   _last_kernel=_clock_tick();

   kernal_active=FALSE;                /* kernal is no longer active */
   }


                                       /* function to add a character to buffer */
void add_to_buffer(char character, char remote)
   {
   int new_head;                       /* location of new head of buffer */

                                       /* if at end of buffer then wrap around */
   if((new_head=b_head+1)>=_in_buf_size) new_head=0;

   if(new_head!=b_tail)                /* if room in the buffer */
      {
      input_buffer[b_head]=character;  /* add character */
      input_remote[b_head]=remote;     /* store it's source */
      b_head=new_head;                 /* store new head of buffer */
      }
   }

                                       /* function to send a repeated */
                                       /* string to the termianl */
void od_repeat(char value,unsigned char times)
   {
   static char buffer[3];
   static char string[257];
   register char counter;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_repeat()");

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   if(times==0) return;
                                       /* generate string of repeat chars */
   for(counter=0;counter<times;counter++) string[counter]=value;
   string[counter]='\0';

   phys_cputs(string);                 /* display string to local console */

   if(od_control.od_avatar)            /* if in AVATAR mode */
      {
      buffer[0]=25;                    /* generate AVATAR repeat-sequence */
      buffer[1]=value;
      buffer[2]=times;
      od_disp(buffer,3,FALSE);         /* send 3 character AVATAR sequence */
      }
   else                                /* if AVATAR not available */
      {
      od_disp(string,times,FALSE);     /* send the character over and over */
      }
   }

void od_page(void)                     /* function to page sysop for chat */
   {
   int counter;                        /* loop counter */
   long until;
   time_t timer;
   struct tm *tblock;
   int minute;
   char failed=FALSE;
   char original_attrib;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_page()");

   if(!inited) od_init();              /* init opendoors if not done already */

   /* Save current display color attribute */
   original_attrib=od_control.od_cur_attrib;

   /* Clear the screen */
   od_clr_scr();
   od_set_attrib(od_control.od_chat_color1);


   timer=time(NULL);
   tblock=localtime(&timer);
   minute=(60*tblock->tm_hour)+tblock->tm_min;
   if(od_control.od_pagestartmin < od_control.od_pageendmin)
      {
      if(minute<od_control.od_pagestartmin || minute>=od_control.od_pageendmin) failed=TRUE;
      }
   else if(od_control.od_pagestartmin > od_control.od_pageendmin)
      {
      if(minute<od_control.od_pagestartmin && minute>=od_control.od_pageendmin) failed=TRUE;
      }
   else
      {
      failed = TRUE;
      }

   if(od_control.od_okaytopage==TRUE) failed=FALSE;

                                       /* if sysop paging not enabled */
   if(od_control.od_okaytopage==FALSE || failed)
     {                                 /* indicate this to user */
     od_disp_str(od_control.od_no_sysop);
     od_disp_str(od_control.od_press_key);
     od_get_answer("\x0d\x0a");
     goto cleanup;
     }
                                       /* Ask user reason for chat */
   od_disp_str(od_control.od_chat_reason);
   od_set_attrib(od_control.od_chat_color2);
   od_putch('[');
                                      /* display IBM chars if in ANSI mode */
   if(od_control.user_ansi || od_control.user_avatar)
      {
      od_repeat('ƒ',77);
      }
   else
      {
      od_repeat('-',77);
      }
   od_disp_str("]\n\r ");
   od_input_str(od_control.user_reasonforchat,77,32,255);

   if(strlen(od_control.user_reasonforchat)!=0)        /* If user did not abort */
      {                                /* Page the sysop */
      od_control.caller_wantchat=TRUE; /* Indicate that user wants to chat */

      /* update status line immediately */
      _force_update=TRUE;
      od_kernal();

      if(_log_wrt!=NULL)
         (*_log_wrt)(8);

      od_set_attrib(od_control.od_chat_color1);
      od_disp_str(od_control.od_paging);

      /* Display sysop page status line if it exists and the sysop status */
      /* line is currently active. */
      if(od_control.od_page_statusline!=-1 && status_line!=8)
         {
         od_set_statusline(od_control.od_page_statusline);
         }

      ++od_control.caller_numpages;    /* Increment the total sysop pages */
      chatted=FALSE;                   /* Sysop hasn't responded yet */

                                       /* loop for length of sysop page */
      for(counter=0;counter<od_control.od_page_len;++counter)
         {
         until=_clock_tick();
         od_putch('.');
         
         /* Abort page if system operator answered */
         if (chatted) goto cleanup;
         
         /* Send beep to local and remote systems */
         od_putch('\a');
         
         /* Check whether system operator has answered after playing beep. */
         if (chatted) goto cleanup;
                                       /* wait for 1 second after beep began */
         if(counter!=od_control.od_page_len)
            {
            while(_clock_tick() <= until + 18L && _clock_tick() >= until)
               {
               od_kernel();
               }
            }
         }

                                       /* if no response, tell user. */
      od_disp_str(od_control.od_no_response);
      od_disp_str(od_control.od_press_key);
      od_get_answer("\x0d\x0a");
      od_disp_str("\n\r\n\r");
      }

cleanup:
   /* Restore original display color attribute */
   od_set_attrib(original_attrib);
   }


void od_chat(void)                     /* sysop chat function */
   {
   register unsigned char inkey;       /* key pressed */
   char *temp_str=NULL;                /* temporary string used for scrn update */
   static char word[79];               /* the current word (used for wordwrap) */
   char wordlen=0;                     /* the length of the current word */
   char colpos=0;                      /* column the cursor is in */
   char *string;                       /* pointer to pos in temp string */
   char counter;                       /* loop counter for updating string */
   char original_attrib;
   long tick;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_chat()");

   if((temp_str=malloc(160))==NULL)
      {
      od_control.od_error = ERR_MEMORY;
      goto cleanup;
      }

   wordlen=0;                          /* reset variables */
   *word=0;
   colpos=0;
   od_control.od_chat_active=TRUE;

   if(!inited) od_init();

   /* Save current display color attribute */
   original_attrib=od_control.od_cur_attrib;

   /* Record that sysop has entered chat mode */
   chatted=TRUE;
   
   /* Turn off "user wants to chat" indictor */
   od_control.caller_wantchat=FALSE;
   _force_update=TRUE;
   od_kernal();

   /* If a pre-chat function hook has been defined, then call it */
   if(od_control.od_cbefore_chat!=NULL)
      {
      _doing_cs_hook=TRUE;
      (*od_control.od_cbefore_chat)(); /* call it! */
      _doing_cs_hook=FALSE;
      }

   /* If chat has been deactivated, return right away */
   if(!od_control.od_chat_active) goto cleanup;

                                       /* indicate that sysop is here for chat */
   od_set_attrib(od_control.od_chat_color1);
   if(od_control.od_before_chat!=NULL)
      od_disp_str(od_control.od_before_chat);
   sysop_color=TRUE;                   /* currently set to sysop color */

   if(_log_wrt!=NULL)
      (*_log_wrt)(9);

   while(od_control.od_chat_active)    /* until chat mode is turned off */
      {                                /* prevent user time from being drained */
      next_minute = time(NULL)+60;
      tick=_clock_tick();
      inkey=od_get_key(FALSE);         /* get a key from user */

      if(sysop_key!=sysop_color)       /* If color not set correctly */
         {
         if(sysop_key)                 /* if sysop was last person to type */
            {                          /* switch to sysop color */
            od_set_attrib(od_control.od_chat_color1);
            }
         else
            {                          /* else, set to user color */
            od_set_attrib(od_control.od_chat_color2);
            }

         sysop_color=sysop_key;        /* record current color setting */
         }

      if(inkey>=32 && inkey<=255)      /* if valid character */
         {
         od_putch(inkey);              /* display character typed */

         if(inkey==32)                 /* this is a new word if spacebar */
            {
            wordlen=0;
            *word=0;
            }

         else if(wordlen<70)           /* otherwise add character to word */
            {
            word[wordlen++]=inkey;
            word[wordlen]='\0';
            }

         if(colpos<75)                 /* record new column if not at end of */
            {                          /* screen */
            ++colpos;
            }

         else                          /* if at edge of screen */
            {
            if(wordlen<70 && wordlen>0)/* if word should be wrapped */
               {
               string=(char *)temp_str;/* generate string to erase old word */
               for(counter=0;counter<wordlen;++counter)
                  {
                  *(string++)=8;
                  }

               for(counter=0;counter<wordlen;++counter)
                  {
                  *(string++)=' ';
                  }

               *string='\0';
               od_disp_str(temp_str);  /* erase old word */
               od_disp_str("\n\r");
               od_disp_str(word);      /* display word on next line */
               colpos=wordlen;         /* record new column position */
               }

            else                       /* if no word wrap */
               {
               od_disp_str("\n\r");    /* send a carriage return */
               colpos=0;               /* now at column 0 */
               }

            wordlen=0;
            *word=0;
            }
         }

      if(inkey==8)                     /* if backspace */
         {
         od_disp_str(backstr);         /* send backspace sequence */
         if(wordlen>0)                 /* if in the middle of a word */
            {
            word[--wordlen]='\0';      /* erase last char from word */
            }
         if(colpos>0) --colpos;        /* record new column position */
         }

      else if(inkey==13)               /* if carriage return */
         {
         od_disp_str("\n\r");          /* send CR-LF sequence */
         wordlen=0;                    /* end the word */
         *word=0;
         colpos=0;                     /* back at column 0 */
         }
                                       /* giveup process after we have had */
                                       /* CPU for average of 5/182 s of    */
                                       /* processor time                   */
      else if(_clock_tick() != tick)
         {
         giveup_slice();
         }
      }


   od_set_attrib(od_control.od_chat_color1);
                                       /* indicate that chat mode has been */
                                       /* aborted */
   if(od_control.od_after_chat!=NULL)
      od_disp_str(od_control.od_after_chat);

   if(od_control.od_cafter_chat!=NULL) /* if function hook is defined */
      {
      _doing_cs_hook=TRUE;
      (*od_control.od_cafter_chat)();  /* call it! */
      _doing_cs_hook=FALSE;
      }

   if(_log_wrt!=NULL)
      (*_log_wrt)(10);

   /* Restore original display color attribute */
   od_set_attrib(original_attrib);

cleanup:
   if(temp_str != NULL)
      {
      free(temp_str);
      }
   }


/* Does not use globworkstr */
void od_disp(char *buffer,int size,char local_echo)
   {
   register int counter;               /* character output counter */

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_disp()");

   if(!inited) od_init();              /* initialize opendoors if not done */

   od_kernal();                        /* call the kernal */

   if(od_control.baud!=0)
      {
      _com_send_buf(buffer, size);
      }

   if(local_echo)
      {                                /* loop through the buffer */
      for(counter=0;counter<size;++counter)
         {
         phys_putch(buffer[counter]);  /* send character to local window */
         }
      }

   od_kernal();                        /* call the kernal again */
   }



void od_disp_str(char *string)         /* function to display ASCIIZ string */
   {
   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_disp_str()");

   /* od_disp() will initialize OpenDoors if not already done */
   od_disp(string,strlen(string),FALSE);
   phys_cputs(string);
   }



void od_set_statusline(char num)       /* function to display status line */
   {
   register char counter;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_set_statusline()");

   if(!inited) od_init();              /* verify that we've been initialized */

   if(!od_control.od_status_on) return;/* check if status line is on */

   if(num<0 || num>8) num=0;

   if(!od_control.od_update_status_now && num==status_line) return;


   store_cursor();                     /* store console settings */
   phys_window(1,1,80,25);

   if(status_line==8)
      {
      if((counter=user_text.cury-(1+_output_bottom-_output_top))>0)
         {
         phys_movetext(1,_output_top+counter,80,_output_bottom+counter,_output_top,1);
         user_text.cury=1+_output_bottom-_output_top;
         }
      else if(user_text.cury<_output_top)
         {
         user_text.cury=_output_top;
         phys_movetext(1,_output_top+24-_output_bottom,80,25,_output_top,1);
         }
      }

   od_control.od_current_statusline=status_line=num;


   if(num==8)
      {
      phys_setattrib(0x07);

      for(counter=1;counter<=25;++counter)
         {
         if(counter<_output_top || counter>_output_bottom)
            {
            if(counter==25)
               {
               phys_puttext(80,25,80,25,blankblock);
               phys_gotoxy(1,25);
               phys_cputs("                                                                               ");
               }
            else
               {
               phys_gotoxy(1,24);
               phys_cputs("                                                                                ");
               }
            }
         }

      phys_setattrib(user_text.attribute);
      phys_gotoxy(user_text.curx,user_text.cury);
      }

   else
      {
      phys_cursor(FALSE);
      phys_setscroll(FALSE);

      (*current_status_function)(num);

      phys_cursor(TRUE);
      phys_setscroll(TRUE);

      phys_window(1,_output_top,80,_output_bottom);
      phys_setattrib(user_text.attribute);
      phys_gotoxy(user_text.curx,user_text.cury);
      }
   }



void store_cursor(void)                /* function to store cursor position */
   {                                   /* screen colors and window settings */
   phys_gettextinfo(&user_text);
   }



void restore_cursor(void)              /* function to restore data saved by */
   {                                   /* store_cursor() */
   phys_window(user_text.winleft,user_text.wintop,user_text.winright,user_text.winbottom);
   phys_setattrib(user_text.attribute);
   phys_gotoxy(user_text.curx,user_text.cury);
   }



void toname(char *string)              /* convert string to name format */
   {
   strlwr(string);                     /* change string to lower case */
   *string=toupper(*string);           /* change first character to upper case */
   if(string[strlen(string)-1]=='\n')  /* trim CR from end of string, if any */
      string[strlen(string)-1]='\0';
                                       /* capitalize other words in name */
   while(*string) if((*string++)==' ') *string=toupper(*string);
   }



void od_set_attrib(int color)          /* function to set cursor color */
   {
   static char string[40];
   register int attrib=color;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_set_attrib()");

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   if(attrib==-1) return;

   if(od_control.od_avatar)            /* check for AVATAR mode */
      {
      if(od_control.od_cur_attrib!=attrib || od_control.od_full_colour)
         {
         phys_setattrib(od_control.od_cur_attrib=attrib);  /* change local text color */

         string[0]=22;                 /* generate AVATAR control sequence */
         string[1]=1;
         string[2]=attrib;
         od_disp(string,3,FALSE);      /* send AVATAR control sequence */
         }
      }

   else if(od_control.caller_ansi)     /* check for ANSI mode */
      {                                /* send ANSI control string */
      _yet=FALSE;

      if(od_control.od_cur_attrib==-1 || od_control.od_full_colour)
         {
ansi_reset:
         add_to_sequence(string,0);    /* reset ANSI terminal status */

         if(attrib&0x80)               /* if blink attribute is set */
            {
            add_to_sequence(string,5); /* add it to the ANSI color sequence */
            }

         if(attrib&0x08)               /* if high intensity attribute is set */
            {
            add_to_sequence(string,1); /* add it to the ANSI color sequence */
            }
         }

      else                             /* if current colour IS known */
         {                             /* if have to reset flashing or bright */
         if( ((od_control.od_cur_attrib&0x80) && !(attrib&0x80)) || ((od_control.od_cur_attrib&0x08) && !(attrib&0x08)))
            {
            od_control.od_cur_attrib=-1;             /* must reset entire colour settings */
            goto ansi_reset;
            }
                                       /* If flashing has to be turned on */
         if((attrib&0x80)!=(od_control.od_cur_attrib&0x80))
            {
            add_to_sequence(string,5); /* add it to the ANSI color sequence */
            }
                                       /* If bright has to be turned on */
         if((attrib&0x08)!=(od_control.od_cur_attrib&0x08) || od_control.od_cur_attrib==-1)
            {
            add_to_sequence(string,1); /* add it to the ANSI color sequence */
            }
         }


                                       /* if foreground color has changed */
      if((attrib&0x07)!=(od_control.od_cur_attrib&0x07) || od_control.od_cur_attrib==-1 || od_control.od_full_colour)
         {                             /* add translated color to sequence */
         add_to_sequence(string,ibm2ansii[attrib&0x07]);
         }
                                       /* if background color has changed */
      if((attrib&0x70)!=(od_control.od_cur_attrib&0x70) || od_control.od_cur_attrib==-1 || od_control.od_full_colour)
         {                             /* add translated color to sequence */
         add_to_sequence(string,ibm2ansii[(attrib&0x70)>>4]+10);
         }


      if(_yet)                         /* if any change in color */
         {
         strcat(string,"m");           /* append change-attribute command */
                                       /* send ANSI sequence to modem */
         od_disp(string,strlen(string),FALSE);
         }

      phys_setattrib(od_control.od_cur_attrib=attrib);     /* chage local text color */
/*  Code to facilitate ANSI testing: */
/*      printf("[(null) ...]",(char *)&string); */
/*      cprintf("[(null)]",(char *)&string); */
      }
   else
      {
      od_control.od_error = ERR_NOGRAPHICS;
      }
   }



void add_to_sequence(char *string,char value)
   {
   char str[5];

   if(_yet)
      {
      sprintf(str,";%d",value);
      strcat(string,str);
      }
   else
      {
      _yet=TRUE;
      sprintf(string,"x[%d",value);
      string[0]=27;
      }
   }


void od_putch(int character)
   {
   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_putch()");

   if(!inited) od_init();              /* Initialize OpenDoors if not done */

   phys_putch(character);              /* Display character locally */
   _remotechar(character);             /* Display character remotely */
   }


void _remotechar(int character)
   {
   if(!inited) od_init();              /* initialize OpenDoors if not done */

   if(od_control.baud)                 /* if not operating in local mode */
      {
      _com_sendchar(character);
      }

   /* If it is time to call kernel */
   if(_last_kernel+4L <= _clock_tick()
      || _clock_tick() < _last_kernel)
      {
      od_kernal();                           /* Call kernal function */
      }
   }


void od_set_dtr(char high)
   {
   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_set_dtr()");

   if(!inited) od_init();              /* verify that we've been initialized */

   if(!od_control.baud)
      {
      od_control.od_error = ERR_NOREMOTE;
      return;
      }

   _com_dtr(high);
   }


char od_get_answer(char *string)
   {
   register char *character;
   register char pressed;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_get_answer()");

   if(!inited) od_init();              /* initialize OpenDoors, if needed */

   for(;;)
      {
      pressed=tolower(od_get_key(TRUE));
      character=(char *)string;
      while(*character)
         {
         if(tolower(*character)==pressed) return (*character);
         ++character;
         }
      }
   }



unsigned char od_colour_config(char *config_line)
   {
   register unsigned char colour=0x07;       /* Default colour is grey on black */
   static char token[40];
   char *start=(char *)config_line;
   char *end;
   char length;
   char identifier;
   char foreground=TRUE;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_colour_config()");

   if(!inited) od_init();              /* verify that we've been initialized */

   while(*start && *start!=colour_check_char)
      {
      if(*start == ' ' || *start== '\t')
         {
         ++start;
         }
      else
         {
         length=0;
         end=(char *)start;
         while(*end && *end!=colour_check_char && *end != ' ' && *end != '\t')
            {
            ++length;
            ++end;
            }

         if(length>39) length=39;
         strncpy(token,start,length);
         token[length]='\0';
         strupr(token);

         for(identifier=0;identifier<12;++identifier)
            if(strcmp(od_config_colours[identifier],token)==0)
               {
               if(identifier<=9)
                  {
                  if(identifier>=8) identifier-=2;

                  if(foreground)
                     {
                     foreground=FALSE;
                     colour&=~0x07;
                     colour|=identifier;
                     }
                  else
                     {
                     colour&=~0x70;
                     colour|=(identifier<<4);
                     }
                  }

               else if(identifier==10)
                  {
                  colour|=0x08;
                  }

               else if(identifier==11)
                  {
                  colour|=0x80;
                  }

               break;
               }

         start=(char *)end;
         }
      }

   colour_end_pos=(char *)start;

   return(colour);
   }


int _pageprompt(char *pausing)
   {
   char prompt_len = strlen(od_control.od_continue);
   struct phys_text_info info;
   int to_return = FALSE;
   char key;
   char counter;

   if(!*pausing) return(FALSE);        /* exit if page pausing is disabled */

   phys_gettextinfo(&info);            /* get current text colour */

                                       /* set to prompt colour */
   od_set_attrib(od_control.od_continue_col);

   od_disp_str(od_control.od_continue);/* display prompt string */

   od_set_attrib(info.attribute);      /* restore text colour */

   for(;;)                             /* loop until valid choice */
      {
      key=od_get_key(TRUE);            /* get a key from the user */

      if(key==tolower(od_control.od_continue_yes) ||
         key==toupper(od_control.od_continue_yes) ||
         key==13)                      /* if user chose to continue */
         {
         goto finished_pausing;        /* remove prompt & resume */
         }

      else if(key==tolower(od_control.od_continue_nonstop) ||
              key==toupper(od_control.od_continue_nonstop))
         {                             /* if user chose nonstop transmission */
            *pausing=FALSE;            /* disable page pausing */
            goto finished_pausing;     /* remote prompt & resume */
         }

      else if(key==tolower(od_control.od_continue_no) ||
              key==toupper(od_control.od_continue_no) ||
              key=='s' || key=='S' || key==3 || key==11 || key==0x18)
            {                          /* if user chose to stop display */
            if(od_control.baud)        /* if operating in remote mode */
               {                       /* clear outbound buffer */
               _com_clear_outbound();
               }

            to_return = TRUE;          /* stop display of file/menu/list */

            goto finished_pausing;
            }
      }

finished_pausing:                      /* remove pause prompt */
   for(counter=0;counter<prompt_len;++counter)
      od_disp_str(backstr);

   return(to_return);
   }
