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
 *     Filename : ODSCRN.C
 *  Description : Contains the code for writing to the local screen in the
 *                MS-DOS environment. Note that although many of these routines
 *                duplicate the functionality and use similar names to many
 *                of the Turbo C(++)/Borland C++ display routines, their
 *                behaviour is not identical. In particular, these routines
 *                do not perform all of the error checking that is done by
 *                the equivalent Borland routines.
 *      Version : 5.00
 */



/* Include OpenDoors header files */
#include "opendoor.h"
#include "odintern.h"

/* Include required standard library header files */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef USEREPLACEMENTS

/* Global variables used by the screen I/O functions */
unsigned int phys_seg;                    /* Segment address of video buffer */
void far *phys_buffer;                        /* Far pointer to video buffer */
unsigned char phys_curx, phys_cury;               /* Current cursor position */
unsigned char phys_winleft, phys_wintop, phys_winright, phys_winbottom;
unsigned char phys_attrib;                         /* Current display colour */
unsigned char phys_scroll;                           /* Is scrolling enabled */
unsigned char phys_page;                              /* Display page to use */
unsigned char phys_curon;                                    /* Is cursor on */


/* phys_init() - Initializes OpenDoors internal display routines */
void phys_init(void)
   {
   unsigned char mode;
   char clear;

   /* Get current video mode */
#ifdef USEINLINE
   ASM    push si
   ASM    push di
   ASM    mov ah, 0x0f
   ASM    int 0x10
   ASM    mov mode, al
   ASM    pop di
   ASM    pop si
#else
   regs.h.ah = 0x0f;
   int86(0x10, &regs, &regs);
   mode = regs.h.al;
#endif

   switch(mode&0x7f)
      {
      case 0x02:             /* No need to change mode, already colour 80x25 */
      case 0x03:
         phys_seg = 0xb800;
         phys_buffer = (void far *)0xb8000000L;
         clear = TRUE;
         break;

      case 0x07:         /* No need to change mode, already monochrome 80x25 */
         phys_seg = 0xb000;
         phys_buffer = (void far *)0xb0000000L;
         clear = TRUE;
         break;

      case 0x21:                     /* Must change mode to monochrome 80x25 */
         phys_seg = 0xb000;
         phys_buffer = (void far *)0xb0000000L;
         clear = FALSE;
         /* set mode to 0x07 */
#ifdef USEINLINE
         ASM    push si
         ASM    push di
         ASM    mov ax, 0x0007
         ASM    int 0x10
         ASM    pop di
         ASM    pop si
#else
         regs.x.ax = 0x0007;
         int86(0x10, &regs, &regs);
#endif
         break;

      default:                           /* Must change mode to colour 80x25 */
         phys_seg = 0xb800;
         phys_buffer = (void far *)0xb8000000L;
         clear = FALSE;
         /* set mode to 0x03 */
#ifdef USEINLINE
         ASM    push si
         ASM    push di
         ASM    mov ax, 0x0003
         ASM    int 0x10
         ASM    pop di
         ASM    pop si
#else
         regs.x.ax = 0x0003;
         int86(0x10, &regs, &regs);
#endif
      }



   /* Adjust address for display page which is being used */
#ifdef USEINLINE
   ASM    push si
   ASM    push di
   ASM    mov ah, 0x0f
   ASM    int 0x10
   ASM    mov phys_page, bh
   ASM    pop di
   ASM    pop si
#else
   regs.h.ah = 0x0f;
   int86(0x10, &regs, &regs);
   phys_page = regs.h.bh;
#endif

   if(phys_page!=0)
      {
      phys_seg += (250 * phys_page);
      ((char far *)phys_buffer) += (4000 * phys_page);
      }

   if(_multitasker == MULTITASKER_DV)
      {                                       /* determine address of DV screen buffer */
#ifdef USEINLINE
      /* This doesn't check rows, bh = rows, bl = columns */
      ASM    mov ax, 0x2b02
      ASM    mov cx, 0x4445
      ASM    mov dx, 0x5351
      ASM    int 0x21
      ASM    cmp bx, 0x1950
      ASM    jne no_change
      ASM    mov phys_seg, dx

/*
      ASM    push es
      ASM    push di
      ASM    mov ax, phys_seg
      ASM    mov es, ax
      ASM    mov ah, 0xfe
      ASM    int 0x10
      ASM    mov ax, es
      ASM    pop di
      ASM    pop es
      ASM    mov phys_seg, ax
*/
#else
      __emit__(0x57);                  /* PUSH ES */
      __emit__(0x06);                  /* PUSH DI */

      _ES=phys_seg;
      _AH=0xfe;
      geninterrupt(0x10);
      _AX=_ES;

      __emit__(0x07);                  /* POP DI */
      __emit__(0x5f);                  /* POP ES */

      phys_seg = _AX;
#endif

/*
      ((int *)phys_buffer)[0] = 0;
      ((int *)phys_buffer)[1] = phys_seg;
*/

      (long) phys_buffer = long_shift_left((long)phys_seg , 16 );
no_change: ;
      }


   /* Initialize display system variables */
   phys_winleft = 0;
   phys_winright = 79;
   phys_wintop = 0;
   phys_winbottom = 24;
   phys_attrib = 0x07;
   phys_scroll = 1;

   /* Clear local screen */
   if(clear)
      {
      phys_clrscr();
      }

   /* Enable flashing cursor */
   phys_curon=FALSE;
   phys_cursor(TRUE);
   }


/* phys_window() - Sets the current window boundaries */
void phys_window(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
   {
   /* Set internal window location variables */
   phys_winleft = x1 - 1;
   phys_winright = x2 - 1;
   phys_wintop = y1 - 1;
   phys_winbottom = y2 - 1;

   /* Ensure that the cursor is located within the new window boundaries */
   if(phys_curx > phys_winright - phys_winleft)
      {
      phys_curx = phys_winright - phys_winleft;
      }
   else if(phys_curx < phys_winleft)
      {
      phys_curx = phys_winleft;
      }

   if(phys_cury > phys_winbottom - phys_wintop)
      {
      phys_cury = phys_winbottom - phys_wintop;
      }
   else if(phys_cury < phys_wintop)
      {
      phys_cury = phys_wintop;
      }

   /* Execute the position flashing cursor primitive */
   phys_update_cursor();
   }


/* phys_gotoxy() - Locates the flashing cursor within the current window */
void phys_gotoxy(unsigned char x, unsigned char y)
   {
   /* Set internal cursor position values */
   phys_curx = x - 1;
   phys_cury = y - 1;

   /* Ensure that cursor falls within the current output window */
   if(phys_curx > phys_winright - phys_winleft)
      phys_curx = phys_winright - phys_winleft;

   if(phys_cury > phys_winbottom - phys_wintop)
      phys_cury = phys_winbottom - phys_wintop;

   /* Execute the position flashing cursor primitive */
   phys_update_cursor();
   }


/* phys_setattrib() - Sets the current display colour attribute */
void phys_setattrib(unsigned char attrib)
   {
   /* Set internal display colour attribute */
   phys_attrib = attrib;
   }


/* phys_setscroll() - Enables / disables scrolling of the window */
void phys_setscroll(unsigned char enabled)
   {
   /* Stores the current scrolling setting */
   phys_scroll = enabled;
   }


/* phys_cursor() - Enables / disables the local flashing cursor */
void phys_cursor(unsigned char enabled)
   {
#ifndef USEINLINE
   char cursor_cx;
#endif

   if(phys_curon == enabled) return;

   phys_curon = enabled;

   /* Execute the cursor on / off primitive */
#ifdef USEINLINE
   ASM    push si
   ASM    push di
   ASM    mov ah, 0x03
   ASM    mov bh, phys_page
   ASM    int 0x10
   ASM    push cx                /* ch = start line, cl = end line */
   ASM    mov ah, 0x0f
   ASM    int 0x10
   ASM    pop cx
   ASM    push ax                /* al = video mode */
   ASM    and ch, 0x1f
   ASM    mov al, phys_curon
   ASM    and al, al
   ASM    jnz set_cursor
   ASM    or ch, 0x20            /* ch bits 5-6 = blink attr */
set_cursor:                      /*               00 = normal */
   ASM    pop ax                 /*               01 = invisible */
   ASM    mov bh, phys_page      /* not required */
   ASM    mov ah, 0x01
   ASM    int 0x10
   ASM    pop di
   ASM    pop si
#else
   regs.h.ah = 0x03;
   regs.h.bh = phys_page;
   int86(0x10, &regs, &regs);
   cursor_cx = regs.x.cx;
   regs.h.ah = 0x0f;
   int86(0x10, &regs, &regs);
   regs.x.cx = cursor_cx;
   regs.h.ch &= 0x1f;
   regs.h.ah = phys_page;
   if(!phys_curon) regs.h.ch |= 0x20;
   regs.h.ah = 0x01;
   int86(0x10);
#endif

   if(phys_curon)
      {
      phys_update_cursor();
      }
   else
      {
#ifdef USEINLINE
      ASM    mov ah, 0x02
      ASM    mov bh, phys_page
      ASM    mov dh, 25
      ASM    mov dl, 80
      ASM    push si
      ASM    push di
      ASM    int 0x10
      ASM    pop di
      ASM    pop si
#else
      regs.h.ah = 0x02;
      regs.h.bh = phys_page;
      regs.h.dh = 25;
      regs.l.dl = 80;
      int86(0x10, &regs, &regs);
#endif

      }
   }


/* phys_gettextinfo() - Retrieves current display system status */
void phys_gettextinfo(struct phys_text_info *info)
   {
   info->wintop = phys_wintop + 1;
   info->winleft = phys_winleft + 1;
   info->winright = phys_winright + 1;
   info->winbottom = phys_winbottom + 1;
   info->attribute = phys_attrib;
   info->curx = phys_curx + 1;
   info->cury = phys_cury + 1;
   }


/* phys_cprintf() - Performs the printf operation to the local screen */
int phys_cprintf(char *format, ...)
   {
   va_list arg_pointer;
   int to_return;
   static char buf[81];

   /* Generate string to display */
   va_start(arg_pointer,format);
   to_return = vsprintf(buf,format,arg_pointer);
   va_end(arg_pointer);

   ASSERT(strlen(buf) <= 80);

   /* Display generated string */
   phys_cputs(buf);

   /* Return appropriate value */
   return (to_return);
   }


/* phys_putch() - Prints a single character to the local screen */
void phys_putch(char character)
   {
   register unsigned char far *dest;
   
   phys_getcursor();

   if(phys_curx > phys_winright - phys_winleft)
      {
      phys_curx = phys_winright - phys_winleft;
      }

   if(phys_cury > phys_winbottom - phys_wintop)
      {
      phys_cury = phys_winbottom - phys_wintop;
      }

   switch(character)
      {
      /* If character is a carriage return */
      case '\r':
         phys_curx = 0;
         break;

      /* If character is a line feed */
      case '\n':
         /* If cursor is at bottom of output window */
         if(phys_cury == phys_winbottom - phys_wintop)
            {
            /* If scrolling is enabled */
            if(phys_scroll)
               {
               /* Execute the scroll primitive */
               phys_scroll_primitive();
               }
            }
         /* If cursor is not at bottom of output window */
         else
            {
            /* Move the cursor down one line */
            ++phys_cury;
            }
         break;

      case '\b':
         /* If backspace */
         if(phys_curx != 0) --phys_curx;
         break;

      case '\a':
         /* If bell */
#ifdef USEINLINE
        ASM    mov ah, 0x02
        ASM    mov dl, 7
        ASM    int 0x21
#else
        regs.h.ah = 0x02;
        regs.l.dl = 7;
        int86(0x21,&regs,&regs);
#endif
         break;

      /* If character is not a control character */
      default:
         /* Output character to display buffer */
         dest = (unsigned char far *)phys_buffer + ((phys_wintop + phys_cury) * 160 + (phys_winleft + phys_curx) * 2);
         *dest++ = character;
         *dest = phys_attrib;

         ASSERT(dest >= (unsigned char far *)phys_buffer && dest < (unsigned char far *)phys_buffer + 4000);

         /* Advance cursor. If at end of line ... */
         if(++phys_curx > phys_winright - phys_winleft)
            {
            /* Wrap cursor if necessary */
            phys_curx = 0;

            /* If moving cursor down one line advances past end of window */
            if(++phys_cury > phys_winbottom - phys_wintop)
               {
               /* Move cursor back to bottom line of window */
               phys_cury = phys_winbottom - phys_wintop;
               /* If scrolling is enabled */
               if(phys_scroll)
                  {
                  /* Execute the scroll primitive */
                  phys_scroll_primitive();
                  }
               }
            }
      }

   /* Execute the update flashing cursor primitive */
   phys_update_cursor();
   }


/* phys_getcursor() - Called by other display functions to get current */
/*                    position of flashing cursor on screen            */
void phys_getcursor(void)
   {
   if(!phys_curon) return;

#ifdef USEINLINE
   ASM    mov ah, 0x03
   ASM    mov bh, phys_page
   ASM    push si
   ASM    push di
   ASM    int 0x10
   ASM    pop di
   ASM    pop si
   ASM    sub dh, phys_wintop
   ASM    mov phys_cury, dh
   ASM    sub dl, phys_winleft
   ASM    mov phys_curx, dl
#else
   regs.h.ah = 0x02;
   regs.h.bh = phys_page;
   int86(0x10, &regs, &regs);
   phys_cury = regs.h.dh;
   phsy_curx = regs.l.dl;
#endif
   }


/* phys_update_cursor() - Called by other display functions to update */
/*                        location of flashing cursor on screen       */
void phys_update_cursor(void)
   {
   if(!phys_curon) return;

   /* Update position of flashing cursor on screen */
#ifdef USEINLINE
   ASM    mov ah, 0x02
   ASM    mov bh, phys_page
   ASM    mov dh, phys_cury
   ASM    add dh, phys_wintop
   ASM    mov dl, phys_curx
   ASM    add dl, phys_winleft
   ASM    push si
   ASM    push di
   ASM    int 0x10
   ASM    pop di
   ASM    pop si
#else
   regs.h.ah = 0x02;
   regs.h.bh = phys_page;
   regs.h.dh = phys_cury;
   regs.l.dl = phsy_curx;
   int86(0x10, &regs, &regs);
#endif
   }


/* phys_clrscr() - Clears the contents of the current window */
void phys_clrscr(void)
   {
   register unsigned int far *addr=(unsigned int far *)phys_buffer + ((phys_wintop * 80) + phys_winleft);
   register unsigned int value = (((unsigned int)phys_attrib) << 8) | 32;
   register unsigned char col_counter;
   register unsigned char line_counter = (phys_winbottom - phys_wintop) + 1;
   register unsigned char col_start = (phys_winright - phys_winleft) + 1;
   register unsigned char skip = 80 - col_start;

   /* Clear contents of current window */
   do {
       col_counter = col_start;
       do {
           ASSERT(addr >= (unsigned int far *)phys_buffer && addr <= (unsigned int far *)phys_buffer+2000);
           *(addr++)=value;
           } while ((--col_counter) != 0);
       addr+=skip;
       } while((--line_counter) != 0);

   /* Move cursor to top left-hand corner of current window */
   phys_curx = phys_cury = 0;

   /* Execute the update flashing cursor primitive */
   phys_update_cursor();
   }


/* phys_scroll_primitive() - Called by other display functions to scroll */
/*                           the contents of the current window up one   */
/*                           line                                        */
void phys_scroll_primitive(void)
   {
   register unsigned int far *dest_addr=(unsigned int far *)phys_buffer + (phys_wintop*80 + phys_winleft);
   register unsigned int far *source_addr;
   register unsigned char col_counter;
   register unsigned char line_counter = phys_winbottom - phys_wintop;
   register unsigned char col_start = phys_winright - phys_winleft + 1;
   register unsigned char skip = 80 - col_start;
   register unsigned int value = (((unsigned int)phys_attrib) << 8) | 32;

   source_addr = dest_addr + 80;

   ASSERT(skip >= 0 && skip <= 80);

   /* Move text in area of window up one line */
   do {
      col_counter = col_start;
      do {
         ASSERT(dest_addr >= (unsigned int far *)phys_buffer && dest_addr <= (unsigned int far *)phys_buffer+2000);
         ASSERT(source_addr >= (unsigned int far *)phys_buffer && source_addr <= (unsigned int far *)phys_buffer+2000);
         *(dest_addr++) = *(source_addr++);
      } while((--col_counter) != 0);
      dest_addr+=skip;
      source_addr+=skip;
   } while ((--line_counter)!=0);

   /* Clear newly created line at bottom of window */
   col_counter = col_start;
   do {
      ASSERT(dest_addr >= (unsigned int far *)phys_buffer && dest_addr <= (unsigned int far *)phys_buffer+2000);
      *(dest_addr++) = value;
      } while((--col_counter)!=0);
   }


/* phys_gettext() - Stores a block of text from the current window into */
/*                  the buffer specified by "buffer" */
int phys_gettext(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, void *buffer)
   {
   register unsigned int *buf = (unsigned int *)buffer;
   register unsigned int far *addr = (unsigned int far *)phys_buffer + ((((--y1) + phys_wintop) * 80) + phys_winleft + (--x1));
   register unsigned char col_counter;
   register unsigned char line_counter = (--y2) - y1 + 1;
   register unsigned char col_start = (--x2) - x1 + 1;
   register unsigned char skip = 80 - col_start;

   ASSERT(x1 >= 0);
   ASSERT(y1 >= 0);
   ASSERT(x2 <= phys_winright - phys_winleft);
   ASSERT(y2 <= phys_winbottom - phys_wintop);
   ASSERT(buffer);

   /* Copy contents of screen block to buffer */
   do {
      col_counter = col_start;
      do {
         ASSERT(addr >= (unsigned int far *)phys_buffer && addr <= (unsigned int far *)phys_buffer+2000);
         ASSERT(buf >= (unsigned int *)buffer && buf <= (unsigned int *)buffer+2000);
         *(buf++) = *(addr++);
      } while ((--col_counter)!=0);
      addr += skip;
   } while((--line_counter)!=0);

   return(TRUE);
   }


/* phys_puttext() - Copys a block of text in the buffer specified by */
/*                  "buffer" to the current window                   */
int phys_puttext(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, void *buffer)
   {
   register unsigned int *buf = (unsigned int *)buffer;
   register unsigned int far *addr = (unsigned int far *)phys_buffer + ((((--y1) + phys_wintop) * 80) + phys_winleft + (--x1));
   register unsigned char col_counter;
   register unsigned char line_counter = (--y2) - y1 + 1;
   register unsigned char col_start = (--x2) - x1 + 1;
   register unsigned char skip = 80 - col_start;

   ASSERT(x1 >= 0 && y1 >= 0 && x1 <= phys_winright - phys_winleft && y1 <= phys_winbottom - phys_wintop);
   ASSERT(x2 >= 0 && y2 >= 0 && x2 <= phys_winright - phys_winleft && y2 <= phys_winbottom - phys_wintop);
   ASSERT(buffer);

   /* Copy contents of screen block to buffer */
   do {
      col_counter = col_start;
      do {
         ASSERT(addr >= (unsigned int far *)phys_buffer && addr <= (unsigned int far *)phys_buffer+2000);
         ASSERT(buf >= (unsigned int *)buffer && buf <= (unsigned int *)buffer+2000);
         *(addr++) = *(buf++);
      } while ((--col_counter)!=0);
      addr += skip;
   } while((--line_counter)!=0);

   return(TRUE);
   }


/* phys_cputs() - Outputs a null-terminated string to the screen */
int phys_cputs(char *string)
   {
   register char *str = string;
   register char far *addr;
   register char col_left;
   register char attr = phys_attrib;
   register char curx;
   char bottom = phys_winbottom - phys_wintop;

   ASSERT(string);

   phys_getcursor();

   if(phys_curx > phys_winright - phys_winleft)
      {
      phys_curx = phys_winright - phys_winleft;
      }

   if(phys_cury > phys_winbottom - phys_wintop)
      {
      phys_cury = phys_winbottom - phys_wintop;
      }

   curx = phys_curx;

   col_left = phys_winright - (curx + phys_winleft);
   addr = (char far *) phys_buffer + (((phys_wintop + phys_cury) * 160) + (phys_winleft + phys_curx) * 2);

   for(;;)
      {
      ASSERT(addr >= (char far *)phys_buffer && addr <= (char far *)phys_buffer+4000);
      switch(*str)
         {
         case '\0':
            goto finished;
         case '\r':
            curx = 0;
            col_left = phys_winright - phys_winleft;
            addr = (char far *)phys_buffer + ((phys_wintop + phys_cury) * 160 + phys_winleft * 2);
            str++;
            break;
         case '\n':
            if (phys_cury < bottom)
               {
               ++phys_cury;
               addr+=160;
               }
            else if(phys_scroll)
               {
               phys_scroll_primitive();
               }
            str++;
            break;

      case '\a':
            /* If bell */
#ifdef USEINLINE
            ASM    mov ah, 0x02
            ASM    mov dl, 7
            ASM    int 0x21
#else
            regs.h.ah = 0x02;
            regs.l.dl = 7;
            int86(0x21,&regs,&regs);
#endif
            str++;
            break;

         case '\b':
            if(curx > 0)
               {
               --curx;
               addr-=2;
               col_left++;
               }
            str++;
            break;

         default:
            *(addr++) = *(str++);
            *(addr++) = attr;
            if(col_left--)
               {
               ++curx;
               }
            else
               {
               curx = 0;
               col_left = phys_winright - phys_winleft;

               if(phys_cury < bottom)
                  {
                  ++phys_cury;
                  }
               else if(phys_scroll)
                  {
                  phys_scroll_primitive();
                  }
               addr = (char far *)phys_buffer + ((phys_wintop+phys_cury)*160 + phys_winleft*2);
               }
         }
      }

   finished:
      phys_curx = curx;
      phys_update_cursor();

      return(0);
   }


/* phys_movetext() - moves a block of the screen to another location */
int phys_movetext(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char xdest, unsigned char ydest)
   {
   void *buffer;

   ASSERT(x1 >= 0 && y1 >= 0 && x1 <= phys_winright - phys_winleft && y1 <= phys_winbottom - phys_wintop);
   ASSERT(x2 >= 0 && y2 >= 0 && x2 <= phys_winright - phys_winleft && y2 <= phys_winbottom - phys_wintop);
   ASSERT(xdest >= 0 && ydest >= 0 && xdest <= phys_winright - phys_winleft && ydest <= phys_winbottom - phys_wintop);

   if(   !(x1 >= 0 && y1 >= 0 && x1 <= phys_winright - phys_winleft && y1 <= phys_winbottom - phys_wintop)
      || !(x2 >= 0 && y2 >= 0 && x2 <= phys_winright - phys_winleft && y2 <= phys_winbottom - phys_wintop)
      || !(xdest >= 0 && ydest >= 0 && xdest <= phys_winright - phys_winleft && ydest <= phys_winbottom - phys_wintop))
      {
      return(FALSE);
      }


   if((buffer = malloc((x2 - x1 + 1) * (y2 - y1 +1) * 2)) == NULL) return (FALSE);

   phys_gettext(x1, y1, x2, y2, buffer);
   phys_puttext(xdest, ydest, x2 + (xdest - x1), y2 + (ydest - y1), buffer);
   free(buffer);

   return(TRUE);
   }
#endif


/* phys_createwin() - creates a local-only popup window on the screen */
void *phys_createwin(int x1, int y1, int x2, int y2, char attr, char *title, char titleattr)
   {
   void *under;
   int between;
   int count;
   int first;
   static char tempstr[81];
   char *string;

   /* Alocate space to store screen contents "under" window */
   if((under = malloc((x2-x1+1)*(y2-y1+1)*2)) == NULL)
      {
      return(NULL);
      }

   /* Retrieve screen contents in window area */
   phys_gettext(x1, y1, x2, y2, under);

   /* Determine area between left & right of window, distance of line before */
   /* title, and distance of line after title */
   count = (between = x2 - x1 - 1) - strlen(title);
   count -= (first = count / 2);

   /* Prepare to begin drawing window at upper left corner */
   phys_gotoxy(x1, y1);
   phys_setattrib(attr);

   /* Draw first line of window */
   phys_putch(214);
   while(first--) phys_putch(196);
   phys_setattrib(titleattr);
   phys_cputs(title);
   phys_setattrib(attr);
   while(count--) phys_putch(196);
   phys_putch(183);

   /* Build string for working lines */
   string = tempstr;
   *string++ = 186;
   count = between;
   while(count--) *string++ = ' ';
   *string++ = 186;
   *string++ = '\0';

   /* Draw working lines of window */
   for(count = y1 + 1; count < y2; ++count)
      {
      phys_gotoxy(x1, count);
      phys_cputs(tempstr);
      }

   /* Draw last line of window */
   phys_gotoxy(x1, y2);
   phys_putch(211);
   while(between--) phys_putch(196);
   phys_putch(189);

   /* return pointer to buffer */
   return(under);
   }


/* phys_delwin() - removes a local-only popup window from the screen */
void phys_delwin(int x1, int y1, int x2, int y2, void *buffer)
   {
   phys_puttext(x1, y1, x2, y2, buffer);
   free(buffer);
   }


/* phys_clreol() - Clears all character from the cursor location to the   */
/*                 edge of the window on the current line, without moving */
/*                 the cursor.                                            */
void phys_clreol(void)
   {
   unsigned char to_delete = phys_winright - (phys_winleft + phys_curx);
   register char far *addr = (char far *) phys_buffer + (((phys_wintop + phys_cury) * 160) + (phys_winleft + phys_curx) * 2);
   register char attr = phys_attrib;

   while(to_delete--)
      {
      *(addr++) = ' ';
      *(addr++) = attr;
      }
   }
