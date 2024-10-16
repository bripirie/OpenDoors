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
 *     Filename : ODCFILE.C
 *  Description : Code composing the optional configuration file sub-system.
 *      Version : 5.00
 */


#include "opendoor.h"                                  /* OpenDoors header files */
#include "odintern.h"

#include <stdio.h>                                    /* C standard header files */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <alloc.h>

#define OCDECL static



unsigned int _gt_val[3];                           /* Internal private variables */
char _gt_num_vals;




void option_config(void)
   {
   void (*custom_line_function)(char *keyword, char *options)=od_control.config_function;
   char *pointer;
   register unsigned int counter;
   unsigned char option;
   OCDECL FILE *config_file;
   OCDECL FILE *door_file=NULL;
   static char line[257];
   static char token[33];
   static char tempstr[256];
   static char workdir[80];
   OCDECL char workdir_set=FALSE;
   OCDECL time_t timer;
   OCDECL struct tm *tblock;
   OCDECL int page_start;
   OCDECL int page_end;
   OCDECL char page_set=FALSE;
   OCDECL char inactivity_set=FALSE;
   OCDECL int inactivity;
   OCDECL char *workstring;
   OCDECL char pagelen_set=FALSE;
   OCDECL char pagelen;
   OCDECL char *name_array[1];

   _is_callback = TRUE;

   timer=time(NULL);                                  /* Get current day of week */
   tblock=localtime(&timer);

   /* Use default configuration file filename if none has been specified. */
   if(od_control.od_config_filename == NULL)
      {
      od_control.od_config_filename = "door.cfg";
      }

   if((config_file=fopen(od_control.od_config_filename,"rt"))==NULL)
      {
      if(strchr(od_control.od_config_filename,'\\')!=NULL || strchr(od_control.od_config_filename,':')!=NULL)
         {
         counter=strlen(od_control.od_config_filename);
         pointer=(char *)od_control.od_config_filename+(counter-1);
         while(counter>0)
            {
            if(*pointer=='\\' || *pointer==':')
               {
               strcpy(line,(char *)pointer+1);
               config_file=fopen(line,"rt");
               break;
               }

            --pointer;
            --counter;
            }
         }
      else
         {
         strcpy(line,od_control.od_config_filename);
         }
      }

/*
   if(config_file==NULL)
      {
      strcpy(tempstr,(char *)_argv[0]);
      counter=strlen(tempstr);
      pointer=(char *)tempstr+(counter-1);
      while(counter>0)
         {
         if(*pointer=='\\' || *pointer==':')
            {
            tempstr[counter]='\0';
            strcat(tempstr,line);
            config_file=fopen(tempstr,"rt");
            break;
            }

         --pointer;
         --counter;
         }
      }
*/

   if(config_file!=NULL)                          /* If able to open config file */
      {
      for(counter=0;counter<TEXT_SIZE;++counter)
         strupr(od_config_text[counter]);
      for(counter=0;counter<LINES_SIZE;++counter)
         strupr(od_config_lines[counter]);

      for(;;)
         {
         if(fgets(line,257,config_file)==NULL) break;       /* Get the next line */

                              /* Ignore all of line after comments or CR/LF char */
         pointer=(char *)line;
         while(*pointer)
            {
            if(*pointer=='\n' || *pointer=='\r' || *pointer==';')
               {
               *pointer='\0';
               break;
               }
            ++pointer;
            }

                                  /* Search for beginning of first token on line */
         pointer=(char *)line;
         while(*pointer && (*pointer == ' ' || *pointer == '\t'))
            {
            ++pointer;
            }
         if(!*pointer) continue;
                                                    /* Get first token from line */
         counter=0;
         while(*pointer && !(*pointer == ' ' || *pointer == '\t'))
            {
            if(counter<32) token[counter++]=*pointer;
            ++pointer;
            }
         if(counter<=32)
            token[counter]='\0';
         else
            token[32]='\0';
         strupr(token);

                            /* Find beginning of configuration option parameters */
         while(*pointer && (*pointer == ' ' || *pointer == '\t')) ++pointer;

                                     /* Trim trailing spaces from setting string */
         for(counter=strlen(pointer)-1;counter>0;--counter)
            {
            if(pointer[counter] == ' ' || pointer[counter] == '\t')
               {
               pointer[counter]='\0';
               }
            else
               {
               break;
               }
            }


         for(counter=0;counter<TEXT_SIZE;++counter)
            {
            if(strcmp(token,od_config_text[counter])==0)
               {
               switch(counter)
                  {
                  case 0:
                     _node_number=get_config_unsigned(pointer);
                     break;

                  case 1:
                     strcpy(od_control.info_path,pointer);
                     break;

                  case 2:
                     if(pointer[strlen(pointer)-1]=='\\' && pointer[strlen(pointer)-2]!=':' && strlen(pointer)>1)
                         {
                         pointer[strlen(pointer)-1]='\0';
                         }

                     _final_dir=(char *)malloc(256);
                     if(_final_dir!=NULL)
                        {
                        strcpy(_final_dir,"X:\\");
                        _final_dir[0]='A'+_getdrv();
                        _getcd(0,(char *)_final_dir+3);
                        }

                     strcpy(workdir,pointer);
                     workdir_set=TRUE;
                     break;

                  case 3:
                     strcpy(od_control.od_logfile_name,pointer);
                     break;

                  case 4:
                     od_control.od_logfile_disable=TRUE;
                     break;

                  case 5:
                  case 6:
                  case 7:
                  case 8:
                  case 9:
                  case 10:
                  case 11:
                      if((counter-5) == tblock->tm_wday)
                         {
                         get_next_time((char **)&pointer);
                         page_start=_gt_val[0]*60+_gt_val[1];
                         get_next_time((char **)&pointer);
                         page_end=_gt_val[0]*60+_gt_val[1];
                         page_set=TRUE;
                         }
                     break;

                  case 12:
                     od_control.od_maxtime=get_config_unsigned(pointer);
                     break;

                  case 13:
                     sysop_name_set=TRUE;
                     strncpy((char *)&forced_sysop_name,pointer,39);
                     forced_sysop_name[39]='\0';
                     break;

                  case 14:
                     system_name_set=TRUE;
                     strncpy((char *)&forced_system_name,pointer,39);
                     forced_system_name[39]='\0';
                     break;

                  case 15:
                     od_control.od_swapping_disable=TRUE;
                     break;

                  case 16:
                     strncpy(od_control.od_swapping_path,pointer,79);
                     od_control.od_swapping_path[79]='\0';
                     break;

                  case 17:
                     od_control.od_swapping_noems=TRUE;
                     break;

                  case 18:
                     _forced_bps=get_config_ulong(pointer);
                     break;

                  case 19:
                     _forced_port=get_config_unsigned(pointer);
                     break;

                  case 20:
                     if(door_file==NULL && !od_control.od_force_local)
                        {
                        name_array[0]=(char *)pointer;
                        if(search_for_infofile(name_array,1,tempstr,NULL)!=-1)
                           if((door_file=fopen(tempstr,"rt"))!=NULL)
                              {
                              od_control.od_info_type=CUSTOM;
                              od_control.user_attribute=0x06;
                              od_control.user_screen_length=23;
                              od_control.user_ansi=TRUE;
                              od_control.user_rip=FALSE;
                              od_control.user_avatar=FALSE;
                              od_control.od_page_pausing=TRUE;
                              od_control.od_page_len=15;
                              od_control.user_timelimit=0;
                              strcpy(od_control.user_name,"Unknown User");
                              strcpy(od_control.user_location,"Unknown Location");
                              od_control.user_security=1;
                              }
                        }
                     break;

                  case 21:
                     if(door_file!=NULL)
                        {
                        if(fgets(tempstr,255,door_file)!=NULL)
                           {
                           if(tempstr[strlen(tempstr)-1]=='\n')
                              {
                              tempstr[strlen(tempstr)-1]='\0';
                              }
                           else
                              {
                              int ch;
                              do
                                 {
                                 ch = fgetc(door_file);
                                 } while(ch != '\n' && ch != EOF);
                              }

                           strupr(pointer);

                           for(option=0;option<LINES_SIZE;++option)
                              {
                              if(strcmp(pointer,od_config_lines[option])==0)
                                 {
                                 switch(option)
                                    {
                                    case 1:
                                       od_control.port=get_config_unsigned(tempstr)-1;
                                       break;

                                    case 2:
                                       od_control.port=get_config_unsigned(tempstr);
                                       break;

                                    case 3:
                                       od_control.baud=get_config_unsigned(tempstr);
                                       break;

                                    case 4:
                                       if(is_true(tempstr)) od_control.baud=0;
                                       break;

                                    case 5:
                                    case 6:
                                       toname(tempstr);
                                       strncpy(od_control.user_name,tempstr,34);
                                       od_control.user_name[34]='\0';
                                       break;

                                    case 7:
                                       strcat(od_control.user_name," ");
                                       toname(tempstr);
                                       strncat(od_control.user_name,tempstr,35-strlen(od_control.user_name));
                                       od_control.user_name[35]='\0';
                                       break;

                                    case 8:
                                       toname(tempstr);
                                       strncpy(od_control.user_handle,tempstr,35);
                                       od_control.user_handle[35]='\0';
                                       break;

                                    case 9:
                                       workstring=(char *)tempstr;
                                       get_next_time((char **)&workstring);
                                       od_control.user_timelimit+=(_gt_val[0]*60);
                                       break;

                                    case 10:
                                       workstring=(char *)tempstr;
                                       get_next_time((char **)&workstring);
                                       if(_gt_num_vals<=1)
                                          {
                                          od_control.user_timelimit+=_gt_val[0];
                                          }
                                       else
                                          {
                                          od_control.user_timelimit+=_gt_val[1]+(_gt_val[0]*60);
                                          }
                                       break;

                                    case 11:
                                       workstring=(char *)tempstr;
                                       get_next_time((char **)&workstring);
                                       if(_gt_num_vals<=1)
                                          {
                                          od_control.user_timelimit+=_gt_val[0]/60;
                                          }
                                       else if(_gt_num_vals==2)
                                          {
                                          od_control.user_timelimit+=(_gt_val[1]/60)+_gt_val[0];
                                          }
                                       else
                                          {
                                          od_control.user_timelimit+=(_gt_val[2]/60)+_gt_val[1]+(_gt_val[0]*60);
                                          }
                                       break;

                                    case 12:
                                       od_control.caller_ansi=is_true(tempstr);
                                       break;

                                    case 13:
                                       od_control.od_avatar=is_true(tempstr);
                                       break;

                                    case 14:
                                       od_control.od_page_pausing=is_true(tempstr);
                                       break;

                                    case 15:
                                       od_control.user_screen_length=get_config_unsigned(tempstr);
                                       break;

                                    case 16:
                                       if(is_true(tempstr))
                                           {
                                           od_control.user_attribute|=0x02;
                                           }
                                       else
                                           {
                                           od_control.user_attribute&=~0x02;
                                           }
                                       break;

                                    case 17:
                                       od_control.user_security=get_config_unsigned(tempstr);
                                       break;

                                    case 18:
                                       toname(tempstr);
                                       strncpy(od_control.user_location,tempstr,25);
                                       od_control.user_location[25]='\0';
                                       break;

                                    case 19:
                                       _node_number=get_config_unsigned(tempstr);
                                       break;

                                    case 20:
                                    case 21:
                                       toname(tempstr);
                                       strncpy(od_control.sysop_name,tempstr,38);
                                       od_control.sysop_name[38]='\0';
                                       break;

                                    case 22:
                                       strcat(od_control.sysop_name," ");
                                       toname(tempstr);
                                       strncat(od_control.sysop_name,tempstr,39-strlen(od_control.system_name));
                                       od_control.sysop_name[39]='\0';
                                       break;

                                    case 23:
                                       strncpy(od_control.system_name,tempstr,39);
                                       od_control.system_name[39]='\0';
                                       break;

                                    case 24:
                                       od_control.user_rip=is_true(tempstr);
                                    }
                                 }
                              }
                           }
                        }
                     break;

                  case 22:
                     inactivity_set=TRUE;
                     inactivity=get_config_unsigned(pointer);
                     if(inactivity<0) inactivity=0;
                     break;

                  case 23:
                     pagelen=get_config_unsigned(pointer);
                     pagelen_set=TRUE;
                     break;

                  case 24:
                     od_control.od_chat_colour2=od_colour_config(pointer);
                     break;

                  case 25:
                     od_control.od_chat_colour1=od_colour_config(pointer);
                     break;

                  case 26:
                     od_control.od_list_title_col=od_colour_config(pointer);
                     break;

                  case 27:
                     od_control.od_list_name_col=od_colour_config(pointer);
                     break;

                  case 28:
                     od_control.od_list_size_col=od_colour_config(pointer);
                     break;

                  case 29:
                     od_control.od_list_comment_col=od_colour_config(pointer);
                     break;

                  case 30:
                     od_control.od_list_offline_col=od_colour_config(pointer);
                     break;

                  case 31:
                     strncpy(_desired_personality,pointer,32);
                     _desired_personality[32]='\0';

                  case 32:
                     /* "NoFossil" */
                     od_control.od_no_fossil = TRUE;
                     break;

                  case 33:
                     /* "PortAddress" */
                     od_control.od_com_address=get_config_hex(pointer);
                     break;

                  case 34:
                     /* "PortIRQ" */
                     od_control.od_com_irq=(char)get_config_unsigned(pointer);
                     break;

                  case 35:
                     /* "ReceiveBuffer" */
                     od_control.od_com_rx_buf=get_config_unsigned(pointer);
                     break;

                  case 36:
                     /* "TransmitBuffer" */
                     od_control.od_com_tx_buf=get_config_unsigned(pointer);
                     break;

                  case 37:
                     /* "PagePromptColour" */
                     od_control.od_continue_col=od_colour_config(pointer);
                     break;

                  case 38:
                     /* "LocalMode" */
                     od_control.od_force_local = TRUE;
                     break;

                  case 39:
                     /* "PopupMenuTitleColour" */
                     od_control.od_menu_title_col=od_colour_config(pointer);
                     break;

                  case 40:
                     /* "PopupMenuBorderColour" */
                     od_control.od_menu_border_col=od_colour_config(pointer);
                     break;

                  case 41:
                     /* "PopupMenuTextColour" */
                     od_control.od_menu_text_col=od_colour_config(pointer);
                     break;

                  case 42:
                     /* "PopupMenuKeyColour" */
                     od_control.od_menu_key_col=od_colour_config(pointer);
                     break;

                  case 43:
                     /* "PopupMenuHighlightColour" */
                     od_control.od_menu_highlight_col=od_colour_config(pointer);
                     break;

                  case 44:
                     /* "PopupMenuHighKeyColour" */
                     od_control.od_menu_highkey_col=od_colour_config(pointer);
                     break;

                  case 45:
                     /* "NoFIFO" */
                     od_control.od_com_no_fifo = TRUE;
                     break;

                  case 46:
                     /* "FIFOTriggerSize" */
                     od_control.od_com_fifo_trigger=get_config_unsigned(pointer);
                     break;
                  }
               }
            }

                           /* Check if command is a programmer customized option */
         if(counter>=TEXT_SIZE && custom_line_function!=NULL)
            {
            (*custom_line_function)((char *)&token,pointer);
            }
         }

      fclose(config_file);                       /* Close the configuration file */
      }
   else
      {
      od_control.od_error = ERR_FILEOPEN;
      }

   if(door_file!=NULL) fclose(door_file);         /* Close custom door info file */

   _is_callback = FALSE;

   _called_from_config=TRUE;
   od_init();                                  /* Call normal od_init() function */
   _called_from_config=FALSE;


   if(page_set)
      {
      od_control.od_pagestartmin=page_start;
      od_control.od_pageendmin=page_end;
      }

   if(inactivity_set && inactivity != 0)
      {
      od_control.od_inactivity=inactivity;
      }

   if(sysop_name_set)
      {
      strcpy((char *)&od_control.sysop_name,(char *)&forced_sysop_name);
      }

   if(system_name_set)
      {
      strcpy((char *)&od_control.system_name,(char *)&forced_system_name);
      }

   if(pagelen_set)
      {
      od_control.od_page_len=pagelen;
      }

   if(workdir_set)
      {
      _chdir(workdir);
      }
   }


                                           /* Get first unsigned int from string */
unsigned int get_config_unsigned(char *string)
   {                                /* Skip any initial non-numerical characters */
   while(*string && (*string<'0' || *string>'9')) ++string;

   return(atoi(string));                               /* Return value of number */
   }

                                           /* Get first unsigned int from string */
unsigned long get_config_ulong(char *string)
   {                                /* Skip any initial non-numerical characters */
   while(*string && (*string<'0' || *string>'9')) ++string;

   return(atol(string));                               /* Return value of number */
   }

unsigned int get_config_hex(char *string)
   {
   unsigned int to_return;

   while(*string && (*string<'0' || *string>'9')) ++string;

   sscanf(string, "%x", &to_return);

   return(to_return);
   }



void get_next_time(char **pointer_to_pointer)
   {
   register char *pointer=(char *)(*pointer_to_pointer);

   _gt_num_vals=0;
   _gt_val[0]=0;
   _gt_val[1]=0;
   _gt_val[2]=0;


   while(*pointer && (*pointer == ' ' || *pointer == '\t')) ++pointer;


   while(*pointer && _gt_num_vals<3)
      {
      if(*pointer<'0' || *pointer>'9') break;
      _gt_val[_gt_num_vals++]=atoi(pointer);
      while(*pointer && *pointer>='0' && *pointer<='9') ++pointer;
      if(*pointer==':' || *pointer=='.' || *pointer==',' || *pointer==';') ++pointer;
      }
      
   *pointer_to_pointer=(char *)pointer;
   }


char is_true(char *string)
   {
   while(*string && (*string == ' ' || *string == '\t')) ++string;

   switch(*string)
      {
      case '1':
      case 't':
      case 'T':
      case 'y':
      case 'Y':
      case 'g':
      case 'G':
         return(TRUE);
      }

   return(FALSE);
   }
