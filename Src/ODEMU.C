/*
 * €€€€€€€€€€			      €€€€€€€‹
 * €€€ﬂﬂﬂﬂ€€€			      €€€ﬂﬂﬂ€€€
 * €€€	  €€€ ‹‹‹‹‹‹‹ ‹‹‹‹‹‹‹ ‹‹‹‹‹‹‹ €€€   €€€ ‹‹‹‹‹‹‹ ‹‹‹‹‹‹‹ ‹‹‹‹‹‹ ‹‹‹‹‹‹‹
 * €€€	  €€€ €€€ﬂ€€€ €€€ﬂ€€€ €€€ﬂ€€€ €€€   €€€ €€€ﬂ€€€ €€€ﬂ€€€ €€€ﬂﬂﬂ €€€ﬂﬂﬂﬂ
 * €€€‹‹‹‹€€€ €€€ €€€ €€€ﬂﬂﬂﬂ €€€ €€€ €€€‹‹‹€€€ €€€ €€€ €€€ €€€ €€€    ﬂﬂﬂﬂ€€€
 * €€€€€€€€€€ €€€€€€€ €€€€€€€ €€€ €€€ €€€€€€€ﬂ	€€€€€€€ €€€€€€€ €€€    €€€€€€€
 *	      €€€
 *	      €€€
 *	      ﬂﬂﬂ				      Door Programming Toolkit
 * ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ
 *
 *	(C) Copyright 1991 - 1994 by Brian Pirie. All Rights Reserved.
 *
 *
 *
 *
 *     Filename : ODEMU.C
 *  Description : Code for the TTY/ANSI/AVATAR terminal emulation routines,
 *		  including .ASC/.ANS/.AVT/.RIP file display functions.
 *	Version : 5.00
 */





#include<stdio.h>		       /* Standard header files */
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<stdarg.h>

#ifndef USEINLINE		       /* If not using inline assembly, */
#include<dos.h> 		       /* include <dos.h> for int86() */
#endif

#include "opendoor.h"                 /* Include OpenDoors header files */
#include "odintern.h"

static void _emubuf(char *buf, char remote_echo);

				       /* TERMINAL EMULATION VARIABLES */
				       /* ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ */
char ansi_color[8]={0,4,2,6,1,5,3,7};  /* ANSI to IBM colour translation table */
char seq_level=0;		       /* ANSI sequence level */
int param[10];			       /* ANSI sequence parameters */
char param_str[4]="";                  /* string containting current parameter */
char param_str_len;		       /* length of the parameter string */
char saved_x=1; 		       /* stored x position */
char saved_y=1; 		       /* stored y position */
char repeat_str[129];		       /* string repeated with ^V^Y */
unsigned char repeat_count;	       /* counter used for AVATAR string repeats */
char avt_seq=0; 		       /* AVATAR sequence level */
char prev_param;		       /* value of previous terminal parameter */
char num_params;		       /* number of terminal parameters */
char def_attr=7;		       /* default screen attribute */
char avt_insert=FALSE;		       /* is AVATAR insert mode on */
char scroll_lines;		       /* used for AVATAR area commands */
char scroll_x1,scroll_y1,scroll_x2,scroll_y2;

char *_hotkeys=NULL;		       /* variables for tracking hotkeys */
char _hotkey_pressed;		       /* while displaying a menu file */


/* od_hotkey_menu() - Displays a .ASC/.ANS/.AVT/.RIP menu file, watching    */
/*		      for any of a specified list of hotkeys. If the hotkey */
/*		      is pressed during menu display, display stops	    */
/*		      immediately, and key pressed by user is returned.     */
/*		      If the wait flag is set, od_hotkey_menu will wait     */
/*		      after displaying the menu, until the user presses one */
/*		      terminated string of ASCII key characters. The	    */
/*		      terminated string of ASCII characterse. The filename  */
/*		      to display is specified in the same format as the     */
/*		      filename passed to od_send_file().		    */
char od_hotkey_menu(char *filename,char *hotkeys,char wait)
   {
   char pressed;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_hotkey_menu()");

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   if(!hotkeys)
      {
      od_control.od_error = ERR_PARAMETER;
      return(0);
      }

   /* Store pointer to string of hotkeys in global hotkey array for access */
   /* from od_send_file(). */
   _hotkeys=(char *)hotkeys;

   /* Clear the hotkey status variable */
   _hotkey_pressed='\0';

   /* Display the menu file using od_send_file() primitive */
   if(!od_send_file(filename))
      {
      return(0);
      }

   /* Clear the global hotkey array */
   _hotkeys=NULL;

   /* If the file display was interrupted by the pressing of one of the */
   /* hotkeys, return the pressed hotkey immediately.			*/
   if(_hotkey_pressed!='\0') return(_hotkey_pressed);

   /* If no hotkey has been pressed key, and the wait flag has been set, */
   /* wait for the user to press a valid hotkey.			 */
   if(wait)
      {
      /* Wait for a valid hotkey using the od_get_answer() primitive */
      pressed=od_get_answer(hotkeys);

      /* If a remote user is connected on this node */
      if(od_control.baud)
	 {
	 /* Clear the FOSSIL outbound buffer */
#ifdef USEINLINE
	 ASM	mov dx, od_control.port
	 ASM	mov ah, 0x09
	 ASM	push di
	 ASM	push si
	 ASM	int 20
	 ASM	pop si
	 ASM	pop di
#else
	 regs.h.ah=0x09;
	 regs.x.dx=od_control.port;
	 int86(20,&regs,&regs);
#endif
	 }

      /* Return the hotkey pressed by the user */
      return(pressed);
      }

   /* No hotkey has been pressed, so return 0 */
   /* (Since 0 is used to terminate the string of valid hotkeys, it can */
   /* never be a valid hotkey itself, and is therefore a safe value to */
   /* indicate the "no key press" state.) */
   return(0);
   }


/* Displays a .ASC/.ANS/.AVT/.RIP file to local and remote screens. Since */
/* the text-mode version of OpenDoors cannot support local .RIP graphics, */
/* the equivalent .ASC/.ANS/.AVT file is displayed on the local screen	  */
/* while the .RIP file is displayed remotely.				  */
int od_send_file(char *filename)
   {
   register FILE *remotefile;	       /* file to send to remote system */
   register FILE *localfile=NULL;      /* file to display locally, if differnt */
   char local=TRUE;		       /* is anything to be displayed locally */
   void *under;
   int strwidth;
   int x1;
   int winwidth;
   int level;			       /* emulation level of file */
   register char counter;
   char pausing;
   char key;
   char *parsepos;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_send_file()");

   if(!inited) od_init();	       /* initialize OpenDoors, if needed */

   if(!filename)
      {
      od_control.od_error = ERR_PARAMETER;
      return(FALSE);
      }

   counter=2;			       /* initialize local variables */

   pausing=od_control.od_page_pausing; /* turn on page pausing, if available */

   if(strchr(filename,'.')==NULL)      /* if operating in auto-filename mode */
      { 			       /* (no extention) */
      level = 4;		       /* begin by searching for a .RIP file */
      if((remotefile = _gfilesearch(filename, &level)) == NULL)
	 {			       /* if no .ASC/.ANS/.AVT/.RIP file */
	 od_control.od_error = ERR_FILEOPEN;
	 return(FALSE); 	       /* then return with an error */
	 }
      if(level == 4)		       /* if the file found was a .RIP */
	 {			       /* search for file to display locally */
	 level = 3;
	 pausing = FALSE;	       /* no page pausing with .RIP display */
         if((localfile = _gfilesearch(filename, &level)) == NULL)
	    {			       /* if no further .ASC/.ANS/.AVT/.RIP */
	    local=FALSE;	       /* files, then no local display */
	    }
	 }
      else if (level == 0)
	 {
	 od_control.od_error = ERR_FILEOPEN;
	 return(FALSE);
	 }

      strcpy(globworkstr,filename);    /* Get filename of remote file */
      strcat(globworkstr,".RIP");
      strupr(globworkstr);
      }
   else 			       /* if full filename was specified */
      { 			       /* then attempt to open that file */
      if ((remotefile=fopen(filename,"rb"))==NULL)
	 {			       /* if unable to open file, then return */
	 od_control.od_error = ERR_FILEOPEN;
	 return(FALSE);
	 }

      strcpy(globworkstr,filename);    /* place upper-case filename in */
      strupr(globworkstr);	       /* globworkstr */

      if(strstr(globworkstr,".RIP"))   /* if it is a .RIP file */
	 {
	 pausing=FALSE; 	       /* no page pausing during .RIP display */
	 local=FALSE;		       /* disable local display */
	 }
      }

   def_attr=0x07;		       /* set default colour to grey on black */

   avt_seq=0;			       /* reset all terminal emulation */
   seq_level=0;
   avt_insert=FALSE;		       /* turn off AVATAR insert mode */

   _last_control_key=0; 	       /* reset [S]top/[P]ause control key */
				       /* status */
   if(!local)
      {
      if((strwidth = strlen(parsepos = globworkstr)) > 76)
	 {
	 parsepos += strwidth - 76;
	 memcpy(parsepos, "...", 3);
	 parsepos += 3;
	 strwidth = 76;
	 }
      winwidth = strwidth <= 20 ? 23 : strwidth + 3;
      x1 = 40 - (winwidth/2);

      store_cursor();
      phys_cursor(FALSE);
      if(!(under = phys_createwin(x1,10,x1+winwidth, 14, 0x19, od_control.od_sending_rip, 0x19)))
	 {
	 /* od_control.od_error will be set by phys_createwin() */
	 return(FALSE);
	 }
      phys_gotoxy(40 - ((strwidth-1)/2), 12);
      phys_cputs(parsepos);
      restore_cursor();
      }


   /* loop to display each line in the file(s) with page pausing, etc. */
   for(;;)
      {
      od_kernal();		       /* call the OpenDoors kernal routine */


      if(_hotkeys!=NULL)	       /* if hotkeys are active */
	 {			       /* if a key is waiting in buffer */
	 while((key=(char)tolower(od_get_key(FALSE))) != 0)
	    {
	    parsepos=(char *)_hotkeys; /* check for key in hotkey string */
	    while(*parsepos)
	       {		       /* if key is found */
	       if(tolower(*parsepos)==key)
		  {		       /* return, indicating that hotkey */
				       /* was pressed */
		  _hotkey_pressed=*parsepos;
		  goto abort_send;
		  }
	       ++parsepos;
	       }
	    }
	 }


      if(_last_control_key)	       /* If a control key has been pressed */
	 {
	 switch(_last_control_key)
	    {
	    case 's':                  /* If it was a stop control key */
	       if(od_control.od_list_stop)
		  {		       /* If enabled, clear keyboard buffer */
abort_send:
		  if(od_control.baud)  /* If operating in remote mode */
		     {		       /* clear the outbound FOSSIL buffer */
		     _com_clear_outbound();
		     }		       /* exit od_send_file() */
		  goto end_transmission;
		  }
	       break;

	    case 'p':                  /* if control key was "pause" */
	       if(od_control.od_list_pause)
		  {		       /* if pause is enabled */
		  od_clear_keybuffer();/* clear keyboard buffer */
		  od_get_key(TRUE);    /* wait for any keypress */
		  }
	    }
	 _last_control_key=0;	       /* clear control key status */
	 }
				       /* get next line, if any */
      if(fgets(globworkstr,256,remotefile)==NULL)
	 {
	 if(localfile)		      /* if different local file */
	    {			      /* display rest of it */
	    while(fgets(globworkstr,256,localfile))
	       {
		/* pass each line to terminal emulator */
	       _emubuf(globworkstr, FALSE);
	       }
	    }
	 goto end_transmission;        /* exit od_send_file() */
	 }

      /* Set parsepos == last char in globworkstr */
      parsepos = (char *)&globworkstr;
      while(*++parsepos) ;
      --parsepos;

				       /* check for end of page state */
      if((*parsepos == '\r' || *parsepos == '\n') &&
	 ++counter>=od_control.user_screen_length && pausing)
	 {
	 if(_pageprompt(&pausing))     /* display page pause prompt */
	    {			       /* if user chose to abort */
	    goto end_transmission;     /* exit od_send_file() */
	    }
	 counter=2;		       /* reset line count */
	 }


      if(local) 		       /* if file is also to be displayed */
	 {			       /* locally */
				       /* if different local file, get next */
				       /* line from it */
         if(localfile)
            {
            od_disp(globworkstr,strlen(globworkstr),FALSE);

            if(fgets(globworkstr,256,localfile)==NULL)
               {
               while(fgets(globworkstr,256,remotefile))
                  {
                  od_disp(globworkstr,strlen(globworkstr),FALSE);
                  }
               goto end_transmission;  /* exit od_send_file() */
               }

            _emubuf(globworkstr,FALSE);
            }
         else
            {
            if(od_control.od_no_ra_codes)
               {
               _emubuf(globworkstr,FALSE);/* pass string through local */
                                          /* terminal emulation system */
                od_disp(globworkstr,strlen(globworkstr),FALSE);
               }
            else
               {
               _emubuf(globworkstr,TRUE); /* pass string through local */
                                          /* terminal emulation system */
               }
            }
         }
      else
         {
                                       /* if file is not being displayed */
                                       /* locally, send line to remote   */
                                       /* system (if any)                */
         od_disp(globworkstr,strlen(globworkstr),FALSE);
         }
      }

end_transmission:
   fclose(remotefile);		      /* close remote file */
   if(localfile)		      /* if different local file */
      {
      fclose(localfile);	      /* close local file */
      }

   if(!local)
      {
      /* wait while file is being sent */
      if(od_control.baud != 0)
	 {
	 while(_com_outbound())
	    {
	    od_kernel();
	    }
	 }

      /* get rid of window */
      store_cursor();
      phys_delwin(x1, 10, x1+winwidth, 14, under);
      restore_cursor();
      phys_cursor(TRUE);
      }

   return(TRUE);		       /* indicate file display success */
   }


/* _gfilesearch() - Searches for a graphical file (.RIP/.AVT/.ANS/.ASC)     */
/*		    with the base filename specified in basename. The level */
/*		    parameter indicates the highest "level" to search, and  */
/*		    can be one of the following values: 		    */
/*			     4 - .RIP					    */
/*			     3 - .AVT					    */
/*			     2 - .ANS					    */
/*			     1 - .ASC					    */
/*			     0 - none					    */
/*		    This function updates the value of the integer pointed  */
/*		    to by level to reflect the type of file found, and	    */
/*		    returns a pointer to the open file structure	    */
/*		    corresponding to this file. If no suitable file is	    */
/*		    found, this function returns NULL, and sets level to 0. */
FILE *_gfilesearch(char *basename, int *level)
   {
   FILE *fp;

   for(;*level>0;--*level)	    /* loop though .AVT/.ANS/.ASC extentions */
      {
      strcpy(globworkstr,basename); /* get base-filename passed to us */

      switch(*level)		    /* try current .AVT/.ASC/.ASC extention */
	 {
	 case 4:		    /* try .RIP extention */
	    if(!od_control.user_rip) continue;
	    strcat(globworkstr,".RIP");
	    break;

	 case 3:		    /* try .AVT extention */
	    if(!od_control.user_avatar) continue;
	    strcat(globworkstr,".AVT");    /* add extention to filename */
	    break;

	 case 2:
	    if(!od_control.user_ansi) continue;
	    strcat(globworkstr,".ANS");    /* try .ANS extention */
	    break;			  /* add extention to filename */

	 case 1:			   /* try .ASC extention */
	    strcat(globworkstr,".ASC");    /* add extention to filename */
	 }

      if((fp=fopen(globworkstr,"rb"))!=NULL)/* if able to open filename */
	 return(fp);			    /* return the file pointer */
      }

   return(NULL);			    /* return NULL if no file found */
   }


/* od_disp_emu() - Sends an entire string to both local and remote	     */
/*		   systems. The characters displayed locally are fed through */
/*		   the local terminal emulation sub-system, allowing	     */
/*		   aribtrary ANSII/AVATAR control sequences to be displayed  */
/*		   both locally and remotely.				     */
void od_disp_emu(char *string, char remote_echo)
   {
   char remote_via_emubuf;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_disp_emu()");

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   /* Send string to remote system */
   if(remote_echo)
      {
      if(od_control.od_no_ra_codes)
	 {
	 od_disp(string, strlen(string), FALSE);
	 remote_via_emubuf = FALSE;
	 }
      else
	 {
	 remote_via_emubuf = TRUE;
	 }
      }
   else
      {
      remote_via_emubuf = FALSE;
      }

   /* Pass string to be emulated to local terminal emulation function */
   _emubuf(string, remote_via_emubuf);
   }


/* od_emulate() - Sends a single character to both local and remote	    */
/*		  systems. The characters displayed locally are fed through */
/*		  the local terminal emulation sub-system, allowing	    */
/*		  aribtrary ANSII/AVATAR control sequences to be displayed  */
/*		  both locally and remotely.				    */
void od_emulate(char in_char)
   {
   static char emulate_buf[2];
   *emulate_buf = in_char;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_emulate()");

   /* Ensure that OpenDoors has been initialized */
   if(!inited) od_init();

   /* Pass character to be emulated to local terminal emulation function */
   _emubuf(emulate_buf, TRUE);
   }


/* _emubuf() - Displays a string on the local screen, interpreting any */
/*	       ANSI/AVATAR terminal emulation control sequences. */
/* (does not use globworkstr) */
static void _emubuf(char *buf, char remote_echo)
{
   static struct phys_text_info info;
   register int temp_int;
   char echo_this_char;

   while(*buf)
   {
   echo_this_char = remote_echo;

   switch(seq_level)		       /* switch according to position in */
      { 			       /* ANSI command string */
      case 0:			       /* if no ANSI command string */
	 switch(*buf)		       /* switch according to character */
	    {
	    case 27:		       /* if char is an (esc) */
	       if(avt_seq==0)	       /* if we are not in the middle of an */
		  {		       /* AVATAR sequence, */
		  seq_level=1;	       /* then start an ANSI sequence */
		  break;
		  }

	    default:		       /* if not start of an ANSI sequence */
	       switch(avt_seq)	       /* check our position in AVATAR sequence */
		  {
		  case 0:	       /* if not in middle of an AVATAR command */
		     switch(*buf)      /* check the character we've been sent */
			{
			case 0x01:     /* QBBS/RA pause for keypress code */
			   if(od_control.od_no_ra_codes) goto output_next_char;
			   /* Wait for user to press [ENTER]/[RETURN] key. */
			   od_get_answer("\n\r");
			   echo_this_char = FALSE;
			   break;
			case 0x06:     /* QBBS/RA ^F user parameters */
			   if(od_control.od_no_ra_codes) goto output_next_char;
			   avt_seq=21;
			   echo_this_char = FALSE;
			   break;

			case 0x0b:     /* QBBS/RA ^K user parameters */
			   if(od_control.od_no_ra_codes) goto output_next_char;
			   avt_seq=22;
			   echo_this_char = FALSE;
			   break;

			case 0x0c:
			   avt_insert=FALSE;
			   phys_setattrib(od_control.od_cur_attrib=def_attr);
			   phys_clrscr();
			   break;

			case 0x19:
			   avt_insert=FALSE;
			   avt_seq=1;
			   break;

			case 0x16:   /* ^V */
			   avt_seq=3;
			   break;

			default:
output_next_char:
			   /* Output next character. */
			   if(avt_insert)
			      {
			      phys_gettextinfo(&info);
			      if(info.curx<80)
				 {
				 phys_movetext(info.curx,info.cury,79,info.cury,info.curx+1,info.cury);
				 }
			      phys_putch(*buf);
			      }

			   else
			      {
			      phys_putch(*buf);
			      }
			}
		     break;

		   case 1:
		     avt_insert=FALSE;
		     prev_param=*buf;
		     avt_seq=2;
		     break;

		  case 2:
		     for(temp_int=0;temp_int<(unsigned int)*buf;++temp_int)
			{
			phys_putch(prev_param);
			}
		     avt_seq=0;
		     break;

		  case 3:
		     switch(*buf)
			{
			case 0x01:
			   avt_insert=FALSE;
			   avt_seq=4;
			   break;

			case 0x02:
			   avt_insert=FALSE;
			   phys_gettextinfo(&info);
			   phys_setattrib(od_control.od_cur_attrib=info.attribute|0x80);
			   avt_seq=0;
			   break;

			case 0x03:
			   avt_insert=FALSE;
			   phys_gettextinfo(&info);
			   if(info.cury>1)
			      {
			      phys_gotoxy(info.curx,info.cury-1);
			      }
			   avt_seq=0;
			   break;

			case 0x04:
			   avt_insert=FALSE;
			   phys_gettextinfo(&info);
			   if(info.cury<25)
			      {
			      phys_gotoxy(info.curx,info.cury+1);
			      }
			   avt_seq=0;
			   break;

			case 0x05:
			   avt_insert=FALSE;
			   phys_gettextinfo(&info);
			   if(info.curx>1)
			      {
			      phys_gotoxy(info.curx-1,info.cury);
			      }
			   avt_seq=0;
			   break;

			case 0x06:
			   avt_insert=FALSE;
			   phys_gettextinfo(&info);
			   if(info.curx<80)
			      {
			      phys_gotoxy(info.curx+1,info.cury);
			      }
			   avt_seq=0;
			   break;

			case 0x07:
			   avt_insert=FALSE;
			   phys_clreol();
			   avt_seq=0;
			   break;

			case 0x08:
			   avt_insert=FALSE;
			   avt_seq=5;
			   break;

			case 0x09:   /* ^I */
			   avt_insert=TRUE;
			   avt_seq=0;
			   break;

			case 0x0a:   /* ^J */
			   scroll_lines=-1;
			   avt_seq=7;
			   break;

			case 0x0b:   /* ^K */
			   scroll_lines=1;
			   avt_seq=7;
			   break;

			case 0x0c:   /* ^L */
			   avt_seq=14;
			   break;

			case 0x0d:   /* ^M */
			   avt_seq=15;
			   break;

			case 0x0e:   /* ^N */
			   phys_gettextinfo(&info);
			   if(info.curx<80)
			      {
			      phys_movetext(info.curx+1,info.cury,80,info.cury,info.curx,info.cury);
			      }

			   phys_setscroll(FALSE);
			   phys_gotoxy(80,info.cury);
			   phys_putch(' ');
			   phys_setscroll(TRUE);
			   phys_gotoxy(info.curx,info.cury);

			   avt_seq=0;
			   break;

			case 0x19:   /* ^Y */
			   avt_seq=19;
			   break;

			default:
			   avt_seq=0;
			}
		     break;

		  case 4:
		     phys_setattrib(od_control.od_cur_attrib=*buf);
		     avt_seq=0;
		     break;

		  case 5:
		     prev_param=*buf;
		     avt_seq=6;
		     break;

		  case 6:
		     phys_gotoxy(*buf,prev_param);
		     avt_seq=0;
		     break;

		  case 7:
		     if(scroll_lines<1)
			{
			scroll_lines=*buf;
			}
		     else
			{
			scroll_lines=-*buf;
			}
		     avt_seq=8;
		     break;

		  case 8:
		     scroll_y1=*buf;
		     avt_seq=9;
		     break;

		  case 9:
		     scroll_x1=*buf;
		     avt_seq=10;
		     break;

		  case 10:
		     scroll_y2=*buf;
		     avt_seq=11;
		     break;

		  case 11:
		     scroll_x2=*buf;
		     avt_seq=12;
		     break;

		  case 12:
		     if(scroll_lines==0 || abs(scroll_lines)>(scroll_y2-scroll_y1))
			{
			fill_area(scroll_x1,scroll_y1,scroll_x2,scroll_y2,' ');
			}

		     else if(scroll_lines<0)
			{
			phys_movetext(scroll_x1,scroll_y1,scroll_x2,scroll_y2+scroll_lines,scroll_x1,scroll_y1-scroll_lines);
			fill_area(scroll_x1,scroll_y1,scroll_x2,scroll_y1-(scroll_lines-1),' ');
			}

		     else
			{
			phys_movetext(scroll_x1,scroll_y1+scroll_lines,scroll_x2,scroll_y2,scroll_x1,scroll_y1);
			fill_area(scroll_x1,scroll_y2-(scroll_lines-1),scroll_x2,scroll_y2,' ');
			}
		     avt_seq=0;
		     break;

		  case 14:
		     scroll_lines=(*buf&0x7f);
		     scroll_x2=' ';
		     avt_seq=17;
		     break;

		  case 15:
		     scroll_lines=(*buf&0x7f);
		     avt_seq=16;
		     break;

		  case 16:
		     scroll_x2=*buf;
		     avt_seq=17;
		     break;

		  case 17:
		     scroll_y1=*buf;
		     avt_seq=18;
		     break;

		  case 18:
		     phys_gettextinfo(&info);
		     phys_setattrib(od_control.od_cur_attrib=scroll_lines);
		     fill_area(info.curx,info.cury,info.curx+*buf,info.cury+scroll_y1,scroll_x2);
		     avt_seq=0;
		     break;

		  case 19:
		     scroll_lines=(*buf&0x7f);
		     repeat_str[repeat_count=0]='\0';
		     avt_seq=20;
		     break;

		  case 20:
		     if(repeat_count<scroll_lines)
			{
			repeat_str[repeat_count]=*buf;
			repeat_str[++repeat_count]='\0';
			}
		     else
			{
			for(repeat_count=0;repeat_count<*buf;++repeat_count)
			   phys_cputs(repeat_str);
			avt_seq=0;
			}
		     break;

		  case 21:	       /* RA/QBBS ^F control codes */
		     echo_this_char = FALSE;
		     switch(*buf)
			{
			case 'A':
			   od_disp_str(od_control.user_name);
			   break;
			case 'B':
			   od_disp_str(od_control.user_location);
			   break;
			case 'C':
			   od_disp_str(od_control.user_password);
			   break;
			case 'D':
			   od_disp_str(od_control.user_dataphone);
			   break;
			case 'E':
			   od_disp_str(od_control.user_homephone);
			   break;
			case 'F':
			   od_disp_str(od_control.user_lastdate);
			   break;
			case 'G':
			   od_disp_str(od_control.user_lasttime);
			   break;
			case 'H':
			   scroll_lines=0;
			   goto show_flags;
			case 'I':
			   scroll_lines=1;
			   goto show_flags;
			case 'J':
			   scroll_lines=2;
			   goto show_flags;
			case 'K':
			   scroll_lines=3;
show_flags:		   for(repeat_count=0;repeat_count<8;++repeat_count)
			      {
			      if((od_control.user_flags[scroll_lines]>>repeat_count)&0x01)
				 {
				 repeat_str[repeat_count]='X';
				 }
			      else
				 {
				 repeat_str[repeat_count]='-';
				 }
			      }
			   repeat_str[repeat_count]='\0';
			   od_disp_str(repeat_str);
			   break;
			case 'L':
			   od_printf("%lu",od_control.user_credit);
			   break;
			case 'M':
			   od_printf("%u",od_control.user_messages);
			   break;
			case 'N':
			   od_printf("%u",od_control.user_lastread);
			   break;
			case 'O':
			   od_printf("%u",od_control.user_security);
			   break;
			case 'P':
			   od_printf("%u",od_control.user_numcalls);
			   break;
			case 'Q':
			   od_printf("%ul",od_control.user_uploads);
			   break;
			case 'R':
			   od_printf("%ul",od_control.user_upk);
			   break;
			case 'S':
			   od_printf("%ul",od_control.user_downloads);
			   break;
			case 'T':
			   od_printf("%ul",od_control.user_downk);
			   break;
			case 'U':
			   od_printf("%d",od_control.user_time_used);
			   break;
			case 'V':
			   od_printf("%d",od_control.user_screen_length);
			   break;
			case 'W':
			   repeat_count=0;
			   while(od_control.user_name[repeat_count])
			      {
			      if((repeat_str[repeat_count]=od_control.user_name[repeat_count])==' ')
				 {
				 repeat_str[repeat_count]='\0';
				 break;
				 }
			      ++repeat_count;
			      }
			   od_disp_str(repeat_str);
			   break;
			case 'X':
			   if(od_control.caller_ansi)
			      {
			      od_disp_str("ON");
			      }
			   else
			      {
			      od_disp_str("OFF");
			      }
			   break;
			case 'Y':
			   if(od_control.user_attribute&0x04)
			      {
			      od_disp_str("ON");
			      }
			   else
			      {
			      od_disp_str("OFF");
			      }
			   break;
			case 'Z':
			   if(od_control.user_attribute&0x02)
			      {
			      od_disp_str("ON");
			      }
			   else
			      {
			      od_disp_str("OFF");
			      }
			   break;
			case '0':
			   if(od_control.user_attribute&0x40)
			      {
			      od_disp_str("ON");
			      }
			   else
			      {
			      od_disp_str("OFF");
			      }
			   break;
			case '1':
			   if(od_control.user_attribute&0x80)
			      {
			      od_disp_str("ON");
			      }
			   else
			      {
			      od_disp_str("OFF");
			      }
			   break;
			case '2':
			   if(od_control.user_attrib2&0x01)
			      {
			      od_disp_str("ON");
			      }
			   else
			      {
			      od_disp_str("OFF");
			      }
			   break;
			case '3':
			   od_disp_str(od_control.ra_userhandle);
			   break;
			case '4':
			   od_disp_str(od_control.ra_firstcall);
			   break;
			case '5':
			   od_disp_str(od_control.ra_birthday);
			   break;
			case '6':
			   od_disp_str(od_control.ra_subdate);
			   break;
			case '7':
			   /* days until subscrption expiry */
			   break;
			case '8':
			   if(od_control.user_attrib2&0x02)
			      {
			      od_disp_str("ON");
			      }
			   else
			      {
			      od_disp_str("OFF");
			      }
			   break;
			case '9':
			   od_printf("%lu:%lu",od_control.user_uploads,od_control.user_downloads);
			   break;
			case ':':
			   od_printf("%lu:%lu",od_control.user_upk,od_control.user_downk);
			   break;
			case ';':
			   if(od_control.user_attrib2&0x04)
			      {
			      od_disp_str("ON");
			      }
			   else
			      {
			      od_disp_str("OFF");
			      }
			}
		     avt_seq=0;
		     break;

		  case 22:	       /* QBBS/RA ^K control codes */
		     echo_this_char = FALSE;
		     switch(*buf)
			{
			case 'A':
			   od_printf("%lu",od_control.system_calls);
			   break;
			case 'B':
			   od_disp_str(od_control.system_last_caller);
			   break;
			case 'C':
			   /* number of active messages */
			   break;
			case 'D':
			   /* system starting message number */
			   break;
			case 'E':
			   /* system ending message number */
			   break;
			case 'F':
			   /* number of times user has paged sysop */
			   break;
			case 'G':
			   /* day of the week (Monday, Tuesday, etc.) */
			   break;
			case 'H':
			   /* number of users in user file */
			   break;
			case 'I':
			   /* Time in 24 hour format */
			   break;
			case 'J':
			   /* today's date */
			   break;
			case 'K':
			   /* minutes connected this call */
			   break;
			case 'L':
			   /* Seconds connected (0) */
			   break;
			case 'M':      /* minutes used today */
                           od_printf("%d",od_control.user_time_used);
			   break;
			case 'N':      /* seconds used today (always 0) */
                           od_disp_str("00");
			   break;
			case 'O':
			   /* Minutes remaining today */
                           od_printf("%d",od_control.user_timelimit);
			   break;
			case 'P':
			   /* seconds remaining today (0) */
			   break;
			case 'Q':      /* Daily time limit */
			   od_printf("0",od_control.caller_timelimit);
			   break;
			case 'R':      /* current baud rate */
			   od_printf("0",od_control.baud);
			   break;
			case 'S':
			   /* day of the week (MON, TUE) */
			   break;
			case 'T':
			   /* Daily download limit (in K) */
			   break;
			case 'U':
			   /* Minutes until next system event */
			   break;
			case 'V':      /* 24 hour format time of next event */
			   phys_cputs(od_control.event_starttime);
			   break;
			case 'W':
			   /* line number (from command line) */
			   break;
			case 'X':      /* terminates the call */
			   od_exit(2,TRUE);
			   break;
			case 'Y':
			   /* Name of current msg area */
			   break;
			case 'Z':
			   /* name of current file area */
			   break;
			case '0':
			   /* # of messages in area */
			   break;
			case '1':
			   /* # of message area */
			   break;
			case '2':
			   /* # of file area */
			   break;
			}
		     avt_seq=0;
		  }
	    }
	 break;

      case 1:			       /* if 1st char (esc) of ANSI sequence.. */
	 switch(*buf)		    /* check next character */
	    {
	    case '[':                  /* if char is '[' */
	       seq_level=2;	       /* then this is a valid ANSI sequence */
	       param_str_len=0;        /* no ANSI parameters have been recieved */
	       num_params=0;
	       break;

	    default:		       /* if char is not '[', then this is not */
	       seq_level=0;	       /* a valid ANSI sequence */
	       phys_putch(27);	       /* display the characters */
	       phys_putch(*buf);
	    }
	 break;

      default:
	 if((*buf>='0' && *buf<='9') || *buf=='?')
	    {
	    if(param_str_len<3)
	       {
	       param_str[param_str_len]=*buf;
	       param_str[++param_str_len]='\0';
	       }
	    else
	       {
	       seq_level=0;
	       }
	    }

	 else if(*buf==';')
	    {
	    if(num_params<10)
	       {
	       if(param_str_len!=0)
		  {
		  if(strcmp(param_str, "?9")==0)
		     {
		     param[num_params]=-2;
		     }
		  else
		     {
		     param[num_params]=atoi(param_str);
		     }
		  param_str[0]='\0';
		  param_str_len=0;
		  ++num_params;
		  }
	       else
		  {
		  param[num_params++]=-1;
		  }
	       }
	    else
	       {
	       seq_level=0;
	       }
	    }

	 else
	    {
	    seq_level=0;

	    if(param_str_len!=0 && num_params<10)
	       {
	       if(strcmp(param_str,"?9")==0)
		  {
		  param[num_params]=-2;
		  }
	       else
		  {
		  param[num_params]=atoi(param_str);
		  }
	       param_str[0]='\0';
	       param_str_len=0;
	       ++num_params;
	       }

	    phys_gettextinfo(&info);

	    switch(*buf)
	       {
	       case 'A':
		  if(num_params==0) param[0]=1;
		  if((temp_int=info.cury-param[0])<1) temp_int=1;
		  if(temp_int>25) temp_int=25;
		  phys_gotoxy(info.curx,temp_int);
		  break;

	       case 'B':
		  if(num_params==0) param[0]=1;
		  if((temp_int=info.cury+param[0])>25) temp_int=25;
		  if(temp_int<1) temp_int=1;
		  phys_gotoxy(info.curx,temp_int);
		  break;

	       case 'C':
		  if(num_params==0) param[0]=1;
		  if((temp_int=info.curx+param[0])>80) temp_int=80;
		  if(temp_int<1) temp_int=1;
		  phys_gotoxy(temp_int,info.cury);
		  break;

	       case 'D':
		  if(num_params==0) param[0]=1;
		  if((temp_int=info.curx-param[0])<1) temp_int=1;
		  if(temp_int>80) temp_int=80;
		  phys_gotoxy(temp_int,info.cury);
		  break;

	       case 'H':
	       case 'f':
		  if(num_params>=2)
		     {
		     if(param[0]==-1)
			{
			phys_gotoxy(param[1],1);
			}
		     else
			{
			phys_gotoxy(param[1],param[0]);
			}
		     }
		  else if(num_params==1)
		     {
		     if(param[0]<=0)
			{
			phys_gotoxy(1,info.cury);
			}
		     else
			{
			phys_gotoxy(1,param[0]);
			}
		     }
		  else /* if(num_params==0) */
		     {
		     phys_gotoxy(1,1);
		     }
		  break;

	       case 'J':
		  if(num_params>=1 && param[0]==2)
		     {
		     /* Clear entire screen */
		     phys_clrscr();
		     }
		  else if(num_params == 0 || param[0]==0)
		     {
		     /* Not supported - Clears from cursor to end of screen */
		     }
		  else if(num_params>=1 && param[0]==1)
		     {
		     /* Not supported - Clears from beginning of screen to */
		     /* cursor */
		     }
		  break;

	       case 'K':
		  if(num_params == 0 || param[0] == 0)
		     {
		     /* Clear to end of line */
		     phys_clreol();
		     }
		  else if(num_params >= 1 && param[0] == 1)
		     {
		     /* Not supported - should clear to beginning of line */
		     }
		  else if(num_params >= 1 && param[0] == 2)
		     {
		     /* Not supported - should clear entire line */
		     }
		  break;

	       case 'm':
		  for(temp_int=0;temp_int<num_params;++temp_int)
		     {
		     if(param[temp_int]==0)
			{
			phys_setattrib(od_control.od_cur_attrib=info.attribute=0x07);
			}
		     else if(param[temp_int]==1)
			{
			phys_setattrib(od_control.od_cur_attrib=info.attribute=info.attribute|0x08);
			}
		     else if(param[temp_int]==2)
			{
			phys_setattrib(od_control.od_cur_attrib=info.attribute=info.attribute&(~0x08));
			}
		     else if(param[temp_int]==4)
			{
			phys_setattrib(od_control.od_cur_attrib=info.attribute=(info.attribute&0xf8)|(1));
			}
		     else if(param[temp_int]==5)
			{
			phys_setattrib(od_control.od_cur_attrib=info.attribute=info.attribute|0x80);
			}
		     else if(param[temp_int]==7)
			{
			phys_setattrib(od_control.od_cur_attrib=info.attribute=(info.attribute<<4) | (info.attribute>>4));
			}
		     else if(param[temp_int]==8)
			{
			phys_setattrib(od_control.od_cur_attrib=info.attribute=(info.attribute&0xf0) | ((info.attribute>>4)&0x07));
			}
		     else if(param[temp_int]>=30 && param[temp_int]<=37)
			{
			phys_setattrib(od_control.od_cur_attrib=info.attribute=(info.attribute&0xf8) + ansi_color[(param[temp_int]-30)]);
			}
		     else if(param[temp_int]>=40 && param[temp_int]<=47)
			{
			phys_setattrib(od_control.od_cur_attrib=info.attribute=(info.attribute&0x8f) + (ansi_color[param[temp_int]-40]<<4));
			}
		     }
		  break;

	       case 's':
		  saved_x=info.curx;
		  saved_y=info.cury;
		  break;

	       case 'u':
		  phys_gotoxy(saved_x,saved_y);
		  break;

	       case '@':
		  /* Not supported - inserts spaces at cursor */
		  break;

	       case 'P':
		  /* Not supported - deletes characters at cursor */
		  break;

	       case 'L':
		  /* Not supported - inserts lines at cursor */
		  break;

	       case 'M':
		  /* Not supported - deletes lines at cursor */
		  break;

	       case 'r':
		  /* Not supported - sets scrolling zone - 1st param is      */
		  /* top row, 2nd param is bottom row. Cursor may go outside */
		  /* zone, but no scrolling occurs there.		     */
		  /* Also resets cursor to row 1, column 1.		     */
		  /* If only one param, bottom row is bottom of screen.      */
		  break;

	       case 'h':
		  if(num_params>=1 && param[0]==4)
		     {
		     /* Not suppored - turn insert mode on */
		     }
		  else if(num_params>=1 && param[0]==-2)
		     {
		     /* home cursor */
		     phys_gotoxy(1,1);
		     }
		  break;

	       case 'l':
		  if(num_params>=1 && param[0]==4)
		     {
		     /* Not suppored - turn insert mode off */
		     }
		  break;

	       case 'E':
		  /* Not supported - repeat CRLF specified # of times */
		  break;

	       case 'F':
		  /* Not supported - repeat reverse CRLF specified # of times */
		  /* Also not suppored ESC M - reverse linefeed, ESC D - LF,  */
		  /* ESC E - CRLF					      */
		  break;
	       }
	    }
      }

   if(echo_this_char)
      {
      od_disp(buf, 1, FALSE);
      }

   ++buf;
   }
}


void fill_area(char x1,char y1,char x2,char y2,register char character)
   {
   register char counter;
   register char last;
   static char temp_str[81];
   static struct phys_text_info info;

   phys_gettextinfo(&info);

   last=x2-x1;

   for(counter=0;counter<=last;++counter)
      {
      temp_str[counter]=character;
      }
   temp_str[counter]=0;

   phys_setscroll(FALSE);

   for(counter=y1;counter<=y2;++counter)
      {
      phys_gotoxy(x1,counter);
      phys_cputs(temp_str);
      }

   phys_gotoxy(info.curx,info.cury);

   phys_setscroll(TRUE);
   }
