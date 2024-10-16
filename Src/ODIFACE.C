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
 *     Filename : ODIFACE.C
 *  Description : Contains the code to interface with various BBS packages,
 *                to initialize OpenDoors at the beginning of program
 *                execution, and to clean up at program exit.
 *      Version : 5.00
 */




#include<stdio.h>                      /* Standard header files */
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<ctype.h>
#include<alloc.h>
#include<time.h>

#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"

static void _readexitinfo(void);
static void _initcs(void);

unsigned int _node_number=65535U;       /* Global variables */

int is_cosys;
int is_sysop;
char *storestr[25];
char _preset=TRUE;
char _exitreason=0;

unsigned long _forced_bps=1;
char _forced_port=-1;
unsigned long _file_bps;

char ipath[120];                       /* path & name of door information file */
char exitpath[120];                    /* path to exitinfo.bbs file */

int _initial_elapsed;                  /* time remaining at door startup */
char *_final_dir=NULL;
char doorsys_lock=0;
time_t door_start;                     /* Time at which door's execution began */
int start_remaining;                   /* Original time remaining value */

char sysop_name_set=FALSE;
char forced_sysop_name[40];
char system_name_set=FALSE;
char forced_system_name[40];

unsigned filesizea;                    /* File size variables */
unsigned filesizeb;

char _called_from_config=FALSE;



char *_config_text[TEXT_SIZE]={
                          "Node",
                          "BBSDir",
                          "DoorDir",
                          "LogFileName",
                          "DisableLogging",
                          "SundayPagingHours",
                          "MondayPagingHours",
                          "TuesdayPagingHours",
                          "WednesdayPagingHours",
                          "ThursdayPagingHours",
                          "FridayPagingHours",
                          "SaturdayPagingHours",
                          "MaximumDoorTime",
                          "SysopName",
                          "SystemName",
                          "SwappingDisable",
                          "SwappingDir",
                          "SwappingNoEMS",
                          "LockedBPS",
                          "SerialPort",
                          "CustomFileName",
                          "CustomFileLine",
                          "InactivityTimeout",
                          "PageDuration",
                          "ChatUserColour",
                          "ChatSysopColour",
                          "FileListTitleColour",
                          "FileListNameColour",
                          "FileListSizeColour",
                          "FileListDescriptionColour",
                          "FileListOfflineColour",
                          "Personality",
                          "NoFossil",
                          "PortAddress",
                          "PortIRQ",
                          "ReceiveBuffer",
                          "TransmitBuffer",
                          "PagePromptColour",
                          "LocalMode",
                          "PopupMenuTitleColour",
                          "PopupMenuBorderColour",
                          "PopupMenuTextColour",
                          "PopupMenuKeyColour",
                          "PopupMenuHighlightColour",
                          "PopupMenuHighKeyColour",
                          "NoFIFO",
                          "FIFOTriggerSize"};

char *_config_lines[LINES_SIZE]=
                          {"Ignore",
                           "ComPort",
                           "FossilPort",
                           "ModemBPS",
                           "LocalMode",
                           "UserName",
                           "UserFirstName",
                           "UserLastName",
                           "Alias",
                           "HoursLeft",
                           "MinutesLeft",
                           "SecondsLeft",
                           "ANSI",
                           "AVATAR",
                           "PagePausing",
                           "ScreenLength",
                           "ScreenClearing",
                           "Security",
                           "City",
                           "Node",
                           "SysopName",
                           "SysopFirstName",
                           "SysopLastName",
                           "SystemName",
                           "RIP"};

/* Default logfile messages */
char *_log_messages[14]={"Carrier lost, exiting door",
                         "System operator terminating call, exiting door",
                         "User's time limit expired, exiting door",
                         "User keyboard inactivity time limit exceeded, exiting door",
                         "System operator returning user to BBS, exiting door",
                         "Exiting door with errorlevel %d",
                         "Invoking operating system shell",
                         "Returning from operating system shell",
                         "User paging system operator",
                         "Entering sysop chat mode",
                         "Terminating sysop chat mode",
                         "%s entering door",
                         "Reason for chat: %s",
                         "Exiting door"};

char *_config_colours[12]={
                          "BLACK",
                          "BLUE",
                          "GREEN",
                          "CYAN",
                          "RED",
                          "MAGENTA",
                          "YELLOW",
                          "WHITE",
                          "BROWN",
                          "GREY",
                          "BRIGHT",
                          "FLASHING"};

									   
struct _ra2exitinfo *ra2exitinfo=NULL;
struct _exitinfo *exitinfo=NULL;
struct _ext_exitinfo *ext_exitinfo=NULL;
struct _pcbsys *pcbsys=NULL;
struct _userssyshdr *userssyshdr=NULL;
struct _userssysrec *userssysrec=NULL;



                                       /* function to convert C string format */
                                       /* to pascal string format */
char *c2pasc(char *pasc_str,unsigned char max_pasc_len,char *c_str)
   {
   register unsigned char sizeof_c=strlen(c_str);

   memcpy((char *)pasc_str+1,c_str,*pasc_str=(sizeof_c<max_pasc_len) ? sizeof_c : max_pasc_len);
   return(pasc_str);
   }

                                       /* function to convert pascal string */
                                       /* format to c string format */
char *pasc2c(char *c_str,char *pasc_str,unsigned char max_len)
   {
   if(*(unsigned char *)pasc_str>=0 && *(unsigned char *)pasc_str<=max_len)
      {
      memcpy(c_str,(char *)pasc_str+1,*pasc_str);
      c_str[*pasc_str]='\0';
      }
   else
      {
      c_str[0]='\0';
      }
   return(c_str);
   }





                                       /* function to create filename from */
                                       /* path and name */
char *makepath(char *path,char *filename)
   {
   static char fullname[80];

   if(strlen(path)==0)
      {
      strcpy(fullname,filename);
      return(fullname);
      }

   strcpy(fullname,path);              /* store directory */
                                       /* add trailing backslash if none */
	if(fullname[strlen(fullname)-1]!='\\')
		{
      strcat(fullname,"\\");
      }


   strcat(fullname,filename);          /* add filename to end */
   return(fullname);
   }


void od_init(void)                     /* OpenDoors initializtion code */
   {
   register char counter;              /* counter for looping through file */
   FILE *fp;                           /* filepointer for reading door info file */
   static char tempstr[256];           /* string used to store read data */
   char *pointer;
   char found=-1;
   static char *doorfile_names[6]={NULL,
                                   (char *)"DORINFO1.DEF",
                                   (char *)"CHAIN.TXT",
                                   (char *)"SFDOORS.DAT",
                                   (char *)"DOOR.SYS",
                                   (char *)"CALLINFO.BBS"};

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_init()");

   if(_is_callback) return;

   if(!_called_from_config)            /* If this is od_init() call by user */
      {
      if(inited) return;                  /* Don't initialize a second time */

      inited = TRUE;

      for(counter=0;counter<12;++counter)
         {
         if(!*od_control.od_colour_names[counter])
            {
            strcpy(od_control.od_colour_names[counter],_config_colours[counter]);
            }
         }
      for(counter=0;counter<LINES_SIZE;++counter)
         {
         if(!*od_control.od_cfg_lines[counter])
            {
            strcpy(od_control.od_cfg_lines[counter],_config_lines[counter]);
            }
         }
      for(counter=0;counter<TEXT_SIZE;++counter)
         {
         if(!*od_control.od_cfg_text[counter])
            {
            strcpy(od_control.od_cfg_text[counter],_config_text[counter]);
            }
         }

      if(od_control.od_mps!=NULL)      /* Enable MPS if necessary */
         {
         (*od_control.od_mps)();
         }


      if(od_control.baud != 0)
         {
         _forced_bps = od_control.baud;
         }
      if(od_control.port != 0)
         {
         _forced_port = od_control.port;
         }

      if(od_control.config_file!=NULL) /* If configuration file support */
         {
         (*od_control.config_file)();  /* Use configuration initialization */
                                       /* function instead */

         return;
         }
      }

   __kx(copyright,copy_checksum);
   __kxx();                   

   _mt_init();

   time(&door_start);                  /* store time of door startup */

   for(counter=0;counter<25;++counter)
      {
      if((storestr[counter]=(char *)malloc(81))==NULL)
         {
malloc_error:
         puts("Critical Error [OpenDoors]: Not enough memory.\n");
         exit(od_control.od_errorlevel[1]);
         }
      }
 
   if((pointer=getenv("TASK"))!=NULL)
      {
      od_control.od_node=atoi(pointer);
      }
   else if(_node_number!=65535U)
      {
      od_control.od_node=_node_number;
      }
   else if(od_control.od_node == 0)
      {
      od_control.od_node = 1;
      }

   od_control.od_box_chars[0]=218;
   od_control.od_box_chars[1]=196;
   od_control.od_box_chars[2]=191;
   od_control.od_box_chars[3]=179;
   od_control.od_box_chars[4]=192;
   od_control.od_box_chars[5]=217;

   if(od_control.od_info_type!=CUSTOM)
      {
      od_control.caller_ansi=FALSE;    /* ANSI setting not known yet */
      od_control.od_avatar=FALSE;      /* AVATAR mode not known yet */
      od_control.user_rip=FALSE;       /* RIP setting not known yet */
      od_control.user_attribute=0x06;  /* Enable screen clearing */
      od_control.user_screen_length=23;/* Default screen length */
      od_control.od_page_pausing=TRUE;
      od_control.od_page_len=15;       /* Default sysop page length */
      }
   else
      {
      if(od_control.user_timelimit==0) od_control.user_timelimit=30;
      if(od_control.port==-1) od_control.baud=0L;
      }

   /* Setup inbound local/remote buffer */
   if(od_control.od_in_buf_size == 0)
      {
      _in_buf_size = BS;
      }
   else
      {
      _in_buf_size = od_control.od_in_buf_size;
      }
   input_buffer = malloc(_in_buf_size);
   input_remote = malloc(_in_buf_size);
   if(input_buffer == NULL || input_remote == NULL)
      {
      goto malloc_error;
      }

   od_control.caller_wantchat=FALSE;   /* turn off wantchat indicator */
   od_control.od_user_keyboard_on=TRUE;/* enable user's keyboard */
   od_control.ra_sysop_next=FALSE;     /* turn off sysop next indicator */

   od_control.od_list_stop=TRUE;
   od_control.od_list_pause=TRUE;
   od_control.user_reasonforchat[0]='\0';


   if(od_control.od_disable&DIS_INFOFILE)
      {
      od_control.od_info_type=NO_DOOR_FILE;
      }

   else if(od_control.od_force_local)  /* If programmer is forcing local mode */
      {
force_local:
      /* No door information file is being used */
      od_control.od_info_type = NO_DOOR_FILE;
      od_control.baud = 0L;            /* Operate in local mode */
      od_control.caller_ansi=TRUE;     /* Enable ANSI mode */
      od_control.caller_timelimit=60;  /* 60 minutes of time is available */

      if(system_name_set)
         {
         strcpy(od_control.user_location, forced_system_name);
         }
      else if(od_control.system_name[0] != '\0')
         {
         strcpy(od_control.user_location, od_control.system_name);
         }
      else
         {
         strcpy(od_control.user_location, "Unknown Location");
         }
      }

   else if(od_control.od_info_type != CUSTOM)
      {                                /* Generate the DORINFO?.DEF filename */
       if(od_control.od_node>35)
          {
          doorfile_names[0]=(char *)"DORINFO1.DEF";
          }
       else if(od_control.od_node>9)
          {
          sprintf(tempstr,"DORINFO%c.DEF",od_control.od_node+55);
          doorfile_names[0]=(char *)tempstr;
          }
       else
          {
          sprintf(tempstr,"DORINFO%d.DEF",od_control.od_node);
          doorfile_names[0]=(char *)tempstr;
          }

       found = -1;

       if(_accessmode(od_control.info_path, 4) != -1)
          {
          /* Check for a DORINFOx.DEF filename. */
          if(_strmatchtail(od_control.info_path, ".DEF") &&
             strlen(od_control.info_path) >= strlen(doorfile_names[1]) &&
             strnicmp((char *)&od_control.info_path +
                (strlen(od_control.info_path) - 12), "DORINFO", 7) == 0)
             {
             found = 0;
             strcpy(ipath, od_control.info_path);
             }
          else
             {
             /* Check filenames other than DORINFOx.DEF */
             for(counter = 2; counter < 6; ++counter)
                {
                if(_strmatchtail(od_control.info_path, doorfile_names[counter]))
                   {
                   strcpy(ipath, od_control.info_path);
                   found = counter;
                   break;
                   }
                }
             }
          }

                                          /* Search for a door information file */
       if(found == -1)
          {
          found=search_for_infofile((char **)&doorfile_names,6,
             (char *)&ipath,(char *)&exitpath);
          }

                                          /* open DORINFO?.DEF */
       if(found==0 || found==1)
          {
          if((fp=fopen(ipath,"r"))==NULL) goto no_go;
                                          /* set door type to DORINFO */
          od_control.od_info_type=DORINFO1;

          if(fgets(tempstr,255,fp)==NULL) /* if not able to read first line */
             {
             goto no_go;
             }

          if(tempstr[strlen(tempstr)-1]=='\n') tempstr[strlen(tempstr)-1]='\0';
          strncpy(od_control.system_name,tempstr,39);
                                          /* get sysop name from DORINFO1.DEF */
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          toname(tempstr);
          strncpy(od_control.sysop_name,tempstr,19);
                                          /* get sysop's last name */
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          toname(tempstr);
          if(strlen(tempstr))
             {
             strcat(od_control.sysop_name," ");
             strncat(od_control.sysop_name,tempstr,19);
             }
                                   /* get com port that modem is connected to */
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.port=tempstr[3]-'1';
                                          /* determine BPS rate of connection */
          if(fgets((char *)storestr[0],255,fp)==NULL) goto no_go;
          od_control.baud= (od_control.port == -1) ? 0 : atol((char *)storestr[0]);

          if(fgets((char *)storestr[1],80,fp)==NULL) goto no_go;

                                          /* get user's first name */
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          toname(tempstr);
          strncpy(od_control.user_name,tempstr,17);
                                          /* get user's last name */
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          toname(tempstr);
          if(strlen(tempstr))
             {
             strcat(od_control.user_name," ");
             strncat(od_control.user_name,tempstr,17);
             }
                                          /* get user's location */
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          toname(tempstr);
          strncpy(od_control.user_location,tempstr,25);
                                          /* get ANSI mode settings */
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(tempstr[0]=='0') od_control.caller_ansi=FALSE;
          else od_control.caller_ansi=TRUE;
                                          /* get user security level */
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_security=atoi(tempstr);
                                          /* get time left in door */
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.caller_timelimit=atoi(tempstr);
          fclose(fp);
          }
                                       /* Read CHAIN.TXT */
       else if(found==2)
          {
          if((fp=fopen(ipath,"r"))==NULL) goto no_go;

          od_control.od_info_type=CHAINTXT;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_num=atoi(tempstr);

          if(fgets((char *)&od_control.ra_userhandle,35,fp)==NULL) goto no_go;
          toname(od_control.ra_userhandle);

          if(fgets((char *)&od_control.user_name,35,fp)==NULL) goto no_go;
          toname(od_control.user_name);

          if(fgets((char *)&od_control.caller_callsign,12,fp)==NULL) goto no_go;
          toname(od_control.caller_callsign);

          if(fgets((char *)storestr[0],80,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.caller_sex=tempstr[0];

          if(fgets((char *)storestr[1],80,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          strncpy(od_control.user_lastdate,tempstr,8);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.ra_screenwidth=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_screen_length=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_security=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          is_sysop=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          is_cosys=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.caller_ansi=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;    /* non-zero if remote */

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.caller_timelimit=atoi(tempstr);
          od_control.caller_timelimit/=60;

          if(fgets((char *)storestr[3],80,fp)==NULL) goto no_go;

          if(fgets((char *)storestr[4],80,fp)==NULL) goto no_go;

          if(fgets((char *)storestr[5],80,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(strcmp(tempstr,"KB")==0)
             {
             od_control.baud=0;
             }
          else
             {
             od_control.baud=atol(tempstr);
             }

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.port=atoi(tempstr)-1;

          if(fgets((char *)storestr[6],80,fp)==NULL) goto no_go;

		  if(fgets((char *)&od_control.user_password,15,fp)==NULL) goto no_go;
		  toname(od_control.user_password);

          if(fgets((char *)storestr[2],80,fp)==NULL) goto no_go;
          if(fgets((char *)storestr[7],80,fp)==NULL) goto no_go;
          if(fgets((char *)storestr[8],80,fp)==NULL) goto no_go;
          if(fgets((char *)storestr[9],80,fp)==NULL) goto no_go;
          if(fgets((char *)storestr[10],80,fp)==NULL) goto no_go;
          if(fgets((char *)storestr[11],80,fp)==NULL) goto no_go;
          if(fgets((char *)storestr[12],80,fp)==NULL) goto no_go;

          fclose(fp);
          }

       else if(found==3)
          {
          if((fp=fopen(ipath,"r"))==NULL) goto no_go;

          od_control.od_info_type=SFDOORSDAT;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_num=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          tempstr[35]='\0';
          toname(tempstr);
          strcpy(od_control.user_name,tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          tempstr[15]='\0';
          toname(tempstr);
          strcpy(od_control.user_password,tempstr);

          if(fgets((char *)storestr[0],80,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.baud=atol(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.port=atoi(tempstr)-1;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.caller_timelimit=atoi(tempstr);

          if(fgets((char *)storestr[13],80,fp)==NULL) goto no_go;

          if(fgets((char *)storestr[14],80,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          strupr(tempstr);
          od_control.caller_ansi=(tempstr[0]=='T');

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_security=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_uploads=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_downloads=atoi(tempstr);

          if(fgets((char *)storestr[1],80,fp)==NULL) goto no_go;

          if(fgets((char *)storestr[2],255,fp)==NULL) goto no_go;
          sprintf(od_control.user_logintime,"%02.2d:%02.2d",atoi((char *)storestr[2])%60,atoi((char *)storestr[2])/60);

          if(fgets((char *)storestr[3],80,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          strupr(tempstr);
          od_control.ra_sysop_next=(tempstr[0]=='T');

          if(fgets((char *)storestr[4],80,fp)==NULL) goto no_go;

          if(fgets((char *)storestr[5],80,fp)==NULL) goto no_go;

          if(fgets((char *)storestr[6],80,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          strupr(tempstr);
          od_control.ra_error_free=(tempstr[0]=='T');

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_msg_area=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_file_area=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.od_node=atoi(tempstr);

          if(fgets((char *)storestr[10],80,fp)==NULL) goto no_go;

          if(fgets((char *)storestr[11],80,fp)==NULL) goto no_go;

          if(fgets((char *)storestr[12],80,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_todayk=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_upk=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_downk=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          tempstr[12]='\0';
          toname(tempstr);
          strcpy(od_control.user_homephone,tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          tempstr[25]='\0';
          toname(tempstr);
          strcpy(od_control.user_location,tempstr);

          if(fgets((char *)storestr[15],80,fp)==NULL)
             {
             storestr[13][0]='\0';
             }

          fclose(fp);
          }

       else if(found==4)
          {
          if((fp=fopen(ipath,"r"))==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;

          if(tempstr[0]=='C' && tempstr[1]=='O' && tempstr[2]=='M' && tempstr[4]==':')
             {                            /* GAP style DOOR.SYS */
             od_control.od_info_type=DOORSYS_GAP;

             od_control.port=tempstr[3]-'1';

             if(fgets((char *)storestr[0],80,fp)==NULL) goto no_go;
             if(fgets((char *)storestr[1],80,fp)==NULL) goto no_go;

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.od_node=atoi(tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             strupr(tempstr);
             if(strchr(tempstr,'N')!=NULL)
                {
                doorsys_lock=1;
                od_control.baud=atol(storestr[0]);
                }
             else if(strchr(tempstr,'Y')!=NULL)
                {
                doorsys_lock=2;
                od_control.baud=19200;
                }
             else
                {
                od_control.baud=atol(tempstr);
                }

             if(od_control.port==-1) od_control.baud=0L;

             if(fgets((char *)storestr[3],80,fp)==NULL) goto no_go;
             if(fgets((char *)storestr[4],80,fp)==NULL) goto no_go;
             if(fgets((char *)storestr[5],80,fp)==NULL) goto no_go;

             if(fgets((char *)storestr[22],80,fp)==NULL) goto no_go;

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             tempstr[35]='\0';
             toname(tempstr);
             strcpy(od_control.user_name,tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             tempstr[25]='\0';
             if(tempstr[strlen(tempstr)-1]=='\n') tempstr[strlen(tempstr)-1]='\0';
             strcpy(od_control.user_location,tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             tempstr[12]='\0';
             toname(tempstr);
             strcpy(od_control.user_homephone,tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             tempstr[12]='\0';
             toname(tempstr);
             strcpy(od_control.user_dataphone,tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             tempstr[15]='\0';
             if(tempstr[strlen(tempstr)-1]=='\n') tempstr[strlen(tempstr)-1]='\0';
             strcpy(od_control.user_password,tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.user_security=atoi(tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.user_numcalls=atoi(tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             tempstr[15]='\0';
             toname(tempstr);
             strcpy(od_control.user_lastdate,tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.caller_timelimit=atoi(tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             strupr(tempstr);
             if(!strcmp(tempstr,"RIP"))
                {
                od_control.user_rip=TRUE;
                od_control.user_ansi=TRUE;
                }
             else if(tempstr[0]=='G')
                {
                od_control.user_rip=FALSE;
                od_control.user_ansi=TRUE;
                }
             else
                {
                od_control.user_rip=FALSE;
                od_control.user_ansi=FALSE;
                }

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.user_screen_length=atoi(tempstr);

             if(fgets((char *)storestr[8],80,fp)==NULL) goto no_go;

             if(fgets((char *)storestr[9],80,fp)==NULL) goto no_go;
             if(storestr[9][strlen(storestr[9])-1]!='\n')
                {
                int ch;
                storestr[9][strlen(storestr[9])-1]='\n';
                do
                   {
                   ch = fgetc(fp);
                   } while(ch != '\n' && ch != EOF);
                }

again:
             if(fgets((char *)storestr[10],80,fp)==NULL) goto no_go;
             if(strchr(storestr[10],',')!=NULL) goto again;

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             tempstr[15]='\0';
             toname(tempstr);
             strcpy(od_control.ra_subdate,tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.user_num=atoi(tempstr);

             if(fgets((char *)storestr[6],80,fp)==NULL) goto no_go;

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.user_uploads=atoi(tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.user_downloads=atoi(tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.user_todayk=atoi(tempstr);

             if(fgets((char *)storestr[21],80,fp)==NULL) goto no_go;


                                        /* Beginning of extending DOOR.SYS data */
             fgets((char *)storestr[7],80,fp);
             fgets((char *)storestr[11],80,fp);
             fgets((char *)storestr[12],80,fp);
             fgets((char *)storestr[13],80,fp);
             if(fgets((char *)storestr[14],80,fp)!=NULL)
                {
                strncpy(od_control.user_birthday,storestr[7],8);
                od_control.user_birthday[8]='\0';

                strncpy(od_control.sysop_name,storestr[13],39);
                od_control.sysop_name[39]='\0';
                toname(od_control.sysop_name);

                strncpy(od_control.user_handle,storestr[14],35);
                od_control.user_handle[35]='\0';
                toname(od_control.user_handle);

                if(fgets(tempstr,255,fp)==NULL) goto finished;
                strncpy(od_control.event_starttime,tempstr,5);
                od_control.event_starttime[5]='\0';

                if(fgets(tempstr,255,fp)==NULL) goto finished;
                if(tempstr[0]=='y' || tempstr[0]=='Y')
                   od_control.user_error_free=TRUE;
                else
                   od_control.user_error_free=FALSE;

                if(fgets((char *)storestr[7],80,fp)==NULL) goto finished;
                if(fgets((char *)storestr[13],80,fp)==NULL) goto finished;
                if(fgets((char *)storestr[14],80,fp)==NULL) goto finished;
                if(fgets((char *)storestr[15],80,fp)==NULL) goto finished;
                if(fgets((char *)storestr[16],80,fp)==NULL) goto finished;

                if(fgets(tempstr,255,fp)==NULL) goto finished;
                strncpy(od_control.user_logintime,tempstr,5);
                od_control.user_logintime[5]='\0';

                if(fgets(tempstr,255,fp)==NULL) goto finished;
                strncpy(od_control.user_lasttime,tempstr,5);
                od_control.user_lasttime[5]='\0';

                if(fgets((char *)storestr[18],80,fp)==NULL) goto finished;
                if(fgets((char *)storestr[19],80,fp)==NULL) goto finished;

                if(fgets(tempstr,255,fp)==NULL) goto finished;
                od_control.user_upk=atoi(tempstr);

                if(fgets(tempstr,255,fp)==NULL) goto finished;
                od_control.user_downk=atoi(tempstr);

                if(fgets(tempstr,255,fp)==NULL) goto finished;
                strncpy(od_control.user_comment,tempstr,79);
                od_control.user_comment[79]='\0';
                if(od_control.user_comment[strlen(od_control.user_comment)-1]=='\n')
                   od_control.user_comment[strlen(od_control.user_comment)-1]='\0';
                if(fgets((char *)storestr[20],80,fp)==NULL) goto finished;

                if(fgets(tempstr,255,fp)==NULL) goto finished;
                od_control.user_messages=atoi(tempstr);


                od_control.od_info_type=DOORSYS_WILDCAT;
                }
             }

          else                            /* DoorWay style DOOR.SYS */
             {
             od_control.od_info_type=DOORSYS_DRWY;

             tempstr[35]='\0';
             toname(tempstr);
             strcpy(od_control.user_name,tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.port=tempstr[0]-'1';

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             if(od_control.port==-1)
                {
                od_control.baud=0L;
                }
             else
                {
                od_control.baud=atol(tempstr);
                }

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             od_control.caller_timelimit=atoi(tempstr);

             if(fgets(tempstr,255,fp)==NULL) goto no_go;
             if(tempstr[0]=='G')
                {
                od_control.caller_ansi=TRUE;
                }
             else
                {
                od_control.caller_ansi=FALSE;
                }
             }
finished:
          fclose(fp);
          }

       else if(found==5)
          {
          if((fp=fopen(ipath,"r"))==NULL) goto no_go;

          od_control.od_info_type=CALLINFO;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          tempstr[35]='\0';
          toname(tempstr);
          strcpy(od_control.user_name,tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          tempstr[25]='\0';
          toname(tempstr);
          strcpy(od_control.user_location,tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_security=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.caller_timelimit=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(tempstr[0]=='M')
             {
             od_control.caller_ansi=FALSE;
             }
          else
             {
             od_control.caller_ansi=TRUE;
             }

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          tempstr[15]='\0';
          toname(tempstr);
          strcpy(od_control.user_password,tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          tempstr[12]='\0';
          toname(tempstr);
          strcpy(od_control.user_homephone,tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.user_screen_length=atoi(tempstr);

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          if(fgets(tempstr,255,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.port=tempstr[3]-'1';

          if(fgets(tempstr,255,fp)==NULL) goto no_go;

          if(fgets(tempstr,255,fp)==NULL) goto no_go;
          od_control.baud=atol(tempstr);

          fclose(fp);
          }

/*
       else if(found==6)
          {
          od_control.od_info_type = PCBOARD;

          if((pcbsys = malloc(sizeof(struct _pcbsys))) == NULL)
             {
no_memory:
             od_control.od_error = ERR_MEMORY;
             goto no_go;
             }
          if((userssyshdr = malloc(sizeof(struct _userssyshdr))) == NULL) goto no_memory;
          if((userssysrec = malloc(sizeof(struct _userssysrec))) == NULL) goto no_memory;

          if((fp=fopen(ipath,"rb"))==NULL)
             {
no_open:
             od_control.od_error = ERR_FILEOPEN;
             goto no_go;
             }
          filesizea = (unsigned short int)_fsize(fp);
          if(filesizea > sizeof(struct _pcbsys)) filesizea = sizeof(struct _pcbsys);
          if(fread(pcbsys,1,filesizea,fp)!=filesizea)
             {
no_read:
             od_control.od_error = ERR_FILEREAD;
             goto no_go;
             }
          fclose(fp);

          if(pcbsys->node == ' ')
             {
             od_control.od_node = 1;
             }
          else
             {
             od_control.od_node = pcbsys->node;
             }
          od_control.user_ansi = pcbsys->useansi;
          if(pcbsys->comport)
             {
             od_control.baud = atol(pcbsys->dteportspeed);
             od_control.port = pcbsys->comport - 1;
             }
          else
             {
             od_control.baud = 0L;
             }

          if((fp=fopen(makepath(exitpath,"USERS.SYS"), "rb"))==NULL) goto no_open;

          if(fread(userssyshdr,1,sizeof(struct _userssyshdr),fp) != sizeof(struct _userssyshdr)) goto no_read;
          filesizeb = userssyshdr->SizeOfRec;
          if(filesizeb > sizeof(struct _userssysrec)) filesizeb = sizeof(struct _userssysrec);
          if(fread(userssysrec,1,filesizeb,fp) != filesizeb) goto no_read;
          fclose(fp);

          strncpy(od_control.user_name, userssysrec->Name, 25);
          od_control.user_name[25] = '\0';
          strncpy(od_control.user_location, userssysrec->City, 24);
          od_control.user_location[24] = '\0';
          strncpy(od_control.user_password, userssysrec->Password, 12);
          od_control.user_password[12] = '\0';
          strncpy(od_control.user_dataphone, userssysrec->BusDataPhone, 13);
          od_control.user_dataphone[13] = '\0';
          strncpy(od_control.user_homephone, userssysrec->HomeVoicePhone, 13);
          od_control.user_homephone[13] = '\0';
          // skipped ->LastDateOn
          strncpy(od_control.user_lasttime, userssysrec->LastTimeOn, 5);
          od_control.user_lasttime[5] = '\0';
          // skipped ->ExpertMode
          // skipped ->Protocol
          // skipped ->PackedFlags
          // skipped ->DateLastDirRead
          od_control.user_security = userssysrec->SecurityLevel;
          od_control.user_numcalls = userssysrec->NumTimesOn;
          od_control.user_screen_length = userssysrec->PageLen;
          od_control.user_uploads = userssysrec->NumUploads;
          od_control.user_downloads = userssysrec->NumDownloads;
          od_control.user_todayk = userssysrec->DailyDnldBytes;
          // skipped ->UserComment
          strncpy(od_control.user_comment, userssysrec->SysopComment, 30);
          od_control.user_comment[30] = '\0';
          od_control.user_time_used = userssysrec->ElapsedTimeOn;
          // skipped ->RegExpDate
          // skipped ->ExpSecurityLevel
          od_control.user_msg_area = userssysrec->LastConference;
          od_control.user_downk = long_shift_right(userssysrec->TotDnldBytes, 10);
          od_control.user_upk = long_shift_right(userssysrec->TotUpldBytes, 10);
          // skipped ->DeleteFlag
          od_control.user_num = (unsigned int)userssysrec->RecNum;
          // skipped ->MsgsRead
          od_control.user_messages = (unsigned int)userssysrec->MsgsLeft;
          }
*/
       else
          {
no_go:
          od_control.od_info_type = NO_DOOR_FILE;

          if(od_control.od_no_file_func != NULL)
             {
             (*od_control.od_no_file_func)();
             }

          if(od_control.od_force_local)
             {
             goto force_local;
             }

          if(od_control.od_info_type == NO_DOOR_FILE)
             {
             puts("Critical Error [OpenDoors]: Unable to read door information (drop) file.\n");
             exit(od_control.od_errorlevel[1]);
             }
          }

      _readexitinfo();
      }

   _initcs();
   }


static void _readexitinfo(void)
   {
   long file_size;
   FILE *fp;
   register char counter;

       od_control.od_extended_info=FALSE; /* EXITINFO.BBS info not available */
       od_control.od_ra_info=FALSE;       /* RA extended info not available */
                                          /* try to open EXITINFO.BBS */
       if(/*od_control.od_info_type == DORINFO1 &&*/ (fp=fopen(makepath(exitpath,"EXITINFO.BBS"),"rb"))!=NULL)
         {
         file_size=_fsize(fp);

         if(file_size>=2363)
            {
            if((ra2exitinfo=malloc(sizeof(struct _ra2exitinfo)))!=NULL)
               {
               if(fread(ra2exitinfo,1,2363,fp)==2363)
                  {
                  od_control.od_ra_info=TRUE;
                  od_control.od_extended_info=TRUE;
                  od_control.od_info_type=RA2EXITINFO;

                  od_control.baud=(long)ra2exitinfo->baud;
                  od_control.system_calls=ra2exitinfo->num_calls;
                  pasc2c(od_control.system_last_caller,ra2exitinfo->last_caller,35);
                  pasc2c(od_control.timelog_start_date,ra2exitinfo->start_date,8);
                  memcpy(&od_control.timelog_busyperhour,&ra2exitinfo->busyperhour,62);
                  pasc2c(od_control.user_name,ra2exitinfo->name,35);
                  pasc2c(od_control.user_location,ra2exitinfo->location,25);
                  pasc2c(od_control.user_org,ra2exitinfo->organisation,50);
                  for(counter=0;counter<3;++counter)
                     pasc2c(od_control.user_address[counter],ra2exitinfo->address[counter],50);
                  pasc2c(od_control.user_handle,ra2exitinfo->handle,35);
                  pasc2c(od_control.user_comment,ra2exitinfo->comment,80);
                  od_control.user_pwd_crc=ra2exitinfo->password_crc;
                  pasc2c(od_control.user_dataphone,ra2exitinfo->dataphone,15);
                  pasc2c(od_control.user_homephone,ra2exitinfo->homephone,15);
                  pasc2c(od_control.user_lasttime,ra2exitinfo->lasttime,5);
                  pasc2c(od_control.user_lastdate,ra2exitinfo->lastdate,8);
                  od_control.user_attribute=ra2exitinfo->attrib;
                  od_control.user_attrib2=ra2exitinfo->attrib2;
                  memcpy(&od_control.user_flags,&ra2exitinfo->flags,14);
                  od_control.user_security=ra2exitinfo->sec;
                  od_control.user_lastread=ra2exitinfo->lastread;
                  memcpy(&od_control.user_numcalls,&ra2exitinfo->nocalls,29);
                  od_control.user_group=ra2exitinfo->group;
                  memcpy(&od_control.user_combinedrecord,&ra2exitinfo->combinedrecord,200);
                  pasc2c(od_control.user_firstcall,ra2exitinfo->firstcall,8);
                  pasc2c(od_control.user_birthday,ra2exitinfo->birthday,8);
                  pasc2c(od_control.user_subdate,ra2exitinfo->subdate,8);
                  od_control.user_screenwidth=ra2exitinfo->screenwidth;
                  od_control.user_language=ra2exitinfo->language;
                  od_control.user_date_format=ra2exitinfo->dateformat;
                  pasc2c(od_control.user_forward_to,ra2exitinfo->forwardto,35);
                  memcpy(&od_control.user_msg_area,&ra2exitinfo->msgarea,15);
                  od_control.event_status=ra2exitinfo->status;
                  pasc2c(od_control.event_starttime,ra2exitinfo->starttime,5);
                  memcpy(&od_control.event_errorlevel,&ra2exitinfo->errorlevel,3);
                  pasc2c(od_control.event_last_run,ra2exitinfo->lasttimerun,8);
                  memcpy(&od_control.user_netmailentered,&ra2exitinfo->netmailentered,2);
                  pasc2c(od_control.user_logintime,ra2exitinfo->logintime,5);
                  pasc2c(od_control.user_logindate,ra2exitinfo->logindate,8);
                  memcpy(&od_control.user_timelimit,&ra2exitinfo->timelimit,6);
                  memcpy(&od_control.user_num,&ra2exitinfo->userrecord,8);
                  pasc2c(od_control.user_timeofcreation,ra2exitinfo->timeofcreation,5);
                  od_control.user_logon_pwd_crc=ra2exitinfo->logonpasswordcrc;
                  od_control.user_wantchat=ra2exitinfo->wantchat;
                  od_control.user_deducted_time=ra2exitinfo->deducted_time;
                  for(counter=0;counter<50;++counter)
                     pasc2c(od_control.user_menustack[counter],ra2exitinfo->menustack[counter],8);
                  od_control.user_menustackpointer=ra2exitinfo->menustackpointer;
                  memcpy(&od_control.user_error_free,&ra2exitinfo->error_free,3);
                  pasc2c(od_control.user_emsi_crtdef,ra2exitinfo->emsi_crtdef,40);
                  pasc2c(od_control.user_emsi_protocols,ra2exitinfo->emsi_protocols,40);
                  pasc2c(od_control.user_emsi_capabilities,ra2exitinfo->emsi_capabilities,40);
                  pasc2c(od_control.user_emsi_requests,ra2exitinfo->emsi_requests,40);
                  pasc2c(od_control.user_emsi_software,ra2exitinfo->emsi_software,40);
                  memcpy(&od_control.user_hold_attr1,&ra2exitinfo->hold_attr1,3);
                  pasc2c(od_control.user_reasonforchat,ra2exitinfo->page_reason,77);
                  ra_status_to_set=ra2exitinfo->status_line-1;
                  pasc2c(od_control.user_last_cost_menu,ra2exitinfo->last_cost_menu,8);
                  od_control.user_menu_cost=ra2exitinfo->menu_cost_per_min;
                  od_control.user_rip=ra2exitinfo->has_rip;

                  od_control.user_ansi=od_control.user_attribute&8;
                  od_control.user_avatar=od_control.user_attrib2&2;
                  }

               else
                  {
                  free(ra2exitinfo);
                  }
               }
            }

         else if(file_size>=1493)
            {
            if(read_exitinfo_primitive(fp,476))
               {
               if((ext_exitinfo=malloc(sizeof(struct _ext_exitinfo)))!=NULL)
                  {
                  if(fread(ext_exitinfo,1,sizeof(struct _ext_exitinfo), fp)==sizeof(struct _ext_exitinfo))
                     {                 /* transfer info into od_control struct */
                     pasc2c(od_control.caller_timeofcreation,exitinfo->bbs.ra.timeofcreation,5);
                     pasc2c(od_control.caller_logonpassword,exitinfo->bbs.ra.logonpassword,15);
                     od_control.caller_wantchat=exitinfo->bbs.ra.wantchat;

                     od_control.ra_deducted_time=ext_exitinfo->deducted_time;
                     for(counter=0;counter<50;++counter)
                        {
                        pasc2c(od_control.ra_menustack[counter],ext_exitinfo->menustack[counter],8);
                        }
                     od_control.ra_menustackpointer=ext_exitinfo->menustackpointer;
                     pasc2c(od_control.ra_userhandle,ext_exitinfo->userhandle,35);
                     pasc2c(od_control.ra_comment,ext_exitinfo->comment,80);
                     pasc2c(od_control.ra_firstcall,ext_exitinfo->firstcall,8);
                     memcpy(od_control.ra_combinedrecord,ext_exitinfo->combinedrecord,25);
                     pasc2c(od_control.ra_birthday,ext_exitinfo->birthday,8);
                     pasc2c(od_control.ra_subdate,ext_exitinfo->subdate,8);
                     od_control.user_screenwidth=ext_exitinfo->screenwidth;
                     od_control.user_msg_area=ext_exitinfo->msgarea;
                     od_control.user_file_area=ext_exitinfo->filearea;
                     od_control.user_language=ext_exitinfo->language;
                     od_control.user_date_format=ext_exitinfo->dateformat;
                     pasc2c(od_control.ra_forward_to,ext_exitinfo->forwardto,35);
                     memcpy(&od_control.ra_error_free,&ext_exitinfo->error_free,3);
                     pasc2c(od_control.ra_emsi_crtdef,ext_exitinfo->emsi_crtdef,40);
                     pasc2c(od_control.ra_emsi_protocols,ext_exitinfo->emsi_protocols,40);
                     pasc2c(od_control.ra_emsi_capabilities,ext_exitinfo->emsi_capabilities,40);
                     pasc2c(od_control.ra_emsi_requests,ext_exitinfo->emsi_requests,40);
                     pasc2c(od_control.ra_emsi_software,ext_exitinfo->emsi_software,40);
                     memcpy(&od_control.ra_hold_attr1,&ext_exitinfo->hold_attr1,3);

                     od_control.od_ra_info=TRUE;
                     od_control.od_extended_info=TRUE;
                     od_control.od_info_type=RA1EXITINFO;
                     }
                  }
               }
            }

         else if(file_size>476)
            {
            if(file_size>644) file_size=644;

            if(read_exitinfo_primitive(fp,(int)file_size))
               {
               od_control.caller_wantchat=exitinfo->bbs.qbbs.qwantchat;
               for(counter=0;counter<exitinfo->bbs.qbbs.gosublevel;++counter)
                  {
                  pasc2c(od_control.ra_menustack[counter],exitinfo->bbs.qbbs.menustack[counter],8);
                  }
               od_control.ra_menustackpointer=exitinfo->bbs.qbbs.gosublevel;
               pasc2c(od_control.ra_menustack[od_control.ra_menustackpointer],exitinfo->bbs.qbbs.menu,8);

               od_control.od_extended_info=TRUE;
               od_control.od_info_type=QBBS275EXITINFO;
               _initial_elapsed=exitinfo->elapsed;
               }
            }

         else if(file_size>=452)
            {
            if(read_exitinfo_primitive(fp,(int)file_size))
               {
               pasc2c(od_control.caller_timeofcreation,exitinfo->bbs.ra.timeofcreation,5);
               pasc2c(od_control.caller_logonpassword,exitinfo->bbs.ra.logonpassword,15);
               od_control.caller_wantchat=exitinfo->bbs.ra.wantchat;

               od_control.od_extended_info=TRUE;
               od_control.od_info_type=EXITINFO;
               }
            }

         od_control.od_page_pausing=od_control.user_attribute&0x04;

         fclose(fp);
         }
   }


static void _initcs(void)
   {
   register char counter;
   char pressed;
   time_t now;

   if(!od_control.od_list_title_col) od_control.od_list_title_col=0x0f;
   if(!od_control.od_continue_col) od_control.od_continue_col=0x0f;
   if(!od_control.od_list_name_col) od_control.od_list_name_col=0x0e;
   if(!od_control.od_list_size_col) od_control.od_list_size_col=0x0d;
   if(!od_control.od_list_comment_col) od_control.od_list_comment_col=0x03;
   if(!od_control.od_list_offline_col) od_control.od_list_offline_col=0x0c;
   if(!od_control.od_menu_title_col) od_control.od_menu_title_col=0x74;
   if(!od_control.od_menu_border_col) od_control.od_menu_border_col=0x70;
   if(!od_control.od_menu_text_col) od_control.od_menu_text_col=0x70;
   if(!od_control.od_menu_key_col) od_control.od_menu_key_col=0x7f;
   if(!od_control.od_menu_highlight_col) od_control.od_menu_highlight_col=0x07;
   if(!od_control.od_menu_highkey_col) od_control.od_menu_highkey_col=0x0f;

/* A possible default colour set for blue-background menus:
 *
 * if(!od_control.od_menu_title_col) od_control.od_menu_title_col=0x1f;
 * if(!od_control.od_menu_border_col) od_control.od_menu_border_col=0x1f;
 * if(!od_control.od_menu_text_col) od_control.od_menu_text_col=0x17;
 * if(!od_control.od_menu_key_col) od_control.od_menu_key_col=0x1c;
 * if(!od_control.od_menu_highlight_col) od_control.od_menu_highlight_col=0x70;
 * if(!od_control.od_menu_highkey_col) od_control.od_menu_highkey_col=0x74;
 */


   od_control.od_noexit=FALSE;
   od_control.od_colour_char='@';
   od_control.od_colour_delimiter='`';


   od_control.od_okaytopage=MAYBE;     /* allow user to page */
   od_control.od_pagestartmin=od_control.od_pageendmin=1440;


   od_control.od_inactivity=200;       /* maximum user inactivity of 200 secs. */
   od_control.od_inactive_warning = 10;
   od_control.od_cur_attrib=-1;
   od_control.od_clear_on_exit=TRUE;   /* enable clearscren on door exit */
   od_control.od_no_ra_codes=TRUE;     /* Disable RA/QBBS control codes */
   if(od_control.od_chat_color1==0) od_control.od_chat_color1=0x0c;
   if(od_control.od_chat_color2==0) od_control.od_chat_color2=0x0f;
   if(strlen(od_control.od_logfile_name)==0) strcpy(od_control.od_logfile_name,"DOOR.LOG");

                                       /* set default messages and prompts */
   od_control.od_before_shell="\n\rPlease wait a moment...\n\r";
   od_control.od_after_shell="\n\r...Thanks for waiting\n\r\n\r";
   od_control.od_help_text="  Alt: [C]hat [H]angup [L]ockout [J]Dos [K]eyboard-Off [D]rop to BBS            ";
   od_control.od_before_chat="\n\rThe system operator has placed you in chat mode to talk with you:\n\r\n\r";
   od_control.od_after_chat="\n\rChat mode ended.\n\r\n\r";
   od_control.od_inactivity_timeout="\n\rMaximum user inactivity time has elapsed, please call again.\n\r\n\r";
   od_control.od_inactivity_warning="\n\rWARNING: Inactivity timeout in 5 seconds, press a key now to remain online.\n\r\n\r";
   od_control.od_time_warning="\n\rWARNING: You only have %d minute(s) remaining for this session.\n\r\n\r";
   od_control.od_time_left="%d mins   ";
   od_control.od_sysop_next="[SN] ";
   od_control.od_no_keyboard="[Keyboard]";
   od_control.od_want_chat="[Want-Chat]";
   od_control.od_no_time="\n\rSorry, you have used up of your time for this session.\n\r\n\r";
   od_control.od_no_sysop="\n\rSorry, the system operator is not available at this time.\n\r";
   od_control.od_press_key="Press [Enter] to continue";
   od_control.od_chat_reason="               Why would you like to chat? (Blank line to cancel)\n\r";
   od_control.od_paging="\n\rPaging system operator for chat";
   od_control.od_no_response=" No response.\n\r\n\r";
   od_control.od_status_line[0]="                                                                     [Node:     ";
   od_control.od_status_line[1]="%s of %s at %u BPS";
   od_control.od_status_line[2]="Security:        Time:                                               [F9]=Help ";
   od_control.od_month[0]="Jan";
   od_control.od_month[1]="Feb";
   od_control.od_month[2]="Mar";
   od_control.od_month[3]="Apr";
   od_control.od_month[4]="May";
   od_control.od_month[5]="Jun";
   od_control.od_month[6]="Jul";
   od_control.od_month[7]="Aug";
   od_control.od_month[8]="Sep";
   od_control.od_month[9]="Oct";
   od_control.od_month[10]="Nov";
   od_control.od_month[11]="Dec";
   od_control.od_day[0]="Sun";
   od_control.od_day[1]="Mon";
   od_control.od_day[2]="Tue";
   od_control.od_day[3]="Wed";
   od_control.od_day[4]="Thu";
   od_control.od_day[5]="Fri";
   od_control.od_day[6]="Sat";
   od_control.od_offline="[OFFLINE] ";
   od_control.od_continue="Continue? [Y/n/=]";
   od_control.od_continue_yes='y';     /* Continue? responses */
   od_control.od_continue_no='n';
   od_control.od_continue_nonstop='=';
   od_control.od_help_text2="  OpenDoors 5.00 - (C) Copyright 1994, Brian Pirie - Registered Version        ";
   od_control.od_sending_rip="¥ Sending RIP File √";
   od_control.od_hanging_up="Terminating Call";


   if(od_control.od_prog_name[0]=='\0')
      {
      strcpy(od_control.od_prog_name,"OpenDoors 5.00");
      }

   for(counter=0;counter<14;++counter)
      {
      if(od_control.od_logfile_messages[counter] == NULL)
         {
         (char *)od_control.od_logfile_messages[counter]=(char *)_log_messages[counter];
         }
      }

   start_remaining=od_control.user_timelimit;

   if(od_control.od_maxtime>0 && od_control.od_maxtime<=1440)
      {
      if(od_control.user_timelimit>od_control.od_maxtime)
         {
         od_control.od_maxtime_deduction=od_control.user_timelimit-od_control.od_maxtime;
         od_control.user_timelimit=od_control.od_maxtime;
         }
      }

                                    /* If sysop name unkown, use word "Sysop" */
   if(strlen(od_control.sysop_name)==0) strcpy(od_control.sysop_name,"Sysop");

                                       /* If in foced local mode and user     */
                                       /* name has not yet been set           */
   if(od_control.od_force_local && od_control.user_name[0] == '\0')
      {
      /* Use sysop's name */
      if(sysop_name_set)
         {
         strcpy(od_control.user_name, forced_sysop_name);
         }
      else
         {
         strcpy(od_control.user_name, od_control.sysop_name);
         }
      }

   _file_bps = od_control.baud;

   _com_ini();                         /* Open serial port, if applicable */

   now = time(NULL);
   next_statup = now + STATUS_FREQUENCY;
   last_activity = now;
   next_minute = now + 60L;
   last_status=od_control.od_status_on=TRUE;

   phys_init();
   phys_window(1,1,80,23);                  /* set local window */

   if(_set_personality==NULL)
      {
no_default:
      if (od_control.od_default_personality == NULL)
         {
         current_status_function = pdef_opendoors;
         }
      else
         {
         current_status_function = od_control.od_default_personality;
         }
      (*current_status_function)(20);
      if(_ra_status)
         {
         od_set_statusline(ra_status_to_set);
         }
      else
         {
         od_set_statusline(0);
         }
      }
   else
      {
      if(!((*_set_personality)(_desired_personality)))
         {
         goto no_default;
         }
      }

   if(_preset)
      {
      atexit(_preexit);
      _preset=FALSE;
      }

   if(od_control.od_logfile!=NULL)
       (*od_control.od_logfile)();

   /* Setup remote terminal for ANSI graphics if operating in RIP mode */
   if(od_control.user_rip)
      {
      od_clr_scr();
      }

   if(_cP_)                            /* display copyright information */
      {
      if(!od_control.od_nocopyright)
         {
         od_set_attrib(0x07);
         od_disp("\r\n",2,FALSE);
         od_disp_str(copyright);
         od_disp_str(reg_str);
         }
      }
   else
      {
      od_set_attrib(0x07);
      od_disp("\r\n",2,FALSE);
      od_disp_str(copyright);
      od_disp_str(notreg_str);
      od_clear_keybuffer();
      do
         {
         pressed=od_get_key(TRUE);
         }
      while(pressed!='\n' && pressed!='\r');
      od_clear_keybuffer();
      }
   }
