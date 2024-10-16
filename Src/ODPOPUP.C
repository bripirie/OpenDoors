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
 *     Filename : ODPOPUP.C
 *  Description : Contains code for popup menus
 *      Version : 5.00
 */

#include <alloc.h>
#include <ctype.h>
#include <string.h>

#include "opendoor.h"
#include "odintern.h"

struct _menuinfo menuinfo[MENU_LEVELS];
char _menucorrect;
char _menucommand;
int _menuflags;
char _menuitems;
char _menlev;

/* Returns -1 on error, 0 on escape, +ve value on selection, -2 left, -3 right */
/* title can be empty or NULL */
int od_popup_menu(char *title, char *text, int _left, int _top, int level, unsigned int _flags)
   {
   struct _menu *menu=NULL;
   char counter;
   char width;
   char right;
   char bottom;
   char cursor;
   int left;
   int top;
   void *win;
   char between_size;
   char title_size;
   char remaining;
   char line_counter;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_popup_menu()");

   /* Initialize OpenDoors, if not already done */
   if(!inited) od_init();

   /* Setup od_box_chars appropriately */
   if(od_control.od_box_chars[BOX_BOTTOM]==0)
      {
      od_control.od_box_chars[BOX_BOTTOM] = od_control.od_box_chars[BOX_TOP];
      }
   if(od_control.od_box_chars[BOX_RIGHT]==0)
      {
      od_control.od_box_chars[BOX_RIGHT] = od_control.od_box_chars[BOX_LEFT];
      }

   /* check level bounds */
   if(level < 1 || level > MENU_LEVELS)
      {
      od_control.od_error = ERR_LIMIT;
      return(-1);
      }
   /* normalize level */
   _menlev=--level;

   if(menuinfo[level].win==NULL)
      {
      left = _left;
      top = _top;
      _menuflags = _flags;

      if(text == NULL)
         {
         od_control.od_error = ERR_PARAMETER;
         return(-1);
         }

      if(menu == NULL)
         {
         if((menu = malloc(sizeof(struct _menu) * 21)) == NULL)
            {
            od_control.od_error = ERR_PARAMETER;
            return(-1);
            }
         }
      menuinfo[level].menu=menu;

      _menuitems=0;
      width=0;
      counter=0;
      _menucommand = -10;
      menu[0].key = 0;
      while(*text && _menuitems < 21)
         {
         switch(*text)
            {
            case '|':
                  menu[_menuitems++].text[counter]='\0';
                  if(counter > width) width = counter;
                  counter = 0;
                  menu[_menuitems].key = 0;
               break;

            case '^':
               if(counter < 76) menu[_menuitems].key=counter;
               break;

            default:
               if(counter<76)
                  {
                  menu[_menuitems].text[counter++]=*text;
                  }
            }
         ++text;
         }

      /* If we were in the middle of a menu item when we encountered the end */
      /* of the string, then it should form an additional menu entry. This   */
      /* handles the case of a menu string to no terminating | for the last  */
      /* entry.                                                              */
      if(counter!=0)
         {
         /* null-terminate current menu entry string */
         menu[_menuitems++].text[counter]='\0';

         /* If this is the widest entry, update he menu width appropriately  */
         if(counter > width) width = counter;
         }

      /* If the menu description string does not contain any menu items */
      if(_menuitems == 0)
         {
         /* Return with parameter error */
         od_control.od_error = ERR_PARAMETER;
         return(-1);
         }

      /* Adjust menu width to allow title to fit, if possible               */
      /* If a title string was passed, and that string is wider than widest */
      /* menu entry ...                                                     */
      if(title != NULL && strlen(title) + 2 > width)
         {
         /* Then width of menu window should be large enough to allow up to */
         /* the first 76 characters of the title to fit.                    */
         width = strlen(title) + 2 > 76 ? 76 : strlen(title) + 2;
         }

      /* Based on number and size of menu items, and width of title,         */
      /* determine the bottom, right and inside width of the menu.           */
      bottom = top + _menuitems + 1;
      right = left + width + 3;
      between_size = (right - left) - 1;

      /* If neither ANSI nor AVATAR mode is available, return with an error */
      if(!(od_control.user_ansi || od_control.user_avatar))
         {
         od_control.od_error = ERR_NOGRAPHICS;
         return(-1);
         }

      /* If menu would "fall off" edge of screen, return with an error */
      if(left<1 || top<1 || right>80 || bottom>25 || right-left < 2 || bottom-top < 2)
         {
         od_control.od_error = ERR_PARAMETER;
         return(-1);
         }

      /* Allocate space to store window information. If unable to allocate */
      /* enough space, return with an error.                               */
      if((win=malloc( (right-left+1)*2 + (bottom-top+1)*160)) == NULL)
         {
         od_control.od_error = ERR_MEMORY;
         return(-1);
         }

      /* Store contents of screen where memu will be drawn in the temporary */
      /* buffer.                                                            */
      if(!od_gettext(left, top, right, bottom, win))
         {
         free(win);
         win=NULL;
         return(-1);          /* (od_error code has been set in od_gettext()) */
         }

      /* Determine number of characters of title to be displayed */
      if(title==NULL)
         {
         title_size = 0;
         }
      else
         {
         if((title_size = strlen(title)) > (between_size - 4))
            {
            title_size = between_size - 4;
            }
         }

      od_set_cursor(top,left);
      od_set_attrib(od_control.od_menu_border_col);
      od_putch(od_control.od_box_chars[BOX_UPPERLEFT]);
      if(title_size == 0)
         {
         od_repeat(od_control.od_box_chars[BOX_TOP],between_size);
         }
      else
         {
         od_repeat(od_control.od_box_chars[BOX_TOP],remaining=((between_size-title_size-2)/2));
         od_set_attrib(od_control.od_menu_title_col);
         od_putch(' ');
         od_disp(title,title_size,TRUE);
         od_putch(' ');
         od_set_attrib(od_control.od_menu_border_col);
         od_repeat(od_control.od_box_chars[BOX_TOP],between_size-remaining-title_size-2);
         }
      od_putch(od_control.od_box_chars[BOX_UPPERRIGHT]);

      line_counter = top + 1;
      _menucorrect=0;
      _menu_look();
      cursor = _menucorrect;
      for(counter=0; counter<_menuitems && line_counter < bottom; ++counter)
         {
         _menu_look();
         if(_menucommand!=-10 && !(_menuflags & MENU_KEEP))
            {
            goto exit_now;
            }

         od_set_cursor(line_counter,left);
         od_putch(od_control.od_box_chars[BOX_LEFT]);
         od_set_attrib(od_control.od_menu_text_col);

         if(counter==cursor)
            {
            _menu_line(left, top, menu, counter, TRUE, width, FALSE);
            }
         else
            {
            _menu_line(left, top, menu, counter, FALSE, width, FALSE);
            }

         od_set_attrib(od_control.od_menu_border_col);
         od_putch(od_control.od_box_chars[BOX_RIGHT]);
         ++line_counter;
         }

      od_set_cursor(bottom,left);
      od_putch(od_control.od_box_chars[BOX_LOWERLEFT]);
      od_repeat(od_control.od_box_chars[BOX_BOTTOM],between_size);
      od_putch(od_control.od_box_chars[BOX_LOWERRIGHT]);
      od_set_cursor(top+1,left+1);
      }
   else
      {
      menu=menuinfo[level].menu;
      _menuitems=menuinfo[level].menu_items;
      width=menuinfo[level].width;
      right=menuinfo[level].right;
      bottom=menuinfo[level].bottom;
      left=menuinfo[level].left;
      top=menuinfo[level].top;
      _menuflags=menuinfo[level].flags;
      win=menuinfo[level].win;
      _menucorrect=cursor=menuinfo[level].cursor;
      _menucommand=-10;

      if(_flags & MENU_DESTROY)
         {
         _menucommand=0;
         goto destroy;
         }

      /* Otherwise, position flashing hardware cursor appropriately */
      od_set_cursor(top+cursor+1, left+1);
      }

   for(;;)
      {
      _menu_look();
      if(_menucorrect != cursor)
         {
         _menu_line(left, top, menu, cursor, FALSE, width, TRUE);
         cursor = _menucorrect;
         _waitdrain(1);
         _menu_look();
         _menu_line(left, top, menu, cursor, TRUE, width, TRUE);
         }

      if(_menucommand != -10)
         {
         goto exit_now;
         }
      }

exit_now:
   if((!(_menuflags & MENU_KEEP)) || _menucommand <=0)
      {
destroy:
      od_puttext(left, top, right, bottom, win);
      free(win);
      menuinfo[level].win=NULL;
      }
   else if(_menuflags & MENU_KEEP)
      {
      menuinfo[level].menu=menu;
      menuinfo[level].menu_items=_menuitems;
      menuinfo[level].width=width;
      menuinfo[level].right=right;
      menuinfo[level].bottom=bottom;
      menuinfo[level].cursor=cursor;
      menuinfo[level].left=left;
      menuinfo[level].top=top;
      menuinfo[level].flags=_menuflags;
      menuinfo[level].win=win;
      }

   return(_menucommand);
   }


void _menu_look(void)
   {
   char pressed;
   char counter;
   long timer;

   /* Loop, processing keys. If a command has been selected, stop looping */
   /* immediately. If there are no more keys waiting, stop looping        */
   while(_menucommand==-10)
      {
      od_kernal();
      if(b_head == b_tail) return;

      pressed = od_get_key(TRUE);

      if(pressed==27)
         {
         timer=_clock_tick();
         while(timer + 2L>_clock_tick() && _clock_tick() >= timer
               && (pressed=od_get_key(FALSE))==0)
            {
            od_kernal();
            }

         if(pressed != '[')
            {
            if(_menuflags & MENU_ALLOW_CANCEL)
               {
               _menucommand = 0;
               return;
               }
            }
         else
            {
            timer=_clock_tick();
            while(timer + 9L > _clock_tick() && _clock_tick() >= timer && (pressed=od_get_key(FALSE))==0)
               {
               od_kernal();
               }
            if(pressed == 0) return;
            switch(pressed)
               {
               case 'A':
                  goto up_arrow;

               case 'B':
                  goto down_arrow;

               case 'C':
                  goto right_arrow;

               case 'D':
                  goto left_arrow;
               }
            }
         }

      else if(pressed==0)
         {
         /* Get the next key from the keyboard */

         timer=_clock_tick();
         while(timer+9L> _clock_tick() && _clock_tick() >= timer && (pressed=od_get_key(FALSE))==0)
            {
            od_kernal();
            }
         if(pressed == 0) return;

         /* Respond appropriately */
         switch(pressed)
            {
            case 0x48:
up_arrow:
               if(--_menucorrect < 0) _menucorrect = _menuitems - 1;
               break;

            case 0x50:
down_arrow:
               if(++_menucorrect >= _menuitems) _menucorrect = 0;
               break;

            case 0x4b:
left_arrow:
               if(_menuflags & MENU_PULLDOWN)
                  {
                  _menucommand = -2;
                  return;
                  }
               else
                  {
                  goto up_arrow;
                  }

            case 0x4d:
right_arrow:
               if(_menuflags & MENU_PULLDOWN)
                  {
                  _menucommand = -3;
                  return;
                  }
               else
                  {
                  goto down_arrow;
                  }
            }
         }

      else if (pressed == '\n' || pressed == '\r')
         {
         _menucommand = _menucorrect + 1;
         return;
         }

      else if(pressed == 5)
         {
         goto up_arrow;
         }

      else if(pressed == 24)
         {
         goto down_arrow;
         }

      else if(pressed == 19)
         {
         goto left_arrow;
         }

      else if(pressed == 4)
         {
         goto right_arrow;
         }

      else
         {
         /* Check whether key is a menu "hot key" */
         for(counter=0; counter<_menuitems; ++counter)
            {
            if(toupper(menuinfo[_menlev].menu[counter].text[menuinfo[_menlev].menu[counter].key]) == toupper(pressed))
               {
               _menucorrect = counter;
               _menucommand = _menucorrect + 1;
               return;
               }
            }

         /* At this point, we know that key was not one of the "hot keys" */
         /* Check for 4, 6, 8 and 2 keys as arrow keys.                   */
         if(pressed == '4')
            {
            goto left_arrow;
            }
         else if(pressed == '6')
            {
            goto right_arrow;
            }
         else if(pressed == '8')
            {
            goto up_arrow;
            }
         else if(pressed == '2')
            {
            goto down_arrow;
            }
         }

      }
   }


void _menu_line(int left, int top, struct _menu *menu, int which, int highlight, int width, int position)
   {
   register char counter;
   register char *string;
   register char pos;
   char text_colour;
   char key_colour;

   ++left;
   ++top;

   text_colour = highlight ? od_control.od_menu_highlight_col : od_control.od_menu_text_col;
   key_colour = highlight ? od_control.od_menu_highkey_col : od_control.od_menu_key_col;

   string = (char *)(menu[which].text);
   pos = menu[which].key;

   if(position) od_set_cursor(top+which, left);

   od_set_attrib(text_colour);
   od_putch(' ');

   for(counter=0; counter<width && *string; ++counter)
      {
       if(counter == pos)
          {
          od_set_attrib(key_colour);
          od_putch(*string++);
          od_set_attrib(text_colour);
          }
       else
          {
          od_putch(*string++);
          }
      }

   od_repeat(' ', (width-counter) + 1);

   if(position) od_set_cursor(top+which, left);
   }
