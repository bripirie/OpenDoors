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
 *     Filename : ODLIST.C
 *  Description : Code for the FILES.BBS listing function.
 *      Version : 5.00
 */

#include<stdio.h>                      /* Standard header files */
#include<ctype.h>
#include<string.h>

#ifndef USEINLINE
#include<dos.h>
#endif

#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"


int od_list_files(char *_directory)    /* Function to display Advanced format */
   {                                   /* FILES.BBS file listing */

   register char line_count=2;         /* Count of # of lines on screen */
   char pausing;                       /* Indicates whether page pausing is on */
   static char line[513];              /* FILES.BBS line */
   static char filename[80];           /* String containing the full filename */
   static char drive[3];               /* String containing the drivename */
   static char dir[70];                /* String containing the directory name */
   static char tempstr1[9];            /* Temporary storage strings */
   static char tempstr2[5];
   static char basename[9];            /* String containing the filename base */
   static char extention[5];           /*     "        " the filename extention */
   static char directory[100];
   register int filename_info;         /* flags containing information about a filename */
   register FILE *fp;                  /* file pointer of files.bbs file */
   static char *string;
   char is_dir;
   char use_next_line = TRUE;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_list_files()");

   if(!inited) od_init();              /* check if we've been inited */

                                       /*  Check user's page pausing setting. */
   pausing=od_control.od_page_pausing;

   if(od_control.od_extended_info) pausing=od_control.user_attribute&0x04;

   if(_directory==NULL)                /* Parse directory parameter */
      {
      strcpy(globworkstr,".");
      strcpy(directory,".\\");
      }
   else if(*_directory=='\0')
      {
      strcpy(globworkstr,".");
      strcpy(directory,".\\");
      }
   else
      {
      strcpy(globworkstr,_directory);
      strcpy(directory,_directory);
      if(globworkstr[strlen(globworkstr)-1] == '\\')
         {
         globworkstr[strlen(globworkstr)-1] = '\0';
         }
      }
                                         /* Get directory information on path */
   if(_odfindfirst(globworkstr,&_fblk,0x20|0x01|0x10)) return(FALSE);

   if(_fblk.ff_attrib&0x10)                           /* If it is a directory */
      {                          /* Append FILES.BBS to directory name & open */
      is_dir = TRUE;
      if((fp=fopen(makepath(globworkstr,"FILES.BBS"),"r"))==NULL)
         {
         od_control.od_error = ERR_FILEOPEN;
         return(FALSE);
         }
      }
   else                                           /* If it is not a directory */
      {                           /* Don't append FILES.BBS name when opening */
      is_dir = FALSE;
      if((fp=fopen(globworkstr,"r"))==NULL)
         {
         od_control.od_error = ERR_FILEOPEN;
         return(FALSE);
         }
      }

   _last_control_key=0;             /* Ignore previously pressed control keys */


   /* Loop until the end of the FILES.BBS file has been reached */
   for(;;)
      {
      if(fgets(line,512,fp)==NULL) break;

      if(!use_next_line)
         {
         if(line[strlen(line)-1]=='\n')
            {
            use_next_line = TRUE;
            }
         continue;
         }

      if(line[strlen(line)-1]=='\n')
         {
         line[strlen(line)-1]='\0';
         }
      else
         {
         use_next_line = FALSE;
         }

      if(_last_control_key!=0)
         {
         switch(_last_control_key)
            {
            case 's':
               if(od_control.od_list_stop)
                  {
                  if(od_control.baud)
                     {
                     _com_clear_outbound();
                     }
                  od_clear_keybuffer();
                  fclose(fp);
                  return(TRUE);
                  }
               break;

            case 'p':
               if(od_control.od_list_pause)
                  {
                  od_clear_keybuffer();
                  od_get_key(TRUE);
                  }
            }
         _last_control_key=0;
         }

                                       /* Determine whether or not this is a */
                                       /* comment line */
      if(line[0]==' ' || strlen(line)==0)
         {                             /* If so, display the line in comment */
                                       /*  colour */
         od_set_attrib(od_control.od_list_title_col);
         od_disp_str(line);
         od_disp_str("\n\r");
         ++line_count;
         }
      else                             /* If the line is not a comment */
         {
         first_word(line,filename);    /* Extract the first word of the line, */
                                       /* And extract the filename */
         filename_info=_fn_splt(filename,drive,dir,basename,extention);
         if(!((filename_info&DRIVE) || (filename_info&DIRECTORY)))
            {
            if(is_dir)
               {
               strcpy(filename,makepath(directory,filename));
               }
            else
               {
               _fn_splt(directory,drive,dir,tempstr1,tempstr2);
               _fn_mrg(filename,drive,dir,basename,extention);
               }
            }
                                       /* Search for the filespec in directory */
         if(_odfindfirst(filename,&_fblk,0x20|0x01)==0)
            {
            do                         /* If filename was found, */
               {                       /* Display information on every file that matches */
               od_set_attrib(od_control.od_list_name_col);
               od_printf("%-12.12s  ",_fblk.ff_name);
               od_set_attrib(od_control.od_list_size_col);
               od_printf("%-6ld   ",_fblk.ff_fsize);
               od_set_attrib(od_control.od_list_comment_col);
               string=other_words(line);
               if(strlen(string)<=56)
                  {
                  od_disp_str(string);
                  od_disp_str("\n\r");
                  }
               else
                  {
                  od_printf("%-56.56s\n\r",string);
                  }
               ++line_count;
               } while(_odfindnext(&_fblk)==0);
            }
         else                          /* Otherwise, indicate that the file is "Offline" */
            {
            _fn_mrg(filename,"","",basename,extention);
            od_set_attrib(od_control.od_list_name_col);
            od_printf("%-12.12s ",filename);
            od_set_attrib(od_control.od_list_offline_col);
            od_disp_str(od_control.od_offline);
            od_set_attrib(od_control.od_list_comment_col);

            od_printf("%-56.56s\n\r",other_words(line));
            ++line_count;
            }
         }

                                       /* Check for end of screen & page pausing */
      if(line_count>=od_control.user_screen_length && pausing)
         {                             /* Provide page pausing at end of each screen */
         if(_pageprompt(&pausing))
            {
            fclose(fp);
            return(TRUE);
            }

         line_count=2;                 /* Reset the line number counter */
         }
      }

   fclose(fp);                         /* When finished, close the file, */
   return(TRUE);                       /*  and exit with success */
   }


void _fn_mrg(char *path, const char *drive, const char *dir, const char *name, const char *ext)
   {
   if(path == NULL) return;

   path[0]='\0';

   if(drive != NULL)
      {
      strcpy(path,drive);
      }
   if(dir != NULL)
      {
      strcat(path,dir);
      }
   if(name != NULL)
      {
      strcat(path,name);
      }
   if(ext != NULL)
      {
      strcat(path,ext);
      }
   }


int  _fn_splt(const char *path, char *drive, char *dir, char *name, char *ext)
   {
   char *where;
   char *start;
   char size;
   int to_return;

   if(path == NULL || drive == NULL || dir == NULL || name == NULL || ext == NULL) return(0);

   start = (char *)path;
   to_return=0;

   if((where = strrchr(start,':'))==NULL)
      {
      drive[0]='\0';
      }
   else
      {
      size = (int)(where - start) + 1;
      if(size > 2) size = 2;
      strncpy(drive,start,size);
      drive[size] = '\0';
      start = where + 1;
      to_return |= DRIVE;
      }

   if((where = strrchr(start,'\\'))==NULL)
      {
      dir[0]='\0';
      }
   else
      {
      size = (int)(where - start) + 1;
      strncpy(dir,start,size);
      dir[size] = '\0';
      start = where + 1;
      to_return |= DIRECTORY;
      }

   if(strchr(start,'*')!=NULL || strchr(start, '?')!=NULL)
      {
      to_return |= WILDCARDS;
      }

   if((where = strrchr(start,'.'))==NULL)
      {
      if(start =='\0')
         {
         ext[0]='\0';
         name[0]='\0';
         }
      else
         {
         ext[0]='\0';
         size = strlen(start);
         if (size > 8) size = 0;
         strncpy(name,start,size);
         name[size]='\0';
         to_return |= FILENAME;
         }
      }
   else
      {
      to_return |= FILENAME;
      to_return |= EXTENSION;

      size = (int)(where - start);

      if(size > 8) size = 8;

      strncpy(name,start,size);
      name[size] = '\0';

      size = strlen(where);
      if(size > 4) size = 4;
      strncpy(ext, where, size);
      ext[size]='\0';
      }

   return(to_return);
   }
