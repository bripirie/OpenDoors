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
 *     Filename : ODCOM.C
 *  Description : Environment independent communications routines
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


/* Internal UART async serial I/O related definitions */

/* Offsets of UART registers */
#define TXBUFF  0                       /* Transmit buffer register */
#define RXBUFF  0                       /* Receive buffer register */
#define DLLSB   0                       /* Divisor latch LS byte */
#define DLMSB   1                       /* Divisor latch MS byte */
#define IER     1                       /* Interrupt enable register */
#define IIR     2                       /* Interrupt ID register */
#define LCR     3                       /* Line control register */
#define MCR     4                       /* Modem control register */
#define LSR     5                       /* Line status register */
#define MSR     6                       /* Modem status register */


/* FIFO control register bits */
#define FE      0x01                    /* FIFO enable */
#define RR      0x02                    /* FIFO receive buffer reset */
#define TR      0x04                    /* FIFO transmit buffer reset */
#define FTS_1   0x00                    /* FIFO trigger size 1 byte */
#define FTS_4   0x40                    /* FIFO trigger size 4 bytes */
#define FTS_8   0x80                    /* FIFO trigger size 8 bytes */
#define FTS_14  0xc0                    /* FIFO trigger size 14 bytes */


/* Modem control register (MCR) bits */
#define DTR     0x01                    /* Data terminal ready */
#define NOT_DTR 0xfe                    /* All bits other than DTR */
#define RTS     0x02                    /* Request to send */
#define OUT1    0x04                    /* Output #1 */
#define OUT2    0x08                    /* Output #2 */
#define LPBK    0x10                    /* Loopback mode bit */


/* Modem status register (MSR) bits */
#define DCTS    0x01                    /* Delta clear to send */
#define DDSR    0x02                    /* Delta data set ready */
#define TERI    0x04                    /* Trailing edge ring indicator */
#define DRLSD   0x08                    /* Delta Rx line signal detect */
#define CTS     0x10                    /* Clear to send */
#define DSR     0x20                    /* Data set ready */
#define RI      0x40                    /* Ring indicator */
#define RLSD    0x80                    /* Receive line signal detect */


/* Line control register (LCR) bits */
#define DATA5   0x00                    /* 5 Data bits */
#define DATA6   0x01                    /* 6 Data bits */
#define DATA7   0x02                    /* 7 Data bits */
#define DATA8   0x03                    /* 8 Data bits */

#define STOP1   0x00                    /* 1 Stop bit */
#define STOP2   0x04                    /* 2 Stop bits */

#define NOPAR   0x00                    /* No parity */
#define ODDPAR  0x08                    /* Odd parity */
#define EVNPAR  0x18                    /* Even parity */
#define STKPAR  0x28                    /* Sticky parity */
#define ZROPAR  0x38                    /* Zero parity */

#define DLATCH  0x80                    /* Baud rate divisor latch */
#define NOT_DL  0x7f                    /* Turns off divisor latch */

/* Line status register (LSR) bits */
#define RDR     0x01                    /* Receive data ready */
#define ERRS    0x1E                    /* All the error bits */
#define TXR     0x20                    /* Transmitter ready */


/* Interrupt enable register bits */
#define DR      0x01                    /* Data ready */
#define THRE    0x02                    /* Tx buffer empty */
#define RLS     0x04                    /* Receive line status */


/* Manifest constants */
#define MAX_PORT  4


/* Built-in async serial I/O global variables */

/* UART addresses */
int uart_data;  /* Data register */
int uart_ier;   /* Interrupt enable register */
int uart_iir;   /* Interrupt identification register */
int uart_lcr;   /* Line control register */
int uart_mcr;   /* Modem control register */
int uart_lsr;   /* Line status register */
int uart_msr;   /* Modem status register */

/* General variables */
char com_installed;             /* Flag: Communications routines installed */
int  intnum;                    /* Interrupt vector number for chosen port */
char i8259bit;                  /* 8259 bit mask */
char old_i8259_mask;            /* Copy as it was when we were called */
int i8259mask_reg;              /* Address of i8259 mask register */
int i8259eoi_reg;               /* Address of i8259 eoi register */
int i8259master_eoi_reg;        /* Address of master PIC eoi register */
char old_ier;                   /* Modem register contents saved for */
char old_mcr;                   /*  restoring when we're done */
void interrupt (*old_vector)(); /* Place to save port vector */
char using_fifo=FALSE;          /* Are we using 16550 FIFOs? */
unsigned char fifo_control;     /* FIFO control register byte */

/* Transmit queue */
int tx_queue_size;/* Actual size of transmit queue */
char *tx_queue;   /* Pointer to transmit queue */
int tx_in;        /* Index of where to store next character */
int tx_out;       /* Index of where to retrieve next character */
int tx_chars;     /* Count of characters in queue */

/*  Receive queue */
int rx_queue_size;/* Actual size of receive queue */
char *rx_queue;   /* Pointer to receive queue */
int rx_in;        /* Index of where to store next character */
int rx_out;       /* Index of where to retrieve next character */
int rx_chars;     /* Count of characters in queue */


void _set_vect(unsigned char vector, void INTERRUPT (far *isr)(void))
   {
   ASM   push ds
   ASM   mov ah, 0x25
   ASM   mov al, vector
   ASM   lds dx, isr
   ASM   int 0x21
   ASM   pop ds
   }


void INTERRUPT (far *_get_vect(unsigned char vector))(void)
   {
   void INTERRUPT (far *isr)(void);

   ASM   push es
   ASM   mov ah, 0x35
   ASM   mov al, vector
   ASM   int 0x21
   ASM   mov isr, bx
   ASM   mov word ptr [isr+2], bx
   ASM   pop es

   return(isr);
   }


/* _com_ini() - initializes serial I/O for either FOSSIL or internal         */
/*                 communications code, if applicable                        */
void _com_ini(void)
   {
   unsigned int divisor;
   unsigned long quotient, remainder;
   unsigned char code;
   unsigned char temp;

                                       /* determine desired BPS rate */
   if(_forced_bps!=1)
      {
      if(od_control.od_disable&DIS_LOCAL_OVERRIDE || od_control.baud!=0) od_control.baud=_forced_bps;
      }
                                       /* determine desired port */
   if(_forced_port!=-1) od_control.port=_forced_port;

   if(!od_control.baud) return;        /* return now if in local mode */

   /* if use of FOSSIL driver has not been disabled, then first attempt to */
   /* use it.                                                              */
   if(!od_control.od_no_fossil)
      {
#ifdef USEINLINE
      ASM    push si
      ASM    push di
      ASM    mov ah, 4
      ASM    mov dx, od_control.port
      ASM    mov bx, 0
      ASM    int 20
      ASM    pop di
      ASM    pop si
      ASM    cmp ax, 6484
      ASM    je fossil
#else
      regs.h.ah=4;                     /* check if fossil driver is installed */
      regs.x.dx=od_control.port;
      regs.x.bx=0;
      int86(20,&regs,&regs);
      if(regs.x.ax!=6484)              /* if no fossil, then exit with error */
         {
#endif
         goto no_fossil;
#ifndef USEINLINE
         }
#endif

fossil:
      od_control.od_com_method = COM_FOSSIL;

      if(od_control.od_disable&DIS_BPS_SETTING) return;

                                       /* set to current baud rate */
      switch(od_control.baud)
         {
         case 300U:
            #ifdef USEINLINE
            code = 67;
            #else
            regs.h.al=67;
            #endif
            break;

         case 600U:
            #ifdef USEINLINE
            code = 99;
            #else
            regs.h.al=99;
            #endif
            break;

         case 1200U:
            #ifdef USEINLINE
            code = 131;
            #else
            regs.h.al=131;
            #endif
            break;

         case 2400U:
            #ifdef USEINLINE
            code = 163;
            #else
            regs.h.al=163;
            #endif
            break;

         case 4800U:
            #ifdef USEINLINE
            code = 195;
            #else
            regs.h.al=195;
            #endif
            break;

         case 9600U:
            #ifdef USEINLINE
            code = 227;
            #else
            regs.h.al=227;
            #endif
            break;

         case 19200U:
            #ifdef USEINLINE
            code = 3;
            #else
            regs.h.al=3;
            #endif
            break;

         case 38400U:
            #ifdef USEINLINE
            code = 35;
            #else
            regs.h.al=35;
            #endif
            break;

         default:                      /* if invalid bps rate */
            return;                    /* don't change current bps setting */
         }

#ifdef USEINLINE
      ASM    push si
      ASM    push di
      ASM    mov al, code
      ASM    mov ah, 0
      ASM    mov dx, od_control.port
      ASM    int 20
      ASM    pop di
      ASM    pop si
#else
      regs.h.ah=0;                     /* initialize fossil driver */
      regs.x.dx=od_control.port;       /* set modem port */
      int86(20,&regs,&regs);           /* call initialization function */
#endif
      return;
      }

no_fossil:
      od_control.od_com_method = COM_INTERNAL;

      /* communications parameters relevant to internal com I/O: */
      /*    od_control.port            - always 0 based          */
      /*    od_control.baud            - 0 = local mode          */
      /*    od_control.od_com_method   - COM_INTERNAL or FOSSIL  */
      /*    od_control.od_com_address  - 0 indicates default     */
      /*    od_control.od_com_irq      - 0 indicates default     */
      /*    od_control.od_com_rx_buf   - receive buffer size     */
      /*    od_control.od_com_tx_buf   - transmit buffer size    */

      /* Establish default buffer sizes */
      if(od_control.od_com_rx_buf == 0)
         od_control.od_com_rx_buf = 256;
      if(od_control.od_com_tx_buf == 0)
         od_control.od_com_tx_buf = 1024;

      /* Allocate transmit and receive buffers */

      /* A copy of the actual transmit and receive buffer sizes is stored in */
      /* tx_queue_size and rx_queue_size to prevent problems when user the   */
      /* od_control.od_com_?x_buf after initialization time.                 */
      tx_queue = malloc(tx_queue_size = od_control.od_com_tx_buf);
      rx_queue = malloc(rx_queue_size = od_control.od_com_rx_buf);

      if(tx_queue == NULL || rx_queue == NULL)
         {
         puts("Critical Error [OpenDoors]: Not enough memory.\n");
         exit(od_control.od_errorlevel[1]);
         }

      /* If serial port address has not been explicitly set */
      if(od_control.od_com_address == 0)
         {
         /* Use default address for this port number */
         if(od_control.port < 4)
            {
            /* Get port address from BIOS data area */
            od_control.od_com_address = *(((int far *)0x400) + od_control.port);
            }

         /* If serial port address is still unknown */
         if(od_control.od_com_address == 0)
            {
            /* ERROR: port number too large */
            puts("Critical Error [OpenDoors]: Serial port address unknown.\n");
            exit(od_control.od_errorlevel[1]);
            }
         }

      /* If irq line has not been explicitly set to a valid value */
      if(od_control.od_com_irq == 0 || od_control.od_com_irq > 15)
         {
         /* Ports 0 and 2 (COM1:, COM3:) use IRQ 4, all others use IRQ 3 */
         if(od_control.port == 0 || od_control.port == 2)
            {
            od_control.od_com_irq = 4;
            }
         else
            {
            od_control.od_com_irq = 3;
            }
         }


      /* Initialize table of UART register port addresses */
      uart_data = od_control.od_com_address;
      uart_ier  = uart_data + IER;
      uart_iir  = uart_data + IIR;
      uart_lcr  = uart_data + LCR;
      uart_mcr  = uart_data + MCR;
      uart_lsr  = uart_data + LSR;
      uart_msr  = uart_data + MSR;


      /* Store interrupt vector number and PIC interrupt information for */
      /* the specified IRQ line.                                         */
      if(od_control.od_com_irq <= 7)
         {
         intnum    = 0x08 + od_control.od_com_irq;
         i8259bit  = 1 << od_control.od_com_irq;
         i8259mask_reg = 0x21;
         i8259eoi_reg = 0x20;
         i8259master_eoi_reg = 0x00;
         }
      else
         {
         intnum    = 0x68 + od_control.od_com_irq;
         i8259bit  = 1 << (od_control.od_com_irq - 8);
         i8259mask_reg = 0xA1;
         i8259eoi_reg = 0xA0;
         i8259master_eoi_reg = 0x20;
         }


      /* Save original state of UART IER register */
      ASM mov dx, uart_ier
      ASM in al, dx
      ASM mov old_ier, al

      /* Test that a UART is indeed installed at this port address */
      ASM mov dx, uart_ier
      ASM mov al, 0
      ASM out dx, al

      ASM mov dx, uart_ier
      ASM in al, dx
      ASM mov temp, al

      if (temp != 0)
         {
         puts("Critical Error [OpenDoors]: No UART at specified port address.\n");
         exit(od_control.od_errorlevel[1]);
         }


      /* Save original PIC interrupt settings, and temporarily disable */
      /* interrupts on this IRQ line while we perform initialization.  */
      ASM cli
 
      ASM mov dx, i8259mask_reg
      ASM in al, dx
      ASM mov old_i8259_mask, al
      ASM or  al, i8259bit
      ASM out dx, al

      ASM sti


      /* Initialize transmit and recieve buffers */
      _com_uart_flush_tx();
      _com_uart_flush_rx();

      /* Save original interrupt vector */
      old_vector = _get_vect(intnum);

      /* Set interrupt vector to point to our ISR */
      _set_vect(intnum, _com_uart_isr);
      com_installed = TRUE;

      /* Set line control register to 8 data bits, no parity bits, 1 stop bit */
      ASM mov dx, uart_lcr
      ASM mov al, DATA8 + NOPAR + STOP1
      ASM out dx, al

      /* Save original modem control register */
      ASM cli

      ASM mov dx, uart_mcr
      ASM in al, dx
      ASM mov old_mcr, al

      /* Keep current DTR setting, and activate RTS */
      temp = (old_mcr & DTR) | (OUT2 + RTS);
      ASM mov dx, uart_mcr
      ASM mov al, temp
      ASM out dx, al

      /* Enable use of 16550A FIFOs, if available */
      if(!od_control.od_com_no_fifo)
         {
         /* Set FIFO enable bit */
         fifo_control = FE;

         /* Set size of FIFO trigger */
         switch(od_control.od_com_fifo_trigger)
            {
            case 1:
               fifo_control |= FTS_1;
               break;
            case 4:
               fifo_control |= FTS_4;
               break;
            case 8:
               fifo_control |= FTS_8;
               break;
            case 14:
               fifo_control |= FTS_14;
               break;
            default:
               od_control.od_com_fifo_trigger = 4;
               fifo_control |= FTS_4;
            }

         /* Attempt to initialize use of FIFO buffers */
         ASM mov al, fifo_control
         ASM mov dx, uart_iir
         ASM out dx, al

         /* Check whether a 16550A UART is actually present by reading */
         /* state of FIFO buffer */
         ASM mov dx, uart_iir
         ASM in al, dx
         ASM mov temp, al

         using_fifo = temp & 0xc0;
         }

      ASM sti

      /* Enable receive interrupts on the UART */
      ASM mov dx, uart_ier
      ASM mov al, DR
      ASM out dx, al


      /* Enable interrupts on the PIC */
      ASM cli

      ASM mov dx, i8259mask_reg
      ASM in al, dx
      ASM mov ah, i8259bit
      ASM not ah
      ASM and al, ah
      ASM out dx, al

      ASM sti


      /* Set baud rate, if possible */

      /* Calculate baud rate divisor */
      ASSERT(od_control.baud != 0);
      _ulongdiv(&quotient, &remainder, 115200UL, od_control.baud);

      /* If division results in a remainder, then this is an invalid     */
      /* baud rate. We only change the UART baud rate if we have a valid */
      /* rate to set it to. Otherwise, we cross our fingers and proceed  */
      /* with the currently set UART baud rate.                          */
      if(remainder == 0L)
         {
         divisor = (unsigned int)quotient;

         /* Disable interrupts */
         ASM cli

         /* Set baud rate divisor latch */
         /* The data register now becomes the lower byte of the baud rate */
         /* divisor, and the interrupt enable register becomes the upper  */
         /* byte of the divisor.                                          */
         ASM mov dx, uart_lcr
         ASM in al, dx
         ASM or al, DLATCH
         ASM out dx, al

         /* Write lower byte of baud rate divisor */
         ASM mov dx, uart_data
         ASM mov ax, divisor
         ASM out dx, al

         /* Write upper byte of baud rate divisor */
         ASM mov dx, uart_ier
         ASM mov al, ah
         ASM out dx, al

         /* Reset baud rate divisor latch */
         ASM mov dx, uart_lcr
         ASM in al, dx
         ASM and al, NOT_DL
         ASM out dx, al

         /* Re-enable interrupts */
         ASM sti
         }
   }


void _com_close(void)
   {
   unsigned char temp;

   if(!od_control.baud) return;

   switch(od_control.od_com_method)
      {
      case COM_FOSSIL:
#ifdef USEINLINE
         ASM    mov ah, 5
         ASM    mov dx, od_control.port
         ASM    int 20
#else
         regs.h.ah=5;
         regs.x.dx=od_control.port;
         int86(20,&regs,&regs);
#endif
         break;

      case COM_INTERNAL:
          /* Reset UART registers to their original values */
          ASM mov dx, uart_mcr
          ASM mov al, old_mcr
          ASM out dx, al
          ASM mov dx, uart_ier
          ASM mov al, old_ier
          ASM out dx, al

          /* Disable interrupts */
          ASM cli
  
          /* Reset this line's interrupt enable status on the PIC to its */
          /* original state.                                            */
         ASM mov dx, i8259mask_reg
         ASM in al, dx
         ASM mov temp, al
  
         temp = (temp  & ~i8259bit) | (old_i8259_mask &  i8259bit);
    
         ASM mov dx, i8259mask_reg
         ASM mov al, temp
         ASM out dx, al
  
         /* Re-enable interrupts */
         ASM sti

         /* Reset vector to original interrupt handler */
         _set_vect(intnum, old_vector);

         break;
      }
   }


int _com_carrier(void)
   {
   if (od_control.od_com_method == COM_FOSSIL)
      {
#ifdef USEINLINE
      int to_return;

      ASM    mov ah, 3
      ASM    mov dx, od_control.port
      ASM    int 20
      ASM    and ax, 128
      ASM    mov to_return, ax
      return (to_return);
#else
      regs.h.ah=3;
      regs.x.dx=od_control.port;
      int86(20,&regs,&regs);
      return(regs.x.ax&128);
#endif
      }

   else /* if (od_control.od_com_method == COM_INTERNAL) */
      {
      unsigned char msr;

      ASM mov dx, uart_msr
      ASM in al, dx
      ASM mov msr, al

      return(msr & RLSD);
      }
   }


void _com_dtr(char high)
   {
   if (od_control.od_com_method == COM_FOSSIL)
      {
#ifdef USEINLINE
      ASM    cmp byte ptr high, 0
      ASM    je low
      ASM    mov al, 1
      ASM    jmp set_dtr

low:
      ASM    xor al, al

set_dtr:
      ASM    mov ah, 6
      ASM    mov dx, od_control.port
      ASM    int 20

#else
      regs.h.ah=6;
      regs.x.dx=od_control.port;
      if(high)
         regs.h.al=1;
      else
         regs.h.al=0;
      int86(20,&regs,&regs);
#endif
      }

   else /* if (od_control.od_com_method == COM_INTERNAL) */
      {
      if(high)
         {
         ASM cli
      
         ASM mov dx, uart_mcr
         ASM in al, dx
         ASM or al, DTR
         ASM out dx, al

         ASM sti
         }
      else
         {
         ASM cli
    
         ASM mov dx, uart_mcr
         ASM in al, dx
         ASM and al, NOT_DTR
         ASM out dx, al

         ASM sti
         }
      }
   }


char _com_outbound(void)
   {
   if (od_control.od_com_method == COM_FOSSIL)
      {
#ifdef USEINLINE
      ASM    mov ah, 0x03
      ASM    mov dx, od_control.port
      ASM    int 20
      ASM    and ah, 0x40
      ASM    jz  still_sending
      return(FALSE);
still_sending:
      return(TRUE);
      ;
#else
      regs.h.ah = 0x03;
      regs.x.dx = od_control.port;
      int86(20, &regs, &regs);
      return(!(regs.h.ah&0x40));
#endif
      }

   else /* if (od_control.od_com_method == COM_INTERNAL) */
      {
      return(tx_chars);
      }
   }


void _com_clear_outbound(void)
   {
   if (od_control.od_com_method == COM_FOSSIL)
      {
#ifdef USEINLINE
      ASM    mov ah, 9
      ASM    mov dx, od_control.port
      ASM    int 20
#else
      regs.h.ah=9;
      regs.x.dx=od_control.port;
      int86(20,&regs,&regs);
#endif
      }

   else /* if (od_control.od_com_method == COM_INTERNAL) */
      {
      _com_uart_flush_tx();
      }
   }


void _com_clear_inbound(void)
   {
   if (od_control.od_com_method == COM_FOSSIL)
      {
#ifdef USEINLINE
      ASM    mov ah, 10
      ASM    mov dx, od_control.port
      ASM    int 20
#else
      regs.h.ah=10;
      regs.x.dx=od_control.port;
      int86(20,&regs,&regs);
#endif
      }

   else /* if (od_control.od_com_method == COM_INTERNAL) */
      {
      _com_uart_flush_rx();
      }
   }


char _com_inbound(void)
   {
   if (od_control.od_com_method == COM_FOSSIL)
      {
      char to_return;

#ifdef USEINLINE
      ASM    mov ah, 3
      ASM    mov dx, od_control.port
      ASM    push si
      ASM    push di
      ASM    int 20
      ASM    pop di
      ASM    pop si
      ASM    and ah, 1
      ASM    mov to_return, ah
#else
      regs.h.ah=3;                     /* check for characters waiting in */
      regs.x.dx=od_control.port;       /* fossil buffer */
      int86(20,&regs,&regs);
      to_return=regs.h.ah&1;
#endif

      return(to_return);
      }

   else /* if (od_control.od_com_method == COM_INTERNAL) */
      {
      return(rx_chars);
      }
   }


char _com_getchar(void)
   {
   if (od_control.od_com_method == COM_FOSSIL)
      {
      char to_return;

#ifdef USEINLINE
         ASM     mov ah, 2
         ASM     mov dx, od_control.port
         ASM     push si
         ASM     push di
         ASM     int 20
         ASM     pop di
         ASM     pop si
         ASM     mov to_return, al
#else
         regs.h.ah=2;                  /* get character from fossil */
         regs.x.dx=od_control.port;
         int86(20,&regs,&regs);
         to_return = regs.h.al;
#endif

      return(to_return);
      }

   else /* if (od_control.od_com_method == COM_INTERNAL) */
      {
      char next_char;

      /* Loop, calling od_kernel(), until next character arrives */
      while(!rx_chars)
         {
         od_kernel();
         }

      /* Disable interrupts */
      ASM cli

      /* Get next character from receive queue */
      next_char = rx_queue[rx_out++];

      /* Wrap queue index in needed */
      if (rx_out == rx_queue_size)
         {
         rx_out = 0;
         }

      /* Decrement count of total character in the receive queue */
      rx_chars--;

      /* Re-enable interrupts */
      ASM sti

      /* Return retrieved character */
      return(next_char);
      }
   }


void _com_sendchar(char chr)
   {
   if (od_control.od_com_method == COM_FOSSIL)
      {
try_again:
#ifdef USEINLINE
      ASM    mov ah, 0x0b
      ASM    mov dx, od_control.port
      ASM    mov al, chr
      ASM    int 20
      ASM    cmp ax, 0
      ASM    jne keep_going
      od_kernel();
      goto try_again;
#else
      regs.h.ah=0x0b;                     /* send character to modem */
      regs.x.dx=od_control.port;
      regs.h.al=chr;
      int86(20,&regs,&regs);
      if(!regs.x.ax)
         {
         od_kernel();
         goto try_again;
         }
#endif
keep_going:
      return;
      }

   else /* if (od_control.od_com_method == COM_INTERNAL) */
      {
      /* Loop, calling od_kernel(), until characters are waiting in the */
      /* transmit buffer. */
      while(!_com_uart_tx_ready())
         {
         od_kernel();
         }

      /* Disable interrupts */
      ASM cli

      /* Place the character in the queue */
      tx_queue[tx_in++] = chr;

      /* Wrap transmit queue index, if needed */
      if (tx_in == tx_queue_size)
         {
         tx_in = 0;
         }

      /* Increment count of total characters in the queue */
      tx_chars++;

      /* Enable transmit interrupt on the UART */
      ASM mov dx, uart_ier
      ASM in al, dx
      ASM or al, THRE
      ASM out dx, al

      ASM sti       /* Interrupts back on */
      }
   }


void _com_send_buf(char *buffer, int size)
   {
   /* If there are no characters to transmit, then there is no need to */
   /* proceed further.                                                 */
   if(size == 0)
      {
      return;
      }

   if(od_control.od_com_method == COM_FOSSIL)
      {
      int counter;

try_again:
      ASM    push di
      ASM    mov cx, size
      ASM    mov dx, od_control.port
#ifdef LARGEDATA
      ASM    mov ax, [bp+SECONDPARAM]
      ASM    mov es, ax
#else
      ASM    mov ax, ds
      ASM    mov es, ax
#endif
      ASM    mov di, [bp+FIRSTPARAM]
      ASM    mov ah, 0x19
      ASM    int 20
      ASM    pop di
      ASM    mov counter, ax

      if(counter<size)
         {
         od_kernel();
         giveup_slice();
         size-=counter;
         buffer+=counter;
         goto try_again;
         }
      }

   else /* if (od_control.od_com_method == COM_INTERNAL) */
      {
      int transfer_size;
      int first_half_size;
      int second_half_size;
      char *dest_pointer;

      /* Loop, copying as much of buffer to transmit queue as possible, */
      /* then waiting for some characters to be transmitted, and copy   */
      /* more of buffer to transmit queue, until entire buffer has been */
      /* transferred.                                                   */
      for(;;)
         {
         /* Disable interrupts */
         ASM cli

         /* Try to transfer all of buffer if possible */
         transfer_size = size;

         /* Adjust number of character to transfer down if there isn't */
         /* enough space in transmit queue.                            */
         if(transfer_size > (tx_queue_size - tx_chars))
            {
            transfer_size = (tx_queue_size - tx_chars);
            }

         /* Block transfer is divided into two segments - everything from */
         /* current in index to end of queue, and everything from         */
         /* beginning of queue to end of free space in queue.             */

         /* Calculate size of first half of transfer */
         first_half_size = tx_queue_size - tx_in;
         if(first_half_size > transfer_size) first_half_size = transfer_size;

         /* Calculate size of second half of transfer */
         second_half_size = transfer_size - first_half_size;

         /* Transfer characters at current queue in index */
         dest_pointer = tx_queue + tx_in;
         while(first_half_size--)
            {
            *dest_pointer++ = *buffer++;
            }

         /* If there is a second half to transfer */
         if(second_half_size)
            {
            /* Copy destination will begin at beginning of queue */
            dest_pointer = tx_queue;

            /* Set final queue in index */
            tx_in = second_half_size;

            /* Perform second half of transfer */
            while(second_half_size--)
               {
               *dest_pointer++ = *buffer++;
               }
            }

         /* If entire transfer was performed in first half */
         else
            {
            /* Set final queue in index */
            tx_in += transfer_size;

            /* Wrap queue in index if we just happened to fill characters   */
            /* up to end of physical queue. If there was one less character */
            /* transferred, no wrap would be necessary, and if there was    */
            /* one more character to be transferred, transfer would have to */
            /* be performed in two halfs.                                   */
            if(tx_in == tx_queue_size) tx_in = 0;
            }

         /* Update count of total characters in the queue */
         tx_chars += transfer_size;

         /* Enable transmit interrupt on the UART */
         ASM mov dx, uart_ier
         ASM in al, dx
         ASM or al, THRE
         ASM out dx, al

         /* Re-enable interrupts */
         ASM sti

         /* Adjust count of characters left to transfer down by number of */
         /* characters transferred.                                       */
         size -= transfer_size;

         /* If there are no characters left to transfer, then we are done */
         if(size == 0) return;

         /* Execute kernel routines */
         od_kernel();

         /* Give other processes a chance to run now */
         giveup_slice();
         }
      }
   }


/* _com_uart_tx_ready()                                                      */
/*                                                                           */
/* Returns TRUE if the transmit buffer is not full                           */
int _com_uart_tx_ready(void)
   {
   /* Return TRUE if tx_chars is less than total tx buffer size */
   return(tx_chars < tx_queue_size);
   }


/* _com_uart_flush_tx()                                                      */
/*                                                                           */
/* Clears the UART async serial I/O transmit buffer.                         */
void _com_uart_flush_tx()
   {
   /* Disable interrupts */
   ASM cli

   /* If we are using 16550A FIFO buffers, then clear the FIFO transmit */
   /* buffer.                                                           */
   if(using_fifo)
      {
      ASM mov al, fifo_control
      ASM or al, TR
      ASM mov dx, uart_iir
      ASM out dx, al
      }

   /* Reset start, end and total count of characters in buffer      */
   /* If buffer is still empty on next transmit interrupt, transmit */
   /* interrupts will be turned off.                                */
   tx_chars = tx_in = tx_out = 0;

   /* Re-enable interrupts */
   ASM sti
   }


/* _com_uart_flush_rx()                                                      */
/*                                                                           */
/* Clears the UART async serial I/O receive buffer.                          */
void _com_uart_flush_rx()
   {
   /* Disable interrupts */
   ASM cli

   /* If we are using 16550A FIFO buffers, then clear the FIFO receive */
   /* buffer.                                                          */
   if(using_fifo)
      {
      ASM mov al, fifo_control
      ASM or al, RR
      ASM mov dx, uart_iir
      ASM out dx, al
      }

   /* Reset start, end and total count of characters in buffer           */
   /* On the next receive interrupt, data will be added at the beginning */
   /* of the buffer.                                                     */
   rx_chars = rx_in = rx_out = 0;

   /* Re-enable interrupts */
   ASM sti
   }


/* _com_uart_isr()                                                           */
/*                                                                           */
/* Interrupt service routine for UART-based serial I/O.                      */
void INTERRUPT _com_uart_isr()
   {
   char  iir;
   char  c;
   char  temp;

   /* Loop until there are no more pending operations to perform with the */
   /* UART. */
   for(;;)
      {
      /* While bit 0 of the UART IIR is 0, there remains pending operations */
      /* Read IIR */
      ASM mov dx, uart_iir
      ASM in al, dx
      ASM mov iir, al

      /* If IIR bit 0 is set, then UART processing is finished.             */
      if (iir & 0x01) break;

      /* Bits 1 and 2 of the IIR register identify the type of operation */
      /* to be performed with the UART.                                  */

      /* Switch on bits 1 and 2 of IIR register */
      switch (iir & 0x06)
         {
         case 0x00:
            /* Operation: modem status has changed */

            /* We don't care about this, so we simply read the MSR register */
            /* and ignore it, in order to force the UART to present any     */
            /* further pending operations.                                  */
            ASM mov dx, uart_msr
            ASM in al, dx
            break;

         case 0x02:
            /* Operation: room in transmit register/FIFO */

            /* Check whether we have any further characters to transmit */
            if (tx_chars <= 0)
               {
               /* If there are no more characters to send, turn off */
               /* transmit interrupts.                              */
               ASM mov dx, uart_ier
               ASM in al, dx
               ASM and al, 0xfd
               ASM out dx, al
               }
            else
               {
               /* If we still have characters to transmit ... */

               /* Check line status register to determine whether transmit  */
               /* register/FIFO truly has room. Some UARTs trigger transmit */
               /* interrupts before the character has been tranmistted,     */
               /* causing transmitted characters to be lost.                */
               ASM mov dx, uart_lsr
               ASM in al, dx
               ASM mov temp, al
        
               if (temp & TXR)
                  {
                  /* There is room in the transmit register/FIFO */

                  /* Get next character to transmit */
                  temp = tx_queue[tx_out++];    

                  /* Write character to UART data register */
                  ASM mov dx, uart_data
                  ASM mov al, temp
                  ASM out dx, al

                  /* Wrap-around transmit buffer pointer, if needed */
                  if (tx_out == tx_queue_size)
                     tx_out = 0;

                  /* Decrease count of characters in transmit buffer */
                  tx_chars--;
                  }
               }
            break;

         case 4:
            /* Operation: Receive Data */

            /* Get character from receive buffer ASAP */
            ASM mov dx, uart_data
            ASM in al, dx
            ASM mov c, al

            /* If there is room in receive buffer */
            if (rx_chars < rx_queue_size)
               {
               /* Store the new character in the receive buffer */
               rx_queue[rx_in++] = c;

               /* Wrap-around buffer index, if needed */
               if (rx_in == rx_queue_size)
                  rx_in = 0;

               /* Increment count of characters in the buffer */
               rx_chars++;
               }
            break;

         case 6:
            /* Operation: Change in line status register */

            /* We just read the register to move on to further operations */
            ASM mov dx, uart_lsr
            ASM in al, dx
            break;
         }
      }

   /* Send end of interrupt to interrupt controller(s) */
   ASM mov dx, i8259eoi_reg
   ASM mov al, 0x20
   ASM out dx, al

   if(i8259master_eoi_reg != 0)
      {
      ASM mov dx, i8259master_eoi_reg
      ASM mov al, 0x20
      ASM out dx, al
      }
   }
