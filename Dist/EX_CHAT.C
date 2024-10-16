/* EX_CHAT.C - Example of a multi-window full-screen chat door written with
 *             OpenDoors. Demonstrates the ease of using sophisticated ANSI /
 *             AVATAR terminal features within OpenDoors programs. See manual
 *             for instructions on how to compile this program or any other
 *             OpenDoors program.
 *
 *             This program create two windows on the screen, seperated by
 *             a bar with user name / sysop name information. This program
 *             permits communication between the local sysop and remote user
 *             by displaying the text typed by the user in one window, and
 *             the text typed by the sysop in the other window. When either
 *             user reaches the bottom of the window, the contents of the
 *             window is scrolled up to provide more room for typing. Words
 *             are also wrapped when either typist reaches the end of a
 *             line. The advantage of a split-screen chat program is that
 *             it permits both sysop and user to type at the same time
 *             without difficulty. The chat function automatically invokes
 *             OpenDoor's internal chat mode if ANSI or AVATAR modes are not
 *             available. The display colours, window sizes and locations,
 *             and distance to scroll a window's contents are configurable
 *             by setting the appropriate variables, below. When the Sysop
 *             invokes a DOS shell, a pop-up window is displayed to indicate
 *             to the user that the door program has been suspended.
 */

#include "opendoor.h"                        /* Include required header files */
#include <string.h>


void fullscreen_chat(void);           /* Full-screen chat function prototypes */
void chat_new_line(void);
void display_shell_window(void);
void remove_shell_window(void);



main()                                     /* PROGRAM'S EXECUTION BEGINS HERE */
   {
   od_init();                                         /* Initialize OpenDoors */

   fullscreen_chat();                     /* Invoke full-screen chat function */

   return(0);                                                    /* Exit door */
   }





                                    /* FULL-SCREEN CHAT CUSTOMIZATION OPTIONS */

char window_colour[2]={0x0b,0x0c};        /* Text colour used for each person */
char bar_colour=0x70;                      /* Colour of window seperation bar */
char top_line[2]={1,13};       /* Specifies location of each window on screen */
char bottom_line[2]={11,23};          /* Line number of bottom of each window */
char bar_line=12;                     /* Line number of window seperation bar */
char scroll_distance=2;            /* Distance to scroll window when required */
char shell_window_title=0x1a;   /* Colour of title of DOS shell notice window */
char shell_window_boarder=0x1f;         /* Colour of DOS shell window boarder */
char shell_window_text=0x1b;            /* Colour of text in DOS shell window */



char cursor_window;                     /* FULL-SCREEN CHAT INTERNAL VARIABLES */
char current_word[2][81];
int word_length[2];
int cursor_col[2];
int cursor_line[2];
unsigned char key;
int old_chat_key;
void *shell_window;
char *before_shell_text;
char *after_shell_text;
                                                 /* FULL-SCREEN CHAT FUNCTION */
void fullscreen_chat(void)
   {
   cursor_window=0;                                /* Reset working variables */
   word_length[0]=word_length[1]=0;
   cursor_col[0]=cursor_col[1]=1;
   cursor_line[0]=top_line[0];
   cursor_line[1]=top_line[1];


                          /* If ANSI or AVATAR graphics mode is not available */
   if(!od_control.user_ansi && !od_control.user_avatar)
      {
      od_chat();                /* Then use OpenDoor's line chat mode instead */
      return;
      }

   od_control.od_cbefore_shell=display_shell_window;/* Set DOS shell settings */
   od_control.od_cafter_shell=remove_shell_window;
   before_shell_text=od_control.od_before_shell;
   after_shell_text=od_control.od_after_shell;
   od_control.od_before_shell=NULL;
   od_control.od_after_shell=NULL;

   old_chat_key=od_control.key_chat;       /* Prevent internal chat mode from */
   od_control.key_chat=0;                                    /* being invoked */

                                                      /* DRAW THE CHAT SCREEN */
   od_set_attrib(window_colour[0]);
   od_clr_scr();                                          /* Clear the screen */

   od_set_cursor(bar_line,1);                   /* Draw window seperation bar */
   od_set_attrib(bar_colour);
   od_clr_line();
   od_printf(" Top : %s    Bottom : %s",od_control.user_name,
                                        od_control.sysop_name);

   od_set_cursor(top_line[0],1);     /* Locate cursor where typing will begin */
   od_set_attrib(window_colour[0]);            /* Set appropriate text colour */

                                                            /* MAIN CHAT LOOP */
   for(;;)                              /* (Repeats for each character typed) */
      {
      key=(char)od_get_key(TRUE);         /* Get next keystroke from keyboard */


                                                     /* CHECK FOR SYSOP ABORT */
      if(key==27 && od_control.od_last_input==1)    /* If sysop pressed [ESC] */
         {
         od_set_attrib(0x07);                         /* Reset display colour */
         od_clr_scr();                                    /* Clear the screen */
         od_control.key_chat=old_chat_key;    /* Re-enable internal chat mode */

         od_control.od_cbefore_shell=NULL;      /* Restore DOS shell settings */
         od_control.od_cafter_shell=NULL;
         od_control.od_before_shell=before_shell_text;
         od_control.od_after_shell=after_shell_text;

         return;                                     /* Exit full-screen chat */
         }

                                                      /* CHECK FOR NEW TYPIST */
      if(od_control.od_last_input!=cursor_window) /* If new person typing now */
         {                             /* Switch cursor to appropriate window */
         cursor_window=od_control.od_last_input;        /* Set current typist */

                                                 /* Move cursor to new window */
         od_set_cursor(cursor_line[cursor_window],cursor_col[cursor_window]);

         od_set_attrib(window_colour[cursor_window]);   /* Change text colour */
         }


      if(key==13 || key==10)            /* IF USER PRESSED [ENTER] / [RETURN] */
         {
         word_length[cursor_window]=0;       /* Enter constitutes end of word */

         chat_new_line();                                /* Move to next line */
         }


      else if(key==8)                            /* IF USER PRESS [BACKSPACE] */
         {
         if(cursor_col[cursor_window] > 1)        /* If not at left of screen */
            {
            --cursor_col[cursor_window];     /* Move cursor back on character */
            if(word_length[cursor_window] > 0) --word_length[cursor_window];
            od_printf("\b \b");           /* Erase last character from screen */
            }
         }


      else if(key==32)                             /* IF USER PRESSED [SPACE] */
         {
         word_length[cursor_window]=0;     /* [Space] constitutes end of word */

         if(cursor_col[cursor_window]==79)               /* If at end of line */
            chat_new_line();                      /* Move cursor to next line */
         else                                        /* If not at end of line */
            {
            ++cursor_col[cursor_window];         /* Increment cursor position */
            od_putch(32);                                  /* Display a space */
            }
         }


      else if(key>32)                  /* IF USER TYPED A PRINTABLE CHARACTER */
         {                                  /* PERFORM WORD WRAP IF NECESSARY */
         if(cursor_col[cursor_window]==79)     /* If cursor is at end of line */
            {
                                                /* If there is a word to wrap */
            if(word_length[cursor_window]>0 && word_length[cursor_window]<78)
               {
                                          /* Move cursor to beginning of word */
               od_set_cursor(cursor_line[cursor_window],
                          cursor_col[cursor_window]-word_length[cursor_window]);

               od_clr_line();                 /* Erase word from current line */

               chat_new_line();                   /* Move cursor to next line */

                                                            /* Redisplay word */
               od_disp(current_word[cursor_window],word_length[cursor_window],
                                                                          TRUE);
               cursor_col[cursor_window]+=word_length[cursor_window];
               }

            else                             /* If there is no word to "wrap" */
               {
               chat_new_line();                   /* Move cursor to next line */
               word_length[cursor_window]=0;              /* Start a new word */
               }
            }

                                             /* ADD CHARACTER TO CURRENT WORD */
                               /* If there is room for more character in word */
         if(strlen(current_word[cursor_window])<79)      /* Add new character */
            current_word[cursor_window][word_length[cursor_window]++]=key;

                                             /* DISPLAY NEWLY TYPED CHARACTER */
         ++cursor_col[cursor_window];
         od_putch(key);
         }
      }
   }



               /* FUNCTION USED BY FULL-SCREEN CHAT TO START A NEW INPUT LINE */
void chat_new_line(void)
   {                                      /* If cursor is at bottom of window */
   if(cursor_line[cursor_window]==bottom_line[cursor_window])
      {                                /* Scroll window up one line on screen */
      od_scroll(1,top_line[cursor_window],79, bottom_line[cursor_window],
                scroll_distance, 0);
      cursor_line[cursor_window]-=(scroll_distance - 1);
      }

   else                               /* If cursor is not at bottom of window */
      {
      ++cursor_line[cursor_window];              /* Move cursor down one line */
      }

                                          /* Move cursor's position on screen */
   od_set_cursor(cursor_line[cursor_window],cursor_col[cursor_window]=1);

   od_set_attrib(window_colour[cursor_window]);         /* Change text colour */
   }


void display_shell_window(void)
   {
   if((shell_window=od_window_create(17,9,63,15,"DOS Shell",
                                     shell_window_boarder, shell_window_title, 
                                     shell_window_text, 0))==NULL) return;

   od_set_attrib(shell_window_text);
   od_set_cursor(11,26);
   od_printf("The Sysop has shelled to DOS");
   od_set_cursor(13,21);
   od_printf("He/She will return in a few moments...");
   }


void remove_shell_window(void)
   {
   od_window_remove(shell_window);
   od_set_cursor(cursor_line[cursor_window],cursor_col[cursor_window]);
   od_set_attrib(window_colour[cursor_window]);
   }
