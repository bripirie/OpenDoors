/*
 * ÛÛÛÛÛÛÛÛÛÛ                         ÛÛÛÛÛÛÛÜ
 * ÛÛÛßßßßÛÛÛ                         ÛÛÛßßßÛÛÛ
 * ÛÛÛ    ÛÛÛ ÜÜÜÜÜÜÜ ÜÜÜÜÜÜÜ ÜÜÜÜÜÜÜ ÛÛÛ   ÛÛÛ ÜÜÜÜÜÜÜ ÜÜÜÜÜÜÜ ÜÜÜÜÜÜ ÜÜÜÜÜÜÜ
 * ÛÛÛ    ÛÛÛ ÛÛÛßÛÛÛ ÛÛÛßÛÛÛ ÛÛÛßÛÛÛ ÛÛÛ   ÛÛÛ ÛÛÛßÛÛÛ ÛÛÛßÛÛÛ ÛÛÛßßß ÛÛÛßßßß
 * ÛÛÛÜÜÜÜÛÛÛ ÛÛÛ ÛÛÛ ÛÛÛßßßß ÛÛÛ ÛÛÛ ÛÛÛÜÜÜÛÛÛ ÛÛÛ ÛÛÛ ÛÛÛ ÛÛÛ ÛÛÛ    ßßßßÛÛÛ
 * ÛÛÛÛÛÛÛÛÛÛ ÛÛÛÛÛÛÛ ÛÛÛÛÛÛÛ ÛÛÛ ÛÛÛ ÛÛÛÛÛÛÛß  ÛÛÛÛÛÛÛ ÛÛÛÛÛÛÛ ÛÛÛ    ÛÛÛÛÛÛÛ
 *            ÛÛÛ
 *            ÛÛÛ
 *            ßßß                                     Door Programming Toolkit
 * ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 *
 *      (C) Copyright 1991 - 1994 by Brian Pirie. All Rights Reserved.
 *
 *
 *
 *
 *     Filename : OPENDOOR.H
 *  Description : This header file contains information needed by a C compiler
 *                in order to permit your program to interface with OpenDoors.
 *                This file must be #included in any program using OpenDoors,
 *                and the program must be linked with the appropriate
 *                OpenDoors libary file (ODOORx.LIB).
 *      Version : 5.00
 */


#ifndef INC_OPENDOOR                           /* Only parse OPENDOOR.H once */
#define INC_OPENDOOR



                                          /* MULTIPLE COMPILER COMPATIBILITY */
                                          /* ------------------------------- */

#ifdef __cplusplus                /* Compiler-dependant function definitions */
#define FUNCDEF extern "C"
#else
#define FUNCDEF
#endif



                                        /* OPENDOORS API FUNCTION PROTOTYPES */
                                        /* --------------------------------- */

/* Programs interface with OpenDoors by calling any of the door toolkit
 * functions. A summary of these functions appears below, followed by the
 * function definition prototypes. Full information on these functions appears
 * in the OpenDoors manual. Functions denoted (ANS/AVT) require ANSI or AVATAR
 * display modes to be active.
 *
 * OUTPUT FUNCTIONS - TEXT DISPLAY
 *    od_printf()         - Performs formatted output, with colour settings
 *    od_disp_str()       - Displays a normal, NULL-terminated string.
 *    od_disp()           - Sends characters to modem, with/without local echo
 *    od_disp_emu()       - Displays a string, interpreting ANSI/AVATAR codes
 *    od_repeat()         - Efficiently displays a character repeatedly
 *    od_putch()          - Displays a single character.
 *
 * OUTPUT FUNCTIONS - COLOUR AND CURSOR CONTROL
 *    od_set_colour()     - Sets colour according to fore/background values
 *    od_set_attrib()     - Sets current colour to specified IBM-PC attribute
 *    od_set_cursor()     - Positions cursor on screen in ANSI & AVATAR modes
 *
 * OUTPUT FUNCTIONS - SCREEN MANIPULATION
 *    od_clr_scr()        - Clears the screen, if screen clearing enabled
 *    od_save_screen()    - Saves the contents of entire screen, in any mode
 *    od_restore_screen() - Restores the contents of entire screen, in any mode
 *
 * OUTPUT FUNCTIONS - BLOCK MANIPULATION
 *    od_clr_line()       - Clears the remainder of the current line
 *    od_gettext()        - Gets the contents of an area on the screen(ANS/AVT)
 *    od_puttext()        - Displays block retrieved with gettext()   (ANS/AVT)
 *    od_scroll()         - Scrolls a portion of the screen           (ANS/AVT)
 *
 * OUTPUT FUNCTIONS - WINDOWS & MENUS
 *    od_draw_box()       - Draws a box on the screen                 (ANS/AVT)
 *    od_window_create()  - Creates a window, storing underlying text (ANS/AVT)
 *    od_window_remove()  - Removes window, restoring underlying text (ANS/AVT)
 *    od_popup_menu()     - Displays popup menu with menu bar         (ANS/AVT)
 *
 * OUTPUT FUNCTIONS - FILE DISPLAY
 *    od_send_file()      - Displays an ASCII/ANSI/AVATAR/RIP file
 *    od_hotkey_menu()    - Displays ASCII/ANSI/AVATAR/RIP menu, with hotkeys
 *    od_list_files()     - Lists files available for download using FILES.BBS
 *
 * INPUT FUNCTIONS
 *    od_get_answer()     - Inputs a key, allowing only particular responses
 *    od_get_key()        - Inputs a key, optionally waiting for next keypress
 *    od_input_str()      - Inputs string of specified length from keyboard
 *    od_edit_str()       - Fancy formatted string input function     (ANS/AVT)
 *    od_clear_keybuffer()- Removes any waiting keys in keyboard input buffer
 *
 * COMMON DOOR ACTIVITY FUNCTIONS
 *    od_page()           - Allows user to page sysop
 *    od_spawn()          - Suspends OpenDoors & starts another program
 *    od_spawnvpe()       - Like od_spawn, but with more options
 *    od_log_write()      - Writes a logfile entry
 *
 * SPECIAL CONTROL FUNCTIONS
 *    od_init()           - Immediately begins door & sets up od_control struct
 *    od_colour_config()  - Translates colour configation line to colour value
 *    od_add_personality()- Adds another personality definition
 *    od_set_statusline() - Sets the current status line setting
 *    od_autodetect()     - Determines the remote system terminal type
 *    od_kernel()         - Call when not calling other functions for > 10 sec.
 *    od_exit()           - Ends a door program and returns to BBS
 *    od_carrier()        - Indicates whether remote connection is present
 *    od_set_dtr()        - Raises / lowers the DTR signal to the modem
 *    od_chat()           - Manually starts chat mode
 */

FUNCDEF int           od_add_personality(char *name, char output_top,
                                        char output_bottom,
                                        void (*function)(unsigned char which));
FUNCDEF void          od_autodetect(int flags);
FUNCDEF int           od_carrier(void);
FUNCDEF void          od_chat(void);
FUNCDEF void          od_clear_keybuffer(void);
FUNCDEF void          od_clr_line(void);
FUNCDEF void          od_clr_scr(void);
FUNCDEF unsigned char od_colour_config(char *config_line);
FUNCDEF void          od_disp(char *buffer, int size, char local_echo);
FUNCDEF void          od_disp_emu(char *string, char remote_echo);
FUNCDEF void          od_disp_str(char *string);
FUNCDEF int           od_draw_box(char left, char top, char right,
                                  char bottom);
FUNCDEF unsigned int  od_edit_str(char *input_string, char *format_string,
                                  int row, int col,
                                  unsigned char normal_colour,
                                  unsigned char highlight_colour,
                                  char character, unsigned int flags);
FUNCDEF void          od_exit(int errorlevel, char term_call);
FUNCDEF char          od_get_answer(char *string);
FUNCDEF int           od_get_key(int wait);
FUNCDEF int           od_gettext(int left, int top, int right, int bottom,
                                 void *block);
FUNCDEF char          od_hotkey_menu(char *filename, char *hotkeys, char wait);
FUNCDEF void          od_init(void);

FUNCDEF void          od_input_str(char *string, int max_len,
                                  unsigned char minchar,
                                  unsigned char maxchar);
FUNCDEF void          od_kernel(void);
FUNCDEF int           od_list_files(char *directory);
FUNCDEF int           od_log_write(char *message);
FUNCDEF void          od_page(void);
FUNCDEF int           od_popup_menu(char *title, char *text, int left, int top,
                                    int level, unsigned int flags);
FUNCDEF void          od_printf(char *format, ...);
FUNCDEF void          od_putch(int character);
FUNCDEF int           od_puttext(int left, int top, int right, int bottom,
                                 void *block);
FUNCDEF void          od_repeat(char value, unsigned char times);
FUNCDEF int           od_restore_screen(void *buffer);
FUNCDEF int           od_save_screen(void *buffer);
FUNCDEF int           od_scroll(int left, int top, int right, int bottom,
                                int distance, unsigned int flags);
FUNCDEF int           od_send_file(char *filename);
FUNCDEF void          od_set_attrib(int colour);
#define               od_set_colour(x, y) od_set_attrib(x|(y<<4))
FUNCDEF void          od_set_cursor(int row, int col);
FUNCDEF void          od_set_dtr(char high);
FUNCDEF int           od_set_personality(char *name);
FUNCDEF void          od_set_statusline(char setting);
FUNCDEF int           od_spawn(char *command_line);
FUNCDEF int           od_spawnvpe(int modeflag, char *path, char *argv[],
                                  char *envp[]);
FUNCDEF void *        od_window_create(int left, int top, int right,
                                       int bottom, char *title, char boardcol,
                                       char titlecol, char insidecol, 
                                       int reserved);
FUNCDEF int           od_window_remove(void *info);                                       



                                              /* OPENDOORS CONTROL STRUCTURE */
                                              /* --------------------------- */

                                        /* Force byte alignment, if possible */
#ifdef __TURBOC__                           /* If this is a Borland compiler */
#if(__TURBOC__>=0x295)         /* If this version supports the option pragma */
#pragma option -a-                    /* Turn off -a (word alignment) option */
#endif
#endif
#ifdef _MSC_VER                           /* If this is a Microsoft compiler */
#pragma pack(1)                           /* Set structure packing to 1 byte */
#endif

struct _od_control               /* OpenDoors door toolkit control structure */
   {
   char          info_path[60];         /* Location of door information file */


   unsigned long baud;                               /* Serial port settings */
   int           od_com_address;
   unsigned char od_com_irq;
   char          od_com_method;
   unsigned int  od_com_rx_buf;
   unsigned int  od_com_tx_buf;
   char          od_com_fifo_trigger;
   char          od_com_no_fifo;
   char          od_no_fossil;
   char          port;


   char          system_name[40];                /* Caller & BBS information */
   char          sysop_name[40];
   long          system_calls;
   char          system_last_caller[36];
   char          timelog_start_date[9];
   int           timelog_busyperhour[24];
   int           timelog_busyperday[7];

   char          user_name[36];
   char          user_location[26];
   char          user_password[16];
   char          user_dataphone[16];
   char          user_homephone[16];
   char          user_lasttime[6];
   char          user_lastdate[9];
   char          user_attribute;
   char          user_flags[4];
   unsigned long user_net_credit;
   unsigned long user_pending;
   unsigned int  user_messages;
   unsigned long user_lastread;
   unsigned int  user_security;
   unsigned long user_numcalls;
   unsigned long user_uploads;
   unsigned long user_downloads;
   unsigned long user_upk;
   unsigned long user_downk;
   unsigned long user_todayk;
   unsigned int  user_time_used;
   unsigned int  user_screen_length;
   char          user_last_pwdchange;
   char          user_attrib2;
   unsigned int  user_group;

   char          event_status;
   char          event_starttime[6];
   unsigned char event_errorlevel;
   char          event_days;
   char          event_force;
   char          event_last_run[9];

   char          user_netmailentered;
   char          user_echomailentered;
   char          user_logintime[6];
   char          user_logindate[9];
   int           user_timelimit;
   long          user_loginsec;
   long          user_credit;
   unsigned int  user_num;
   int           user_readthru;
   int           user_numpages;
   int           user_downlimit;
   char          user_timeofcreation[6];
   char          user_logonpassword[16];
   char          user_wantchat;
   char          user_ansi;

   int           user_deducted_time;
   char          user_menustack[50][9];
   char          user_menustackpointer;
   char          user_handle[36];
   char          user_comment[81];
   char          user_firstcall[9];
   char          user_combinedrecord[200];
   char          user_birthday[9];
   char          user_subdate[9];
   unsigned char user_screenwidth;
   unsigned char user_language;
   unsigned char user_date_format;
   char          user_forward_to[36];
   char          user_error_free;
   char          sysop_next;
   char          user_emsi_session;
   char          user_emsi_crtdef[41];
   char          user_emsi_protocols[41];
   char          user_emsi_capabilities[41];
   char          user_emsi_requests[41];
   char          user_emsi_software[41];
   unsigned char user_hold_attr1;
   unsigned char user_hold_attr2;
   unsigned char user_hold_len;

   char          user_reasonforchat[78];
   char          user_callsign[12];
   unsigned int  user_msg_area;
   unsigned int  user_file_area;
   char          user_protocol;
   unsigned int  user_file_group;
   unsigned char user_last_birthday_check;
   char          user_sex;
   unsigned long user_xi_record;
   unsigned int  user_msg_group;
   char          user_avatar;
   char          user_org[51];
   char          user_address[3][51];
   long          user_pwd_crc;
   long          user_logon_pwd_crc;
   char          user_last_cost_menu[9];
   unsigned int  user_menu_cost;
   char          user_rip;


   char          od_info_type;           /* Door information file statistics */
   char          od_extended_info;
   unsigned int  od_node;
   char          od_ra_info;


   char          od_always_clear;                           /* Door settings */
   char          od_force_local;
   char          od_chat_active;
   char          od_current_statusline;
   int           od_error;
   char          od_last_input;
   char          od_logfile_disable;
   char          od_logfile_name[80];
   unsigned int  od_maxtime;
   int           od_maxtime_deduction;
   char          od_okaytopage;
   int           od_pagestartmin;
   int           od_pageendmin;
   char          od_page_pausing;
   char          od_page_statusline;
   char          od_user_keyboard_on;
   char          od_update_status_now;
   int           od_cur_attrib;

    
   char          od_box_chars[8];       /* OpenDoors customization variables */
   char          od_cfg_text[47][33];
   char          od_cfg_lines[25][33];
   void          (*od_config_file)(void);
   char *        od_config_filename;
   void          (*od_config_function)(char *keyword, char *options);
   char          od_colour_char;
   char          od_colour_delimiter;
   char          od_colour_names[12][33];
   char          od_clear_on_exit;
   void          (*od_default_personality)(unsigned char operation);
   char          od_default_rip_win;
   unsigned int  od_disable;
   unsigned char od_errorlevel[8];
   char          od_full_colour;
   char          od_full_put;
   unsigned int  od_in_buf_size;
   int           od_inactivity;
   int           od_inactive_warning;
   char          od_list_pause;
   char          od_list_stop;
   void          (*od_logfile)(void);
   char          *od_logfile_messages[14];
   void          (*od_mps)(void);
   char          od_nocopyright;
   char          od_noexit;
   char          od_no_ra_codes;
   char          od_page_len;
   unsigned long od_reg_key;
   char          od_reg_name[36];
   char          od_status_on;
   char          od_spawn_freeze_time;
   char          od_swapping_disable;
   char          od_swapping_noems;
   char          od_swapping_path[80];
   char          od_prog_name[40];
   void          (*od_no_file_func)(void);

   void          (*od_before_exit)(void);           /* Custom function hooks */
   void          (*od_cbefore_chat)(void);
   void          (*od_cafter_chat)(void);
   void          (*od_cbefore_shell)(void);
   void          (*od_cafter_shell)(void);
   void          (*od_time_msg_func)(char *string);
   void          (*od_ker_exec)(void);
   void          (*od_local_input)(int key);


   unsigned int  key_chat;          /* OpenDoors function key customizations */   unsigned int        key_dosshell;
   unsigned int  key_drop2bbs;
   unsigned int  key_hangup;
   unsigned int  key_keyboardoff;
   unsigned int  key_lesstime;
   unsigned int  key_lockout;
   unsigned int  key_moretime;
   unsigned int  key_status[9];
   unsigned int  key_sysopnext;

   char          od_num_keys;                    /* Additional function keys */
   int           od_hot_key[16];
   int           od_last_hot;
   void          (*od_hot_function[16])(void);


   char *        od_after_chat;           /* OpenDoors prompt customizations */
   char *        od_after_shell;
   char *        od_before_chat;
   char *        od_before_shell;
   char *        od_chat_reason;
   char *        od_day[7];
   char *        od_hanging_up;
   char *        od_help_text;
   char *        od_help_text2;
   char *        od_inactivity_timeout;
   char *        od_inactivity_warning;
   char *        od_month[12];
   char *        od_no_keyboard;
   char *        od_no_sysop;
   char *        od_no_response;
   char *        od_no_time;
   char *        od_offline;
   char *        od_paging;
   char *        od_press_key;
   char *        od_sending_rip;
   char *        od_status_line[3];
   char *        od_sysop_next;
   char *        od_time_left;
   char *        od_time_warning;
   char *        od_want_chat;

   char *        od_continue;
   char          od_continue_yes;
   char          od_continue_no;
   char          od_continue_nonstop;


   char          od_chat_colour1;         /* OpenDoors colour customizations */
   char          od_chat_colour2;
   char          od_list_comment_col;
   char          od_list_name_col;
   char          od_list_offline_col;
   char          od_list_size_col;
   char          od_list_title_col;
   char          od_continue_col;
   char          od_menu_title_col;
   char          od_menu_border_col;
   char          od_menu_text_col;
   char          od_menu_key_col;
   char          od_menu_highlight_col;
   char          od_menu_highkey_col;
   };

extern struct _od_control od_control;        /* od_control external variable */

#ifdef _MSC_VER                           /* If this is a Microsoft compiler */
#pragma pack()                      /* Set structure packing back to default */
#endif



                                                       /* MANIFEST CONSTANTS */
                                                       /* ------------------ */

#ifndef FALSE                                /* Boolean constant definitions */
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#define MAYBE 2

#define P_WAIT                  0                     /* od_spawnvpe() flags */
#define CURRENT                 0
#define IRET                    1
       
#define EDIT_FLAG_NORMAL        0x0000                /* od_edit_str() flags */
#define EDIT_FLAG_NO_REDRAW     0x0001
#define EDIT_FLAG_FIELD_MODE    0x0002
#define EDIT_FLAG_EDIT_STRING   0x0004
#define EDIT_FLAG_STRICT_INPUT  0x0008
#define EDIT_FLAG_PASSWORD_MODE 0x0010
#define EDIT_FLAG_ALLOW_CANCEL  0x0020
#define EDIT_FLAG_FILL_STRING   0x0040
#define EDIT_FLAG_AUTO_ENTER    0x0080
#define EDIT_FLAG_AUTO_DELETE   0x0100
#define EDIT_FLAG_KEEP_BLANK    0x0200
#define EDIT_FLAG_PERMALITERAL  0x0400
#define EDIT_FLAG_LEAVE_BLANK   0x0800
       
#define EDIT_RETURN_ERROR       0             /* od_edit_str() return values */
#define EDIT_RETURN_CANCEL      1
#define EDIT_RETURN_ACCEPT      2
#define EDIT_RETURN_PREVIOUS    3
#define EDIT_RETURN_NEXT        4

#define MENU_NORMAL             0x0000        /* od_popup_menu() flag values */
#define MENU_ALLOW_CANCEL       0x0001
#define MENU_PULLDOWN           0x0002
#define MENU_KEEP               0x0004
#define MENU_DESTROY            0x0008

#define DETECT_NORMAL           0x0000        /* od_autodetect() flag values */

#define SCROLL_NORMAL           0x0000                  /* od_scroll() flags */
#define SCROLL_NO_CLEAR         0x0001

#define STATUS_NORMAL           0          /* OpenDoors status line settings */
#define STATUS_NONE             8
#define STATUS_ALTERNATE_1      1
#define STATUS_ALTERNATE_2      2
#define STATUS_ALTERNATE_3      3
#define STATUS_ALTERNATE_4      4
#define STATUS_ALTERNATE_5      5
#define STATUS_ALTERNATE_6      6
#define STATUS_ALTERNATE_7      7

#define D_BLACK                 0            /* OpenDoors colour definitions */
#define D_BLUE                  1
#define D_GREEN                 2
#define D_CYAN                  3
#define D_RED                   4
#define D_MAGENTA               5
#define D_BROWN                 6
#define D_GREY                  7
#define L_BLACK                 8
#define L_BLUE                  9
#define L_GREEN                 10
#define L_CYAN                  11
#define L_RED                   12
#define L_MAGENTA               13
#define L_YELLOW                14
#define L_WHITE                 15
#define B_BLACK                 L_BLACK
#define B_BLUE                  L_BLUE
#define B_GREEN                 L_GREEN
#define B_CYAN                  L_CYAN
#define B_RED                   L_RED
#define B_MAGENTA               L_MAGENTA
#define B_BROWN                 L_YELLOW
#define B_GREY                  L_WHITE

                  /* Door information file formats (od_control.od_info_type) */
#define DORINFO1              0                              /* DORINFO?.DEF */
#define EXITINFO              1     /* QBBS 2.6? EXITINFO.BBS & DORINFO?.DEF */
#define RA1EXITINFO           2       /* RA 1.?? EXITINFO.BBS & DORINFO?.DEF */
#define CHAINTXT              3                                 /* CHAIN.TXT */
#define SFDOORSDAT            4                               /* SFDOORS.DAT */
#define CALLINFO              5                              /* CALLINFO.BBS */
#define DOORSYS_GAP           6                     /* GAP/PC-Board DOOR.SYS */
#define DOORSYS_DRWY          7                          /* DoorWay DOOR.SYS */
#define QBBS275EXITINFO       8               /* QuickBBS 2.75+ EXITINFO.BBS */
#define CUSTOM                9                /* User-defined custom format */
#define DOORSYS_WILDCAT       10                        /* WildCat! DOOR.SYS */
#define RA2EXITINFO           11                    /* RA 2.00+ EXITINFO.BBS */
#define NO_DOOR_FILE          100      /* No door information file was found */

                                         /* Error type (od_control.od_error) */
#define ERR_NONE              0                              /* No error yet */
#define ERR_MEMORY            1          /* Unable to allocate enough memory */
#define ERR_NOGRAPHICS        2    /* Function requires ANSI/AVATAR/RIP mode */
#define ERR_PARAMETER         3    /* Invalid value was passed to a function */
#define ERR_FILEOPEN          4                       /* Unable to open file */
#define ERR_LIMIT             5       /* An internal limit has been exceeded */
#define ERR_FILEREAD          6                  /* Unable to read from file */
#define ERR_NOREMOTE          7  /* Function may not be called in local mode */

#define ERRORLEVEL_ENABLE     0         /* od_control.od_errorlevel indicies */
#define ERRORLEVEL_CRITICAL   1
#define ERRORLEVEL_NOCARRIER  2
#define ERRORLEVEL_HANGUP     3
#define ERRORLEVEL_TIMEOUT    4
#define ERRORLEVEL_INACTIVITY 5
#define ERRORLEVEL_DROPTOBBS  6
#define ERRORLEVEL_NORMAL     7

#define POPUP_ERROR           -1   /* Special od_popup_menu() return values. */
#define POPUP_ESCAPE          0
#define POPUP_LEFT            -2
#define POPUP_RIGHT           -3

#define BOX_UPPERLEFT         0               /* od_box_chars array indicies */
#define BOX_TOP               1
#define BOX_UPPERRIGHT        2
#define BOX_LEFT              3
#define BOX_LOWERLEFT         4
#define BOX_LOWERRIGHT        5
#define BOX_BOTTOM            6
#define BOX_RIGHT             7

                   /*  Method used for serial I/O (od_control.od_com_method) */
#define COM_FOSSIL            1
#define COM_INTERNAL          2

                              /* Optional component initialization functions */
FUNCDEF void option_config(void);
FUNCDEF void option_logfile(void);
FUNCDEF void option_mps(void);

#define INCLUDE_CONFIG_FILE   option_config             /* Optional settings */
#define NO_CONFIG_FILE        NULL
#define INCLUDE_LOGFILE       option_logfile
#define NO_LOGFILE            NULL
#define INCLUDE_MPS           option_mps
#define NO_MPS                NULL

                                 /* Built-in personality defintion functions */
FUNCDEF void pdef_opendoors(unsigned char operation);
FUNCDEF void pdef_pcboard(unsigned char operation);
FUNCDEF void pdef_ra(unsigned char operation);
FUNCDEF void pdef_wildcat(unsigned char operation);

                                         /* Personality definition constants */
#define PER_OPENDOORS         pdef_opendoors
#define PER_PCBOARD           pdef_pcboard
#define PER_RA                pdef_ra
#define PER_WILDCAT           pdef_wildcat

#define DIS_INFOFILE          0x0001                     /* od_disable flags */
#define DIS_CARRIERDETECT     0x0002
#define DIS_TIMEOUT           0x0004
#define DIS_LOCAL_OVERRIDE    0x0008
#define DIS_BPS_SETTING       0x0010
#define DIS_LOCAL_INPUT       0x0020

#define ES_DELETED            0                     /* Event status settings */
#define ES_ENABLED            1
#define ES_DISABLED           2

#define PEROP_DISPLAY1        0                    /* Personality operations */
#define PEROP_DISPLAY2        1
#define PEROP_DISPLAY3        2
#define PEROP_DISPLAY4        3
#define PEROP_DISPLAY5        4
#define PEROP_DISPLAY6        5
#define PEROP_DISPLAY7        6
#define PEROP_DISPLAY8        7
#define PEROP_DISPLAY9        8
#define PEROP_DISPLAY10       9
#define PEROP_UPDATE1         10
#define PEROP_UPDATE2         11
#define PEROP_UPDATE3         12
#define PEROP_UPDATE4         13
#define PEROP_UPDATE5         14
#define PEROP_UPDATE6         15
#define PEROP_UPDATE7         16
#define PEROP_UPDATE8         17
#define PEROP_UPDATE9         18
#define PEROP_UPDATE10        19
#define PEROP_INITIALIZE      20
#define PEROP_CUSTOMKEY       21
#define PEROP_DEINITIALIZE    22



                                         /* ALTERNATIVE SPELLING DEFINITIONS */
                                         /* -------------------------------- */

#define od_color_config                od_colour_config
#define od_chat_color1                 od_chat_colour1
#define od_chat_color2                 od_chat_colour2
#define od_set_color(x, y)             od_set_attrib(x|(y<<4))
#define od_color_char                  od_colour_char
#define od_color_delimiter             od_colour_delimiter
#define od_config_colors               od_config_colours
#define od_color_names                 od_colour_names
#define od_full_color                  od_full_colour



                                      /* BACKWARDS COMPATIBILITY DEFINITIONS */
                                      /* ----------------------------------- */

#define key_help                       key_status[6]
#define key_nohelp                     key_status[0]
#define user_credit                    user_net_credit
#define caller_netmailentered          user_netmailentered
#define caller_echomailentered         user_echomailentered
#define caller_logintime               user_logintime
#define caller_logindate               user_logindate
#define caller_timelimit               user_timelimit
#define caller_loginsec                user_loginsec
#define caller_credit                  user_credit
#define caller_userrecord              user_num
#define caller_readthru                user_readthru
#define caller_numpages                user_numpages
#define caller_downlimit               user_downlimit
#define caller_timeofcreation          user_timeofcreation
#define caller_logonpassword           user_logonpassword
#define caller_wantchat                user_wantchat
#define caller_ansi                    user_ansi
#define ra_deducted_time               user_deducted_time
#define ra_menustack                   user_menustack
#define ra_menustackpointer            user_menustackpointer
#define ra_userhandle                  user_handle
#define ra_comment                     user_comment
#define ra_firstcall                   user_firstcall
#define ra_combinedrecord              user_combinedrecord
#define ra_birthday                    user_birthday
#define ra_subdate                     user_subdate
#define ra_screenwidth                 user_screenwidth
#define ra_msg_area                    user_msg_area
#define ra_file_area                   user_file_area
#define ra_language                    user_language
#define ra_date_format                 user_date_format
#define ra_forward_to                  user_forward_to
#define ra_error_free                  user_error_free
#define ra_sysop_next                  sysop_next
#define ra_emsi_session                user_emsi_session
#define ra_emsi_crtdef                 user_emsi_crtdef
#define ra_emsi_protocols              user_emsi_protocols
#define ra_emsi_capabilities           user_emsi_capabilities
#define ra_emsi_requests               user_emsi_requests
#define ra_emsi_software               user_emsi_software
#define ra_hold_attr1                  user_hold_attr1
#define ra_hold_attr2                  user_hold_attr2
#define ra_hold_len                    user_hold_len
#define caller_usernum                 user_num
#define caller_callsign                user_callsign
#define caller_sex                     user_sex
#define od_avatar                      user_avatar
#define B_YELLOW                       L_YELLOW
#define B_WHITE                        L_WHITE
#define od_rbbs_node                   od_node
#define STATUS_USER1                   STATUS_ALTERNATE_1
#define STATUS_USER2                   STATUS_ALTERNATE_2
#define STATUS_USER3                   STATUS_ALTERNATE_3
#define STATUS_USER4                   STATUS_ALTERNATE_4
#define STATUS_SYSTEM                  STATUS_ALTERNATE_5
#define STATUS_HELP                    STATUS_ALTERNATE_7
#define od_init_with_config(filename,function)\
                                  od_control.config_file=INCLUDE_CONFIG_FILE;\
                                  od_control.config_filename=filename;\
                                  od_control.config_function=function;\
                                  od_init()
#define od_registered_to               od_control.od_reg_name
#define od_registration_key            od_control.od_reg_key
#define od_program_name                od_control.od_prog_name
#define od_log_messages                od_control.od_logfile_messages
#define od_config_text                 od_control.od_cfg_text
#define od_config_lines                od_control.od_cfg_lines
#define od_config_colours              od_control.od_colour_names
#define config_file                    od_config_file
#define config_filename                od_config_filename
#define config_function                od_config_function
#define default_personality            od_default_personality
#define logfile                        od_logfile
#define mps                            od_mps
#define od_kernal                      od_kernel
FUNCDEF int                            od_log_open(void);
FUNCDEF void                           od_emulate(register char in_char);

#endif /* INC_OPENDOOR */
