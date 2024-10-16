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
 *     Filename : ODEDSTR.C
 *  Description : Code for the ANSI/AVATAR fancy line editing function,
 *                od_edit_str().
 *      Version : 5.00
 */



#include "opendoor.h"                 /* Include OpenDoors standard header file */
#include "odintern.h"

#include <ctype.h>                                        /* Other header files */
#include <stddef.h>
#include <string.h>



unsigned char _format_offset[80];                 /* Offset of each format char */
char _format_literal[80];                               /* Is character literal */
char _original_string[81];                          /* Original value of string */
char *_input_string;
char *_format_string;
unsigned char _string_length;
char _blank_character;


unsigned int od_edit_str(char *input_string, char *format_string, int row, int col, unsigned char normal_colour, unsigned char highlight_colour, char character, unsigned int flags)
   {
   register char temp_char;        /* Temporary variables used by od_edit_str() */
   register unsigned char counter;
   register unsigned char current_value;
   register char *string_pointer;
   long timer;

   register unsigned char cursor_pos;                /* Non-temporary variables */
   int keys_pressed=0;
   unsigned int to_return;
   char insert_mode=TRUE;
   char add_at_end=0;
   char normal=TRUE;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_edit_str()");

   if(!inited) od_init();                 /* verify that we've been initialized */


   _input_string=(char *)input_string;
   _format_string=(char *)format_string;


                              /* Check for validity of od_edit_str() parameters */
   if(_input_string==NULL || _format_string==NULL) return(EDIT_RETURN_ERROR);
   if(row<1 || col<1 ) return(EDIT_RETURN_ERROR);

   string_pointer=(char *)_format_string;    /* Set pointer to beginning of str */
   _string_length=0;                          /* Maximum length of input string */
   current_value=0;                 /* Type of literal string we are looking at */
   counter=0;                           /* Counter of position in format string */

   while(*string_pointer)                       /* Loop to end of format string */
      {
      temp_char=*string_pointer++;     /* Get next character from format string */

      if(current_value==0)      /* If current character is not a literal string */
         {
         if(temp_char==39 || temp_char==34)             /* If " or ' characters */
            {
            current_value=temp_char;             /* Beginning of literal string */
            }
         else if(temp_char!=32)                                   /* Otherwise, */
            {
            if(_string_length>=80) return(EDIT_RETURN_ERROR);
            _format_offset[_string_length]=counter;   /* Record char's position */
            _format_literal[_string_length]=FALSE;     /* Char is not a literal */
            ++_string_length;               /* Increment length of input string */
            }
         }

      else                             /* If we are looking at a literal string */
         {
         if(temp_char==current_value)        /* Check for end of literal string */
            {
            current_value=0;        /* If found, stop literal string processing */
            }
         else
            {
            if(_string_length>=80) return(EDIT_RETURN_ERROR);  /* Limit str len */
            _format_offset[_string_length]=counter;   /* Record char's position */
            _format_literal[_string_length]=TRUE;          /* Char is a literal */
            ++_string_length;               /* Increment length of input string */
            }
         }

      ++counter;                            /* Increment format string position */
      }

   if(_string_length==0) return(EDIT_RETURN_ERROR);/* Check input string length */

   if(flags&EDIT_FLAG_EDIT_STRING)                /* If editing existing string */
      {                                /* Check for valid existing input string */
      if(strlen(_input_string)>_string_length)
         {
         _input_string[_string_length]='\0';
         }
      cursor_pos=strlen(_input_string);          /* Set cursor to end of string */
      }

   else                                       /* If not editing existing string */
      {
      _input_string[0]='\0';                   /* Blank current string contents */
      cursor_pos=0;                        /* Set cursor to beginning of string */
      }

   strcpy(_original_string,_input_string);             /* Store original string */

   od_set_attrib(highlight_colour);              /* Set appropriate text colour */

                                       /* Determine appropriate blank character */
   _blank_character=(flags&EDIT_FLAG_PASSWORD_MODE) ? ' ' : character;

   if((flags&EDIT_FLAG_STRICT_INPUT) || (flags&EDIT_FLAG_PERMALITERAL)) insert_mode=FALSE;

   if(!(flags&EDIT_FLAG_NO_REDRAW))                /* If no-redraw flag not set */
      {
      od_set_cursor(row,col);                         /* Set to redraw position */
      if(flags&EDIT_FLAG_PASSWORD_MODE)                     /* If password mode */
         {
         od_repeat(character,strlen(_input_string)); /* Display blanked-out str */
         }
      else
         {
         od_disp_str(_input_string);                   /* Display actual string */
         }
      if(flags&EDIT_FLAG_PERMALITERAL)
         {
         display_permaliteral();
         }
      else
         {
         od_repeat(_blank_character,(_string_length-strlen(_input_string))+1);
         }
      }

   od_set_cursor(row,col+cursor_pos);     /* Set cursor at appropriate position */


   if(normal) goto keep_going;

   for(;;)
      {
      if(flags&EDIT_FLAG_AUTO_ENTER)              /* If "auto-enter" mode is on */
         {
         if(strlen(_input_string)==_string_length)          /* If end of string */
            {
            to_return=EDIT_RETURN_ACCEPT;        /* Try to accept current input */
            goto try_to_accept;                 /* Check whether input is valid */
            }
         }

keep_going:
      if((flags&EDIT_FLAG_PERMALITERAL) && (cursor_pos<_string_length))
         {
         if(_format_literal[cursor_pos])
            {
            if(cursor_pos<strlen(_input_string))
               {
               goto pressed_right_arrow;
               }
            temp_char=_format_string[_format_offset[cursor_pos]];
            ++keys_pressed;
            goto try_this_character;
            }
         }

get_another_key:

      temp_char=od_get_key(TRUE);                        /* Get a key from user */
      ++keys_pressed;                             /* Increment total keystrokes */

try_this_character:
      if(temp_char==27)                                    /* If char is ESCape */
         {
                                                       /* Wait 110 milliseconds */
         timer=_clock_tick();
         while(timer + 2L > _clock_tick() && _clock_tick() >= timer
               && b_head==b_tail)
            od_kernal();                              /* ...calling od_kernal() */

         if(b_head==b_tail)                  /* If no further chars are waiting */
            {                            /* Then it was the actual [ESCape] key */
            if(flags&EDIT_FLAG_ALLOW_CANCEL)        /* If cancel key is allowed */
               {
               strcpy(_input_string,_original_string);/* Reset input to old str */
               to_return=EDIT_RETURN_CANCEL;         /* Indicate user cancelled */
               goto exit_and_redraw; /* Exit, displaying change back to old str */
               }
            }

         else   /* If there are more chars after the ESC char, in less than .5s */
            {                       /* Then presume user did not press [ESCape] */
            temp_char=od_get_key(TRUE);  /* Get next character */

                              /* Make sure we are dealing with an ANSI sequence */
            if(temp_char!='[') goto try_this_character;

            switch(od_get_key(TRUE))          /* Get last char of ANSI sequence */
               {
               case 'A':                        /* If user pressed up arrow key */
pressed_up_arrow:
                  if(flags&EDIT_FLAG_FIELD_MODE)
                     {
                     to_return=EDIT_RETURN_PREVIOUS;
                     goto try_to_accept;
                     }
                  break;


               case 'B':                      /* If user pressed down arrow key */
pressed_down_arrow:
                  if(flags&EDIT_FLAG_FIELD_MODE)
                     {
                     to_return=EDIT_RETURN_NEXT;
                     goto try_to_accept;
                     }
                  break;


               case 'C':                     /* If user pressed right arrow key */
pressed_right_arrow:
                  if(cursor_pos<strlen(_input_string))         /* If not at end */
                     {
                     cursor_pos++;                         /* Move cursor right */
                     od_set_cursor(row,col+cursor_pos);/* Move cursor on screen */
                     }
                  if(add_at_end)
                     {
                     add_at_end=0;
                     goto add_another_key;
                     }
                  break;


               case 'D':                      /* If user pressed left arrow key */
pressed_left_arrow:
                  if(cursor_pos>0)                       /* If not at beginning */
                     {
                     cursor_pos--;                          /* Move cursor left */
                     od_set_cursor(row,col+cursor_pos);/* Move cursor on screen */
                     }
                  if(flags&EDIT_FLAG_PERMALITERAL && _format_literal[cursor_pos] && cursor_pos>0) goto pressed_left_arrow;
                  break;
               }
            }
         }


      else if(temp_char==0)                         /* If character was a zero, */
         {                   /* Then following char will be a Doorway character */
         temp_char=od_get_key(TRUE);               /* So get the next character */
         
         switch(temp_char)             /* Check second char of Doorway sequence */
            {
            case 0x48:                                           /* If up arrow */
               goto pressed_up_arrow;                  /* Goto up arrow handler */

            case 0x50:                                         /* If down arrow */
               goto pressed_down_arrow;              /* Goto down arrow handler */

            case 0x4b:                                         /* If left arrow */
               goto pressed_left_arrow;              /* Goto left arrow handler */

            case 0x4d:                                        /* If right arrow */
               goto pressed_right_arrow;            /* Goto right arrow handler */

            case 0x0f:                                      /* If shift-tab key */
               goto pressed_up_arrow;                  /* Goto up arrow handler */

            case 0x47:                                           /* If home key */
               if(cursor_pos!=0)                        /* If not at beginnning */
                  {
                  cursor_pos=0;                  /* Set string pos to beginning */
                  od_set_cursor(row,col);              /* Move cursor on screen */

                  }
               break;

            case 0x4f:                                            /* If end key */
               if(cursor_pos!=strlen(_input_string))           /* If not at end */
                  {
                  cursor_pos=strlen(_input_string);    /* Set string pos to end */
                  od_set_cursor(row,col+cursor_pos);   /* Move cursor on screen */
                  }
               break;

            case 0x53:                                         /* If Delete key */
pressed_delete:                                            /* If okay to delete */
               if(!(flags&EDIT_FLAG_STRICT_INPUT) && cursor_pos<strlen(_input_string) && !(flags&EDIT_FLAG_PERMALITERAL))
                  {                        /* Move remaining line, if any, back */
                  current_value=strlen(_input_string)-1;
                  for(counter=cursor_pos;counter<current_value;++counter)
                     {
                     od_putch(_input_string[counter]=_input_string[counter+1]);
                     }
                  _input_string[current_value]='\0';  /* Erase last char of str */
                  od_putch(_blank_character);            /* Blank old character */
                  od_set_cursor(row,col+cursor_pos);      /* Move to cursor pos */
                  goto check_cursor_char;           /* Update changes to string */
                  }
               break;

            case 0x52:                                         /* If Insert key */
pressed_insert:
               if(!(flags&EDIT_FLAG_STRICT_INPUT) && !(flags&EDIT_FLAG_PERMALITERAL))
                  {
                  insert_mode=!insert_mode;            /* Toggle insert setting */
                  }
               break;

            default:                                 /* If not Doorway sequence */
               goto try_this_character;       /* Treat it as a normal character */
            }
         }


      else if(temp_char==13 || temp_char==26)          /* If enter key / CTRL-Z */
         {
         to_return=EDIT_RETURN_ACCEPT;               /* User has accepted input */
         goto try_to_accept;                    /* Check whether input is valid */
         }

      else if(temp_char==127 || temp_char==07)                 /* If delete key */
         {
         goto pressed_delete;                        /* Goto delete key handler */
         }


      else if(temp_char==8)                                 /* If backspace key */
         {
backspace_again:
         if(cursor_pos>0)               /* If we are not at beginning of string */
            {
            if(flags&EDIT_FLAG_PERMALITERAL)
                  {  /* counter, current_value */
                  for(counter=0;counter<cursor_pos;++counter)
                     {
                     if(!_format_literal[counter]) goto continue_deletion;
                     }
                  goto get_another_key;
                  }
continue_deletion:

            if(cursor_pos==strlen(_input_string)) /* If we are at end of string */
               {
               _input_string[--cursor_pos]='\0';   /* Erase last char in string */
               if((flags&EDIT_FLAG_PERMALITERAL) && _format_literal[cursor_pos])
                  {
                  goto backspace_again;
                  }
               else
                  {
                  od_set_cursor(row,col+cursor_pos);     /* Move to new cursor pos */
                  od_putch(_blank_character);               /* Blank old character */
                  od_set_cursor(row,col+cursor_pos);   /* Move again to cursor pos */
                  }
               }
            else if(!(flags&EDIT_FLAG_STRICT_INPUT) && !(flags&EDIT_FLAG_PERMALITERAL))      /* If not strict mode */
               {
               --cursor_pos;                                /* Move cursor left */
               od_set_cursor(row,col+cursor_pos);     /* Move to new cursor pos */
               goto pressed_delete;
               }
            }
         }


      else if(temp_char==5)                        /* If previous field request */
         {
         goto pressed_up_arrow;                        /* Goto up arrow handler */
         }


      else if(temp_char==24 || temp_char==9)           /* If next field request */
         {
         goto pressed_down_arrow;                    /* Goto down arrow handler */
         }


      else if(temp_char==22)                                   /* If insert key */
         {
         goto pressed_insert;                        /* Call insert key handler */
         }


      else if(temp_char==19)                                     /* If left key */
         {
         goto pressed_left_arrow;                    /* Call left arrow handler */
         }


      else if(temp_char==4)                                     /* If right key */
         {
         goto pressed_right_arrow;                  /* Call right arrow handler */
         }


      else if(temp_char==25)                                    /* If Control-Y */
         {
         goto kill_whole_line;                 /* Erase entire contents of line */
         }

      else
         {
         if(keys_pressed==1 && (flags&EDIT_FLAG_AUTO_DELETE))  /* If autodelete */
            {
kill_whole_line:
            if(strlen(_input_string)!=0)              /* If string is not empty */
               {
               od_set_cursor(row,col);           /* Move to beginning of string */
               od_repeat(_blank_character,strlen(_input_string));/* Blank it out */
               }
            od_set_cursor(row,col);              /* Move to new cursor position */
            cursor_pos=0;                              /* Store cursor position */
            _input_string[0]='\0';                 /* Blank out string contents */
            }

add_another_key:
         if(!is_valid_char_for_pos(temp_char,cursor_pos))
            {                         /* If character is not a valid input char */
            if(_format_literal[cursor_pos])
               {
               if(cursor_pos<strlen(_input_string))
                  {
                  if(_input_string[cursor_pos]==_format_string[_format_offset[cursor_pos]])
                     {
                     add_at_end=temp_char;
                     goto pressed_right_arrow;
                     }
                  }
               add_at_end=temp_char;
               temp_char=_format_string[_format_offset[cursor_pos]];
               }
            else
               {
               continue;
               }
            }

                                /* Convert char to correct value, if applicable */
         temp_char=as_char_for_pos(temp_char,cursor_pos);

         if(cursor_pos>=strlen(_input_string))    /* If we are at end of string */
            {
            cursor_pos=strlen(_input_string); /* Reset original cursor position */
            if(cursor_pos<_string_length)     /* If there is room to add a char */
               {
               if(flags&EDIT_FLAG_PASSWORD_MODE)            /* If password mode */
                  {
                  od_putch(character);        /* Display the password character */
                  }
               else                                  /* If not in password mode */
                  {
                  od_putch(temp_char);                 /* Display the character */
                  }
               _input_string[cursor_pos]=temp_char;      /* Store the character */
               _input_string[++cursor_pos]='\0'; /* Add a new string terminator */
               }
            }


         else if(insert_mode)    /* If in insert mode, but not at end of string */
            {
            if(strlen(_input_string)<_string_length)       /* If room in string */
               {
               if(flags&EDIT_FLAG_PASSWORD_MODE)         /* If in password mode */
                  {
                  od_set_cursor(row,col+strlen(_input_string));  /* Move to end */
                  od_putch(character);             /* Add another password char */
                  }
               else                                  /* If not in password mode */
                  {
                  od_putch(temp_char);             /* Display the new character */
                  for(counter=cursor_pos;counter<strlen(_input_string);++counter)
                     {                           /* Loop through rest of string */
                     od_putch(_input_string[counter]);  /* Disp remaining chars */
                     }
                  }

               _input_string[(strlen(_input_string)+1)]='\0';  /* New str term */
               for(counter=strlen(_input_string);counter>cursor_pos;--counter)
                  {                     /* Shuffle remaining characters forward */
                  _input_string[counter]=_input_string[counter-1];
                  }
               _input_string[cursor_pos++]=temp_char;  /* Add new char in space */
               od_set_cursor(row,col+cursor_pos);     /* Move to new cursor pos */
               }
            else
               {
               goto get_another_key;
               }
            }


         else                 /* If in overwrite mode, but not at end of string */
            {
            if(flags&EDIT_FLAG_PASSWORD_MODE)               /* If password mode */
               {
               od_putch(character);           /* Display the password character */
               }
            else                                     /* If not in password mode */
               {
               od_putch(temp_char);                    /* Display the character */
               }
            _input_string[cursor_pos++]=temp_char;   /* Add character to string */
            }

         if(cursor_pos<_string_length)      /* If not at end of possible string */
            {
            if(_format_literal[cursor_pos])    /* If next char is literal const */
               {
               temp_char=_format_string[_format_offset[cursor_pos]];
               goto add_another_key;
               }
            }

         if(add_at_end)
            {
            temp_char=add_at_end;
            add_at_end=0;
            goto add_another_key;
            }


check_cursor_char:
         if(cursor_pos<strlen(_input_string))/* If there is a char under cursor */
            {                      /* If character corresponds to format string */
            if(is_valid_char_for_pos(_input_string[cursor_pos],cursor_pos))
               {                              /* Determine correct char for pos */
               temp_char=as_char_for_pos(_input_string[cursor_pos],cursor_pos);
                                          /* If actual character is not correct */
               if(temp_char!=_input_string[cursor_pos])
                  {                        /* Change character to correct value */
                  _input_string[cursor_pos]=temp_char;

                  if(flags&EDIT_FLAG_PASSWORD_MODE)         /* If password mode */
                     {
                     od_putch(character);     /* Display the password character */
                     }
                  else                               /* If not in password mode */
                     {
                     od_putch(temp_char);              /* Display the character */
                     }

                  od_set_cursor(row,col+cursor_pos);   /* Reset cursor position */
                  }
               }
            }

         }
      }



try_to_accept:                                        /* Accept string if valid */
   if(flags&EDIT_FLAG_FILL_STRING)                  /* If string must be filled */
      {                          /* If string is not filled, continue inputting */
      if(strlen(_input_string)!=_string_length) goto keep_going;
      }

   for(counter=0;counter<strlen(_input_string);++counter)/* Loop through string */
      {                                  /* Testing each character for validity */
      if(!is_valid_char_for_pos(_input_string[counter],counter)) goto keep_going;
      }

   current_value=FALSE;                          /* String has not been changed */
                                                         /* If string is valid, */
   for(counter=0;counter<strlen(_input_string);++counter)/* Loop through string */
      {                                /* Find correct value for each character */
      temp_char=as_char_for_pos(_input_string[counter],counter);
      if(temp_char!=_input_string[counter])           /* If char is not correct */
         {
         _input_string[counter]=temp_char;      /* Change char to correct value */
         current_value=TRUE;                   /* Indicate that str was changed */
         }
      }

   if(flags&EDIT_FLAG_LEAVE_BLANK)                      /* If permaliteral mode */
      {                                        /* Count # of literal characters */
      counter=0;
      while(counter<strlen(_input_string))
         if(_format_literal[counter]) ++counter; else break;
                                                  /* If only literals in string */
      if(strlen(_input_string)==counter && counter>0)
         {                                       /* Then they shouldn't be here */
         _input_string[0]='\0';
         goto exit_and_redraw;
         }
      }

   if(current_value) goto exit_and_redraw;  /* Always redraw if str was changed */


exit_od_edit_str:
   if(!(flags&EDIT_FLAG_NO_REDRAW))                /* If no-redraw flag not set */
      {
exit_and_redraw:
      od_set_attrib(normal_colour);              /* Set appropriate text colour */
      od_set_cursor(row,col);                         /* Set to redraw position */

      if(flags&EDIT_FLAG_PASSWORD_MODE)                     /* If password mode */
         {
         od_repeat(character,strlen(_input_string)); /* Display blanked-out str */
         }
      else
         {
         od_disp_str(_input_string);                   /* Display actual string */
         }

      if(flags&EDIT_FLAG_KEEP_BLANK)        /* If we should keep the background */
         {                                            /* Then redraw background */
         if(flags&EDIT_FLAG_PERMALITERAL)
            {
            display_permaliteral();
            }
         else
            {
            od_repeat(_blank_character,(_string_length-strlen(_input_string))+1);
            }
         }
      else                                 /* If we should erase the background */
         {                                         /* Then erase the background */
         od_repeat(' ',(_string_length-strlen(_input_string))+1);
         }
      }

   return(to_return);
   }



int is_valid_char_for_pos(char character, unsigned char pos)
   {
   if(_format_literal[pos])                   /* If this character is a literal */
      {                                     /* Check required literal character */
      if(character!=_format_string[_format_offset[pos]])
         {                                          /* If not correct character */
         return(FALSE);                                        /* Indicate this */
         }
      return(TRUE);
      }
               /* If this position has a corresponding format control character */
   switch(_format_string[_format_offset[pos]])            /* check control char */
      {
      case '#':                                          /* If only number char */
         if(character<'0' || character>'9') return(FALSE);
         break;

      case '%':                                           /* If number or space */
         if((character<'0' || character>'9') && character!=' ') return(FALSE);
         break;

      case '9':                                     /* If floating point number */
         if(character>='0' && character<='9') break;
         if(character=='.' || character=='+' || character=='-') break;
         return(FALSE);

      case '*':                                    /* If "printable" characters */
         if(character<32 || character>127) return(FALSE);
         break;

      case 'C':                                      /* If city name characters */
      case 'c':
         if(character>='A' && character<='Z') break;
         if(character>='a' && character<='z') break;
         if(character==' ' || character==',' || character=='.') break;
         if(character=='*' || character=='?') break;
         return(FALSE);

      case 'A':                                    /* If any of the alpha codes */
      case 'a':
      case 'L':
      case 'l':
      case 'M':
      case 'm':
      case 'U':
      case 'u':
         if(character>='A' && character<='Z') break;
         if(character>='a' && character<='z') break;
         if(character==' ') break;
         return(FALSE);

      case 'D':                                           /* If date characters */
      case 'd':
         if(character>='0' && character<='9') break;
         if(character=='-' || character=='/') break;
         return(FALSE);

      case 'F':                                       /* If filename characters */
      case 'f':
         if(character>='A' && character<='Z') break;
         if(character>='0' && character<='9') break;
         if(character>='a' && character<='z') break;
         switch(character)
            {
            case ':':                                    /* Filename separators */
            case '.':
            case '\\':
            
            case '?':                                    /* Wildcard characters */
            case '*':

            case '#':                        /* Other valid filename characters */
            case '$':
            case '&':
            case '\'':
            case '(':
            case '>':
            case '-':
            case '@':
            case '_':
            case '!':
            case '{':
            case '}':
            case '~':
               return(TRUE);
            }

         return(FALSE);

      case 'H':                                    /* If hexidecimal characters */
      case 'h':
         if(character>='0' && character<='9') break;
         if(character>='A' && character<='F') break;
         if(character>='a' && character<='f') break;
         return(FALSE);

      case 'T':                                          /* If telephone number */
      case 't':
         if(character>='0' && character<='9') break;
         if(character=='-' || character=='(' || character==')' || character==' ') break;
         return(FALSE);

      case 'W':                                   /* If filename with wildcards */
      case 'w':
         if(character>='A' && character<='Z') break;
         if(character>='a' && character<='z') break;
         if(character==':' || character=='.' || character=='\\' || character=='*' || character=='?') break;
         return(FALSE);

      case 'X':                                             /* If alpha numeric */
      case 'x':
         if(character>='A' && character<='Z') break;
         if(character>='a' && character<='z') break;
         if(character>='0' && character<='9') break;
         if(character==' ') break;
         return(FALSE);

      case 'Y':                                                    /* If Yes/No */
      case 'y':
         if(character=='y' || character=='n' || character=='Y' || character=='N') break;
         return(FALSE);
      }

   return(TRUE);                                                     /* Default */
   }



char as_char_for_pos(char character, unsigned char pos)
   {
   if(_format_literal[pos])                   /* If this character is a literal */
      {                                       /* Return only valid char for pos */
      return(_format_string[_format_offset[pos]]);
      }

               /* If this position has a corresponding format control character */
   switch(_format_string[_format_offset[pos]])                 /* check control char */
      {
      case 'Y':                                                    /* If Yes/No */
      case 'y':
         return(toupper(character));

      case 'F':                                       /* If filename characters */
      case 'f':
         return(toupper(character));

      case 'L':                                     /* If lower case characters */
      case 'l':
         return(tolower(character));

      case 'U':                                     /* If upper case characters */
      case 'u':
         return(toupper(character));

      case 'M':                                     /* If mixed case characters */
      case 'm':
      case 'C':
      case 'c':
         if(pos==0) return(toupper(character));   /* First char is always upper */
         if(_format_literal[pos-1]) return(toupper(character));
         if(toupper(_format_string[_format_offset[pos]])!='M') return(toupper(character));

             /* If previous char is a word delimiter, then this should be upper */
         if(_input_string[pos-1]==' ' || _input_string[pos-1]=='.' || _input_string[pos-1]==',' || _input_string[pos-1]=='-') return(toupper(character));
                                             /* Otherwise, this should be lower */
         return(tolower(character));
      }

   return(character);
   }



void display_permaliteral(void)
   {
   register unsigned char counter;
   register unsigned char repeat=0;

   for(counter=strlen(_input_string);counter<=_string_length;++counter)
      {
      if(counter!=_string_length)
         {
         if(_format_literal[counter])
            {
            if(repeat>0)
               {
               od_repeat(_blank_character,repeat);
               repeat=0;
               }
            od_putch(_format_string[_format_offset[counter]]);
            }
         else
            {
            ++repeat;
            }
         }
      else
         {
         ++repeat;
         }
      }

   if(repeat>0) od_repeat(_blank_character,repeat);
   }
