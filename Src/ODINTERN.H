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
 *     Filename : OD_INTERN.H
 *  Description : Contains internal global definitions used for compiling the
 *                OpenDoors library - should not be used when compiling an
 *                application which uses OpenDoors, but only OpenDoors itself.
 *      Version : 5.00
 */


/* Include other required header files */
#include <time.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>



/* Compile setting defintions */
#define USEREPLACEMENTS        /* Causes use of TC/BC++ replacement functions */
#define USEINLINE             /* Use of inline assembly language is available */

#define CVERSION                   /* This package should always set CVERSION */
#undef PASCALVERSION
#undef BASICVERSION

#ifdef USEINLINE
#define ASM asm                             /* Define inline assembly keyword */
#endif

#define INTERRUPT interrupt

#ifdef INCLUDE_ASSERT
#include <assert.h>
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

#ifdef INCLUDE_TRACE
#include <stdio.h>
#define TRACE_API 1
#define TRACE(x, y) printf("[%s]", y);
#else
#define TRACE(x, y)
#endif


/* Internal replacements for Turbo C/Borland C++ specific library functions */
#ifdef USEREPLACEMENTS
   struct phys_text_info
      {
      unsigned char winleft;
      unsigned char wintop;
      unsigned char winright;
      unsigned char winbottom;
      unsigned char attribute;
      unsigned char curx;
      unsigned char cury;
      };

   void phys_clrscr(void);
   int phys_cprintf(char *format, ...);
   int phys_cputs(char *string);
   void phys_cursor(unsigned char enabled);
   int phys_gettext(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, void *buffer);
   void phys_gettextinfo(struct phys_text_info *info);
   void phys_getcursor(void);
   void phys_gotoxy(unsigned char x, unsigned char y);
   void phys_init(void);
   void phys_putch(char character);
   int phys_puttext(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, void *buffer);
   int phys_movetext(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char xdest, unsigned char ydest);
   void phys_setattrib(unsigned char attrib);
   void phys_setscroll(unsigned char enabled);
   void phys_scroll_primitive(void);
   void phys_update_cursor(void);
   void phys_window(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
   void phys_clreol(void);
#else
   #include <conio.h>
   #define phys_clrscr            clrscr
   #define phys_cprintf           cprintf
   #define phys_cputs             cputs
   #define phys_cursor(x)
   #define phys_gettext           gettext
   #define phys_gettextinfo       gettextinfo
   #define phys_gotoxy            gotoxy
   #define phys_init              clrscr
   #define phys_movetext          movetext
   #define phys_putch             putch
   #define phys_puttext           puttext
   #define phys_setattrib         textattr
   #define phys_setscroll(x)      _wscroll = x ? 1 : 0;
   #define phys_window            window
   #define phys_text_info         text_info
   #define phys_clreol            clreol
#endif


/* Memory-model dependant definitions */
#ifdef __TINY__
#define SMALLDATA
#define SMALLCODE
#define FIRSTPARAM 4
#define SECONDPARAM 6
#define THIRDPARAM 8
#endif
#ifdef __SMALL__
#define SMALLDATA
#define SMALLCODE
#define FIRSTPARAM 4
#define SECONDPARAM 6
#define THIRDPARAM 8
#endif
#ifdef __COMPACT__
#define LARGEDATA
#define SMALLCODE
#define FIRSTPARAM 4
#define SECONDPARAM 6
#define THIRDPARAM 8
#endif
#ifdef __MEDIUM__
#define SMALLDATA
#define LARGECODE
#define FIRSTPARAM 6
#define SECONDPARAM 8
#define THIRDPARAM 10
#endif
#ifdef __LARGE__
#define LARGEDATA
#define LARGECODE
#define FIRSTPARAM 6
#define SECONDPARAM 8
#define THIRDPARAM 10
#endif
#ifdef __HUGE__
#define LARGEDATA
#define LARGECODE
#define FIRSTPARAM 6
#define SECONDPARAM 8
#define THIRDPARAM 10
#endif



/* Global constant definitions */
#define TEXT_SIZE 47                    /* Number of built-in Logfile options */
#define LINES_SIZE 25             /* Number of user-defined info file options */
#define BS 256       /* Default size of local/remote combined keyboard buffer */
#define MAX_PERSONALITIES 12               /* Maximum number of personalities */
#define STATUS_FREQUENCY 3L     /* Frequency to update status line in seconds */
#define MENU_LEVELS 10                               /* Number of menu levels */

#define SFP_CHAT  0x01

#define MULTITASKER_NONE 0
#define MULTITASKER_DV   1
#define MULTITASKER_WIN  2
#define MULTITASKER_OS2  3

#define FA_RDONLY        0x01
#define FA_HIDDEN        0x02
#define FA_SYSTEM        0x04
#define FA_LABEL         0x08
#define FA_DIREC         0x10
#define FA_ARCH          0x20

#define WILDCARDS 0x01
#define EXTENSION 0x02
#define FILENAME  0x04
#define DIRECTORY 0x08
#define DRIVE	  0x10

/* Internal types definitions */
#ifndef BOOL
#define BOOL char
#endif

struct _menu
   {
   char text[77];
   char key;
   };

struct _menuinfo
   {
   struct _menu *menu;
   char menu_items;
   char width;
   char right;
   char bottom;
   char cursor;
   int left;
   int top;
   int flags;
   void *win;
   };

struct find_block
   {
   char ff_reserved[21];
   char ff_attrib;
   unsigned ff_ftime;
   unsigned ff_fdate;
   long ff_fsize;
   char ff_name[13];
   };

struct _ra2exitinfo                  /* PASCAL format of RA 2.00 EXITINFO.BBS */
   {
   unsigned int baud;
   long num_calls;
   char last_caller[36];
   char extra1[128];
   char start_date[9];
   int busyperhour[24];
   int busyperday[7];
   char name[36];
   char location[26];
   char organisation[51];
   char address[3][51];
   char handle[36];
   char comment[81];
   long password_crc;
   char dataphone[16];
   char homephone[16];
   char lasttime[6];
   char lastdate[9];
   unsigned char attrib;
   unsigned char attrib2;
   char flags[4];
   unsigned long credit;
   unsigned long pending;
   unsigned int posted;
   unsigned int sec;
   unsigned long lastread;
   unsigned long nocalls;
   unsigned long ups;
   unsigned long downs;
   unsigned long upk;
   unsigned long downk;
   unsigned long todayk;
   int elapsed;
   int screenlen;
   char lastpwdchange;
   unsigned int group;
   int combinedrecord[200];
   char firstcall[9];
   char birthday[9];
   char subdate[9];
   char screenwidth;
   unsigned char language;
   unsigned char dateformat;
   char forwardto[36];
   unsigned int msgarea;
   unsigned int filearea;
   char default_protocol;
   unsigned int file_group;
   unsigned char last_dob_check;
   unsigned char sex;
   unsigned long int xirecord;
   unsigned int msg_group;
   unsigned char extra2[48];
   char status;
   char starttime[6];
   char errorlevel;
   char days;
   char forced;
   char lasttimerun[9];
   char netmailentered;
   char echomailentered;
   char logintime[6];
   char logindate[9];
   int  timelimit;
   long loginsec;
   int userrecord;
   int readthru;
   int numberpages;
   int downloadlimit;
   char timeofcreation[6];
   long logonpasswordcrc;
   char wantchat;
   int deducted_time;
   char menustack[50][9];
   char menustackpointer;
   char extra3[200];
   char error_free;
   char sysop_next;
   char emsi_session;
   char emsi_crtdef[41];
   char emsi_protocols[41];
   char emsi_capabilities[41];
   char emsi_requests[41];
   char emsi_software[41];
   char hold_attr1;
   char hold_attr2;
   char hold_len;
   char page_reason[81];
   char status_line;
   char last_cost_menu[9];
   int menu_cost_per_min;
   char has_avatar;
   char has_rip;
   char extra4[86];
   };


struct _exitinfo                             /* Pascal format of EXITINFO.BBS */
   {
   int baud;
   long num_calls;
   char last_caller[36];
   char extraspace[128];
   char start_date[9];
   int busyperhour[24];
   int busyperday[7];
   char uname[36];
   char uloc[26];
   char password[16];
   char dataphone[13];
   char homephone[13];
   char lasttime[6];
   char lastdate[9];
   unsigned char attrib;
   unsigned char flags[4];
   unsigned int credit;
   unsigned int pending;
   unsigned int posted;
   unsigned int lastread;
   unsigned int sec;
   unsigned int nocalls;
   unsigned int ups;
   unsigned int downs;
   unsigned int upk;
   unsigned int downk;
   unsigned int todayk;
   int elapsed;
   unsigned int screenlen;
   unsigned char lastpwdchange;
   unsigned char attrib2;
   unsigned char group;
   unsigned int xirecord;
   char extra2[3];
   char status;
   char starttime[6];
   char errorlevel;
   char days;
   char forced;
   char lasttimerun[9];
   char netmailentered;
   char echomailentered;
   char logintime[6];
   char logindate[9];
   int  timelimit;
   long loginsec;
   long net_credit;
   int userrecord;
   int readthru;
   int numberpages;
   int downloadlimint;
   union
      {
      struct
         {
         char timeofcreation[6];
         char logonpassword[16];
         char wantchat;
         } ra;
      struct
         {
         char qwantchat;
         char gosublevel;
         char menustack[20][9];
         char menu[9];
         } qbbs;
      } bbs;
   };

struct _ext_exitinfo                    /* Pascal format of extended exitinfo */
   {
   int deducted_time;
   char menustack[50][9];
   char menustackpointer;
   char userhandle[36];
   char comment[81];
   char firstcall[9];
   char combinedrecord[25];
   char birthday[9];
   char subdate[9];
   unsigned char screenwidth;
   unsigned char msgarea;
   unsigned char filearea;
   unsigned char language;
   unsigned char dateformat;
   char forwardto[36];
   char extra_space[43];
   char error_free;
   char sysop_next;
   char emsi_session;
   char emsi_crtdef[41];
   char emsi_protocols[41];
   char emsi_capabilities[41];
   char emsi_requests[41];
   char emsi_software[41];
   char hold_attr1;
   char hold_attr2;
   char hold_len;
   char extr_space[100];
   };

struct _pcbsys
   {
   char display[2];       /* "-1" = On, " 0" = Off */
   char printer[2];
   char pagebell[2];
   char calleralarm[2];
   char sysopflag;        /* ' ', 'N'=sysop next, 'X'=exit to dos */
   char errorcorrection[2];
   char graphicsmode;     /* 'Y'=Yes, 'N'=No, '7'=7E1 */
   char nodechat;         /* 'A'=available, 'U'=unavailable */
   char dteportspeed[5];
   char connectspeed[5];  /* "Local"=local mode */
   int  recordnum;
   char firstname[15];
   char password[15];
   int logontimeval;      /* minutes since midnight */
   int todayused;         /* -ve # of minutes */
   char logontime[5];
   int  timeallowed;
   int  kallowed;
   char conference;
   char joined[5];
   char scanned[5];
   int conferenceaddtime;
   int creditminutes;
   char languageext[4];
   char fullname[25];
   int minutesremaining;
   char node;             /* ' ' if no network */
   char eventtime[5];
   char eventactive[2];
   char slideevent[2];
   long memmessage;
   char comport;          /* 0=none, 1-8 */
   char reserved1[2];
   char useansi;          /* 1 = yes, 0 = no */
   char lasteventdate[8];
   int lasteventminute;
   char dosexit;
   char eventupcoming;
   char stopuploads;
   int conferencearea;
   };

struct _userssyshdr
   {
   unsigned Version;           /* PCBoard version number (i.e. 145) */
   long     RecNo;             /* Record number from USER's file */
   unsigned SizeOfRec;         /* Size of "fixed" user record */
   unsigned NumOfAreas;        /* Number of conference areas (Main=1) */
   unsigned NumOfBitFields;    /* Number of Bit Map fields for conferences */
   unsigned SizeOfBitFields;   /* Size of each Bit Map field */
   char     AppName[15];       /* Name of the Third Party Application */
   unsigned AppVersion;        /* Version number for the application */
   unsigned AppSizeOfRec;      /* Size of a "fixed length" record (if any) */
   unsigned AppSizeOfConfRec;  /* Size of each conference record (if any) */
   long     AppRecOffset;      /* Offset of AppRec into USERS.INF record */
   char     Updated;           /* TRUE if USERS.SYS has been updated */
   };

struct _pcbflags
   {
   int Dirty    :1;            /* Dirty Flag (meaning file has been updated) */
   int MsgClear :1;            /* User's choice for screen clear after messages */
   int HasMail  :1;            /* Indicates if NEW mail has been left for user */
   int Reserved :5;
   };

struct _pcbdate
   {
   int Day   :5;               /* 5 bit integer representing the Day */
   int Month :4;               /* 4 bit integer representing the Month */
   int Year  :7;               /* 7 bit integer representing the Year MINUS 80 */
   };

struct _userssysrec
   {
   char     Name[26];          /* Name (NULL terminated) */
   char     City[25];          /* City (NULL terminated) */
   char     Password[13];      /* Password (NULL terminated) */
   char     BusDataPhone[14];  /* Business or Data Phone (NULL terminated) */
   char     HomeVoicePhone[14];/* Home or Voice Phone (NULL terminated) */
   unsigned LastDateOn;        /* Julian date for the Last Date On */
   char     LastTimeOn[6];     /* Last Time On (NULL Terminated) */
   char     ExpertMode;        /* 1=Expert, 0=Novice */
   char     Protocol;          /* Protocol (A thru Z) */
   struct _pcbflags PackedFlags;
   struct _pcbdate DateLastDirRead;
   int      SecurityLevel;     /* Security Level */
   unsigned NumTimesOn;        /* Number of times the caller has connected */
   char     PageLen;           /* Page Length when display data on the screen */
   unsigned NumUploads;        /* Total number of FILES uploaded */
   unsigned NumDownloads;      /* Total number of FILES downloaded */
   long     DailyDnldBytes;    /* Number of BYTES downloaded so far today */
   char     UserComment[31];   /* Comment field #1 (NULL terminated) */
   char     SysopComment[31];  /* Comment field #1 (NULL terminated) */
   int      ElapsedTimeOn;     /* Number of minutes online */
   unsigned RegExpDate;        /* Julian date for Registration Expiration Date */
   int      ExpSecurityLevel;  /* Expired Security Level */
   unsigned LastConference;    /* Number of the conference the caller was in */
   long     TotDnldBytes;      /* Total number of BYTES downloaded */
   long     TotUpldBytes;      /* Total number of BYTES uploaded */
   char     DeleteFlag;        /* 1=delete this record, 0=keep */
   long     RecNum;            /* Record Number in USERS.INF file */
   char     Reserved[9];       /* Bytes 391-399 from the USERS file */
   long     MsgsRead;          /* Number of messages the user has read in PCB */
   long     MsgsLeft;          /* Number of messages the user has left in PCB */
   };

typedef struct _vector
   {
   char number;                        /* vector number */
   char flag;                          /* 0-CURRENT, 1-IRET, 2-free, 3-end */
   unsigned int vseg;                  /* vector segment */
   unsigned int voff;                  /* vector offset */
   } VECTOR;

/* Uitlity macros */

#define _clock_tick() (*(long far *)0x46cL)

/* Internal function prototypes */
void __kx(char *string,int sum);
void __kxx(void);
int _accessmode(char *filename, int amode);
void _add_key(int code);
void _autocomclose(void);
void _autocomini(void);
int _chkems(char *, int *);
int _chdir(char *path);
void _close_logfile(int reason);
int _com_carrier(void);
void _com_clear_inbound(void);
void _com_clear_outbound(void);
void _com_close(void);
void _com_dtr(char high);
char _com_getchar(void);
void _com_ini(void);
char _com_inbound(void);
char _com_outbound(void);
void _com_send_buf(char *buffer,int size);
void _com_sendchar(char ch);
void _com_uart_flush_rx(void);
void _com_uart_flush_tx(void);
void INTERRUPT _com_uart_isr();
int _com_uart_tx_ready(void);
int _create(char *, int *);
int _dskspace(int, unsigned int *, unsigned int *);
void _fn_mrg(char *path, const char *drive, const char *dir, const char *name, const char *ext);
int  _fn_splt(const char *path, char *drive, char *dir, char *name, char *ext);
long int _fsize(FILE *stream);
int _getcd(int, char *);
int _getdrv(void);
int _getems(int, int *);
int _getrc(void);
void _getvect(int, unsigned int *, unsigned int *);
void INTERRUPT (far *_get_vect(unsigned char vector)) (void);
FILE *_gfilesearch(char *basename, int *level);
int  _log_write(int code);
BOOL _ulongdiv(unsigned long * pluQuotient, unsigned long * pluRemainder,
              unsigned long luDividend, unsigned long luDivisor);
void _menu_line(int left, int top, struct _menu *menu, int which, int highlight, int width, int position);
void _menu_look(void);
void _mt_init(void);
int _odfindfirst(const char *path, struct find_block *block, int attrib);
int _odfindnext(struct find_block *block);
int _pageprompt(char *pausing);
void _preexit(void);
int _restmap(char *);
void _remotechar(int character);
void _remove_key(int code);
int _savemap(char *);
void _setdrvcd(int, char * );
void _setvect(VECTOR *);
void _set_vect(unsigned char vector, void INTERRUPT (far *isr) (void));
int _spawnvpe(int modeflag,char *path,char *argv[],char *envp[]);
int _spawnve(int modeflag, char *path, char *argv[], char * envp[]);
char _strmatchtail(char *string, char *tail);
void _waitdrain(int length);
char _waitnocase(char *string, long length);
int _xsize(unsigned int, long *, long *);
int _xspawn(char *, char *, char *, VECTOR *, int, int, char *, int);
void add_to_buffer(char character, char remote);
void add_to_sequence(char *string,char value);
int addvect(int, int);
char as_char_for_pos(char character, unsigned char pos);
char *c2pasc(char *pasc_str,unsigned char max_pasc_len,char *c_str);
void chat(void);
int cmdenv(char **, char **, char *, char **, char **);
int doxspawn(char *, char **, char **);
void display_permaliteral(void);
void fill_area(char x1,char y1,char x2,char y2,register char character);
char *first_word(char *string,char *outstr);
int get_char(void);
unsigned int get_config_hex(char *string);
unsigned int get_config_unsigned(char *string);
unsigned long get_config_ulong(char *string);
void get_next_time(char **string_pointer);
void giveup_slice(void);
char is_true(char *string);
int is_valid_char_for_pos(char character, unsigned char pos);
unsigned long long_shift_left(unsigned long value, char amount);
unsigned long long_shift_right(unsigned long value, char amount);
char *makepath(char *path,char *filename);
char *other_words(char *string);
char *pasc2c(char *c_str,char *pasc_str,unsigned char max_len);
void *phys_createwin(int x1, int y1, int x2, int y2, char attr, char *title, char titleattr);
void phys_delwin(int x1, int y1, int x2, int y2, void *buffer);
int read_exitinfo_primitive(FILE *fp,int count);
void restore_cursor(void);
void savevect(void);
char search_for_infofile(char **filenames, int num_filenames, char *found, char *dir);
char search_in_dir(char **filenames, int num_filenames, char *found, char *dir);
void store_cursor(void);
int tempfile(char *, int *);
int testfile(char *, char *, int *);
void toname(char *string);
char *user_age(void);
int write_exitinfo_primitive(FILE *fp,int count);



/* Internal variables used throughout OpenDoors */
extern int _scrollaction;                                       /* odblock.c */
extern unsigned int _gt_val[3];                                 /* odcfile.c */
extern char _gt_num_vals;                                       /* odcfile.c */
extern char sysop_name_set;                                     /* odcfile.c */
extern char forced_sysop_name[40];                              /* odcfile.c */
extern char system_name_set;                                    /* odcfile.c */
extern char forced_system_name[40];                             /* odcfile.c */
extern int uart_data;                                             /* odcom.c */
extern int uart_ier;                                              /* odcom.c */
extern int uart_iir;                                              /* odcom.c */
extern int uart_lcr;                                              /* odcom.c */
extern int uart_mcr;                                              /* odcom.c */
extern int uart_lsr;                                              /* odcom.c */
extern int uart_msr;                                              /* odcom.c */
extern char com_installed;                                        /* odcom.c */
extern int  intnum;                                               /* odcom.c */
extern char i8259bit;                                             /* odcom.c */
extern char old_i8259_mask;                                       /* odcom.c */
extern int i8259mask_reg;                                         /* odcom.c */
extern int i8259eoi_reg;                                          /* odcom.c */
extern int i8259master_eoi_reg;                                   /* odcom.c */
extern char old_ier;                                              /* odcom.c */
extern char old_mcr;                                              /* odcom.c */
extern void interrupt (*old_vector)();                            /* odcom.c */
extern char using_fifo;                                           /* odcom.c */
extern unsigned char fifo_control;                                /* odcom.c */
extern int tx_queue_size;                                         /* odcom.c */
extern char *tx_queue;                                            /* odcom.c */
extern int tx_in;                                                 /* odcom.c */
extern int tx_out;                                                /* odcom.c */
extern int tx_chars;                                              /* odcom.c */
extern int rx_queue_size;                                         /* odcom.c */
extern char *rx_queue;                                            /* odcom.c */
extern int rx_in;                                                 /* odcom.c */
extern int rx_out;                                                /* odcom.c */
extern int rx_chars;                                              /* odcom.c */
extern long _last_kernel;                                        /* odcore.c */
extern char globworkstr[257];                                    /* odcore.c */
extern char block[2];                                            /* odcore.c */
extern char blankblock[2];                                       /* odcore.c */
extern char clearit[2];                                          /* odcore.c */
extern char backstr[4];                                          /* odcore.c */
extern char ansi_clear[3];                                       /* odcore.c */
extern char avatar_clear[2] ;                                    /* odcore.c */
extern char _yet;                                                /* odcore.c */
extern char ibm2ansii[8];                                        /* odcore.c */
extern struct phys_text_info user_text;                          /* odcore.c */
extern char _force_update;                                       /* odcore.c */
extern union REGS regs;                                          /* odcore.c */
extern time_t next_minute;                                       /* odcore.c */
extern time_t next_statup;                                       /* odcore.c */
extern time_t last_activity;                                     /* odcore.c */
extern unsigned char *input_buffer;                              /* odcore.c */
extern char _is_callback;                                        /* odcore.c */
extern char *input_remote;                                       /* odcore.c */
extern unsigned int _in_buf_size;                                /* odcore.c */
extern int b_head;                                               /* odcore.c */
extern char _doing_cs_hook;                                      /* odcore.c */
extern int b_tail;                                               /* odcore.c */
extern int last_inactivity_setting;                              /* odcore.c */
extern char colour_check_char;                                   /* odcore.c */
extern char *colour_end_pos;                                     /* odcore.c */
extern int (*_log_wrt)(int);                                     /* odcore.c */
extern void (*_log_close)(int);                                  /* odcore.c */
extern char inited;                                              /* odcore.c */
extern char kernal_active;                                       /* odcore.c */
extern char warned;                                              /* odcore.c */
extern char chatted;                                             /* odcore.c */
extern struct find_block _fblk;                                  /* odcore.c */
extern char _sfunc_pending;                                      /* odcore.c */
extern char last_status;                                         /* odcore.c */
extern char sysop_color;                                         /* odcore.c */
extern char sysop_key;                                           /* odcore.c */
extern char _multitasker;                                        /* odcore.c */
extern char status_line;                                         /* odcore.c */
extern char _last_control_key;                                   /* odcore.c */
extern void (*current_status_function)(unsigned char which);     /* odcore.c */
extern char _output_top;                                         /* odcore.c */
extern char _output_bottom;                                      /* odcore.c */
extern char _desired_personality[33];                            /* odcore.c */
extern int (*_set_personality)(char *name);                      /* odcore.c */
extern char ra_status_to_set;                                    /* odcore.c */
extern unsigned char _format_offset[80];                        /* odedstr.c */
extern char _format_literal[80];                                /* odedstr.c */
extern char _original_string[81];                               /* odedstr.c */
extern char *_input_string;                                     /* odedstr.c */
extern char *_format_string;                                    /* odedstr.c */
extern unsigned char _string_length;                            /* odedstr.c */
extern char _blank_character;                                   /* odedstr.c */
extern char ansi_color[8];                                        /* odemu.c */
extern char seq_level;                                            /* odemu.c */
extern int param[10];                                             /* odemu.c */
extern char param_str[4];                                         /* odemu.c */
extern char param_str_len;                                        /* odemu.c */
extern char saved_x;                                              /* odemu.c */
extern char saved_y;                                              /* odemu.c */
extern char repeat_str[129];                                      /* odemu.c */
extern unsigned char repeat_count;                                /* odemu.c */
extern char avt_seq;                                              /* odemu.c */
extern char prev_param;                                           /* odemu.c */
extern char num_params;                                           /* odemu.c */
extern char def_attr;                                             /* odemu.c */
extern char avt_insert;                                           /* odemu.c */
extern char scroll_lines;                                         /* odemu.c */
extern char scroll_x1;                                            /* odemu.c */
extern char scroll_y1;                                            /* odemu.c */
extern char scroll_x2;                                            /* odemu.c */
extern char scroll_y2;                                            /* odemu.c */
extern char *_config_colours[12];                               /* odiface.c */
extern char *_config_lines[LINES_SIZE];                         /* odiface.c */
extern char *_config_text[TEXT_SIZE];                           /* odiface.c */
extern char *_log_messages[14];                                 /* odiface.c */
extern unsigned int _node_number;                               /* odiface.c */
extern int is_cosys;                                            /* odiface.c */
extern int is_sysop;                                            /* odiface.c */
extern char *storestr[25];                                      /* odiface.c */
extern unsigned long _forced_bps;                               /* odiface.c */
extern char _forced_port;                                       /* odiface.c */
extern unsigned long _file_bps;                                 /* odiface.c */
extern char ipath[120];                                         /* odiface.c */
extern struct _ra2exitinfo *ra2exitinfo;                        /* odiface.c */
extern struct _exitinfo *exitinfo;                              /* odiface.c */
extern struct _ext_exitinfo *ext_exitinfo;                      /* odiface.c */
extern struct _pcbsys *pcbsys;                                  /* odiface.c */
extern struct _userssyshdr *userssyshdr;                        /* odiface.c */
extern struct _userssysrec *userssysrec;                        /* odiface.c */
extern char exitpath[120];                                      /* odiface.c */
extern int _initial_elapsed;                                    /* odiface.c */
extern unsigned filesizea;                                      /* odiface.c */
extern unsigned filesizeb;                                      /* odiface.c */
extern char _exitreason;                                        /* odiface.c */
extern char *_final_dir;                                        /* odiface.c */
extern char _preorexit;                                         /* odiface.c */
extern char doorsys_lock;                                       /* odiface.c */
extern time_t door_start;                                       /* odiface.c */
extern char _preset;                                            /* odiface.c */
extern int start_remaining;                                     /* odiface.c */
extern char _called_from_config;                                /* odiface.c */
extern char _preorexit;                                         /* odifsup.c */
extern char _ra_status;                                         /* odifsup.c */
extern FILE *logfile_pointer;                                     /* odlog.c */
extern unsigned char num_personalities;                         /* odmulti.c */
extern unsigned char current_personality;                       /* odmulti.c */
extern char _menlev;                                            /* odpopup.c */
extern char _menucommand;                                       /* odpopup.c */
extern char _menucorrect;                                       /* odpopup.c */
extern int _menuflags;                                          /* odpopup.c */
extern char _menuitems;                                         /* odpopup.c */
extern struct _menuinfo menuinfo[MENU_LEVELS];                  /* odpopup.c */
extern char _ra_was_on;                                            /* odra.c */
extern unsigned int phys_seg;                                    /* odscrn.c */
extern void far *phys_buffer;                                    /* odscrn.c */
extern unsigned char phys_curon;                                 /* odscrn.c */
extern unsigned char phys_curx;                                  /* odscrn.c */
extern unsigned char phys_cury;                                  /* odscrn.c */
extern unsigned char phys_page;                                  /* odscrn.c */
extern unsigned char phys_winleft;                               /* odscrn.c */
extern unsigned char phys_wintop;                                /* odscrn.c */
extern unsigned char phys_winright;                              /* odscrn.c */
extern unsigned char phy_winbottom;                              /* odscrn.c */
extern unsigned char phys_attrib;                                /* odscrn.c */
extern unsigned char phys_scroll;                                /* odscrn.c */
extern int _swap;                                               /* odspawn.c */
extern char *_swappath;                                         /* odspawn.c */
extern int _useems;                                             /* odspawn.c */
extern int _required;                                           /* odspawn.c */
extern char is_shell;                                           /* odspawn.c */
extern char **environ;                                         /* odswap.asm */
extern char _status_work_str[80];                                /* odstat.c */
extern char _cP_;                                                 /* odsys.c */
extern char *copyright;                                           /* odsys.c */
extern int copy_checksum;                                         /* odsys.c */
extern char reg_str[82];                                          /* odsys.c */
extern char *notreg_str;                                          /* odsys.c */
extern int notreg_checksum;                                       /* odsys.c */
