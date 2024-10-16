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
 *     Filename : ODSPAWN.C
 *  Description : Contains the C code for the spawning and memory swapping
 *                routines.
 *      Version : 5.00
 */



#include "opendoor.h"                  /* OpenDoor global structure & prototypes */
#include "odintern.h"

#include<stdlib.h>                     /* Standard header files */
#include<dir.h>
#include<string.h>
#include<ctype.h>
#include<time.h>
#include<stdio.h>
#include<errno.h>        /* Needed for error table */

#ifndef USEINLINE
#include<dos.h>
#endif



int _swap=0;                           /* if 0, do swap */
char *_swappath=NULL;                  /* swap path */
int _useems=0;                         /* if 0, use EMS */
int _required=0;                       /* child memory requirement in K */
static long swapsize;                  /* swap size requirement in bytes */
static int ems=2;                      /* if 0, EMS is available */
static int mapsize;                    /* size of page map information */
static unsigned int tempno=1;          /* tempfile number */
static char errtab[]=                  /* error table */
   {
   0,
   EINVAL,
   ENOENT,
   ENOENT,
   EMFILE,
   EACCES,
   EBADF,
   ENOMEM,
   ENOMEM,
   ENOMEM,
   E2BIG,
   ENOEXEC,
   EINVAL,
   EINVAL,
   -1,
   EXDEV,
   EACCES,
   EXDEV,
   ENOENT,
   -1};

static VECTOR vectab1[]=
   {
    0,    1,     0,  0,
    1,    1,     0,  0,
    2,    1,     0,  0,
    3,    1,     0,  0,
    0x1B, 1,     0,  0,
    0x23, 1,     0,  0,
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    2,     0,  0,                /* free record */
    0,    3,     0,  0                 /* end record */
};

static VECTOR vectab2[(sizeof vectab1)/(sizeof vectab1[0])];


char is_shell;


int od_spawn(char *command_line)       /* OpenDoors quick-spawn function */
   {
   char *argv[4];
   int rc;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_spawn()");

   *argv=getenv("COMSPEC");

   argv[1]="/c";
   argv[2]=command_line;
   argv[3]=NULL;

   if(*argv!=NULL)
      {
      if((rc=od_spawnvpe(P_WAIT,*argv,argv,NULL))!=-1 || errno != ENOENT )
         {
         return(rc!=-1);
         }
      }

   *argv="command.com";

   return(od_spawnvpe(P_WAIT,*argv,argv,NULL)!=-1);
   }


                                       /* Full-featured spawn function */
int od_spawnvpe(int modeflag, char *path, char *argv[], char *envp[])
   {
   char *directory;
   char *screen_buffer;
   int to_return;
   time_t start_time;
   int drive;
   unsigned long quotient;

   /* Log function entry if running in trace mode */
   TRACE(TRACE_API, "od_spawnvpe()");

   if(!inited) od_init();              /* verify that we've been initialized */

   if((screen_buffer=malloc(4000))==NULL) return(-1);
   if((directory=malloc(256))==NULL)
      {
      free(screen_buffer);
      return(-1);
      }

   store_cursor();                     /* store cursor, color, etc settings */
   phys_window(1,1,80,25);                  /* set window to full screen */
   phys_gettext(1,1,80,25,(char *)screen_buffer);  /* store contents of screen */
   phys_setattrib(0x07);                     /* set current color to grey */
   if(od_control.od_clear_on_exit)
      {
      phys_clrscr();
      }
   else
      {
      phys_gotoxy(1,1);
      }

   strcpy(directory,"X:\\");
   directory[0]='A'+(drive = _getdrv());
   _getcd(0,(char *)directory+3);      /* store current directory */

   start_time=time(NULL);

   _waitdrain(192);     /* Wait up to ten seconds for bufffer to drain */

   _com_close();                       /* close serial port */

                                       /* execute command */
   to_return=_spawnvpe(modeflag,path,argv,envp);

   _com_ini();                         /* re-open serial port */

   if(!(is_shell || od_control.od_spawn_freeze_time))
     {
     _ulongdiv(&quotient, NULL, time(NULL)-start_time, 60L);
     od_control.user_timelimit-=(int)quotient;
     }
   else
     {
     next_minute += time(NULL)-start_time;
     }

   last_activity=time(NULL);

   phys_puttext(1,1,80,25,(char *)screen_buffer);  /* redisplay the door screen */
   restore_cursor();                   /* restore cursor to old position */
   od_clear_keybuffer();               /* clear inbound buffer */

   _setdrvcd(drive, directory);

   free(screen_buffer);                /* de-allocate allocated space */
   free(directory);

   return(to_return);                  /* return specified value */
   }



int _spawnvpe(int modeflag,char *path,char *argv[],char *envp[])
   {
   register char *e;
   register char *p;
   char buf[80];
   int rc;


   _swappath=(char *)(strlen(od_control.od_swapping_path)==0 ? NULL : (char *)od_control.od_swapping_path);
   _useems=od_control.od_swapping_noems;
   _swap=od_control.od_swapping_disable;

   if((rc=_spawnve(modeflag, path, argv, envp))!=-1
       || errno!=ENOENT || *path=='\\' || *path=='/'
       || *path && *(path+1)==':' || (e=getenv("PATH"))==NULL)
      {
      return(rc);
      }

   for (;;e++)
      {
      if((p=strchr(e,';'))!=NULL)
         {
         if(p-e > 66)
            {
            e=p;
            continue;
            }
         }
      else if(strlen(e)>66)
         {
         return( -1 );
         }

      p=buf;

      while(*e && *e!=';') *p++=*e++;

      if(p>buf)
         {
         if(*(p-1)!='\\' && *(p-1)!='/') *p++ = '\\';
         strcpy(p,path);

         if((rc=_spawnve(modeflag,buf,argv,envp))!=-1 || errno!=ENOENT)
            {
            return(rc);
            }
         }
      if(*e=='\0') return(-1);
      }
   }



int addvect( number, opcode )
int number;
int opcode;
{
    register VECTOR *vect = vectab1;

    if ( number < 0 || number > 0xFF ||
        ( opcode != IRET && opcode != CURRENT ))
    {
        errno = EINVAL;
        return( -1 );
    }

    /* see if number is already in table */
    while ( vect->flag != 3 && ( vect->flag == 2 ||
        vect->number != ( char )number ))
    {
        vect++;
    }

    if ( vect->flag == 3 )
    {
        /* look for a free record */
        vect = vectab1;
        while ( vect->flag == CURRENT || vect->flag == IRET )
            vect++;
    }

    if ( vect->flag != 3 )
    {
        vect->number = ( char )number;
        vect->flag = ( char )opcode;
        if ( opcode == CURRENT )
        _getvect( number, &vect->vseg, &vect->voff );
        return( 0 );
    }

    errno = ENOMEM;
    return( -1 );
}

static void savevect()
{
    register VECTOR *vect1 = vectab1;
    register VECTOR *vect2 = vectab2;

    while ( vect1->flag != 3 )
    {
        if ( vect1->flag != 2 )
        {
            vect2->number = vect1->number;
            vect2->flag = CURRENT;
            _getvect( vect1->number, &vect2->vseg, &vect2->voff );
        }
        else
            vect2->flag = 2;           /* free */
        vect1++;
        vect2++;
    }
    vect2->flag = 3;                   /* end */
}

static int testfile( p, file, handle )
register char *p;
register char *file;
int *handle;
{
    unsigned int startno = tempno;
    int drive = ( *file | 32 ) - 96;   /* a = 1, b = 2, etc. */
    int root;
    unsigned int bytes;                /* bytes per cluster */
    unsigned int clusters;             /* free clusters */
    int need;                          /* clusters needed for swap file */
    int rc;                            /* return code */
    unsigned long quotient;
    unsigned long remainder;

    if ( file + 2 == p )
    {
        *p++ = '\\';
        if ( _getcd( drive, p ))       /* get current directory */
            return( 1 );               /* invalid drive */
        p = file + strlen( file );
    }
    else
    {
        *p = '\0';
        if ( _accessmode( file, 0 ))
            return( 1 );               /* path does not exist */
    }
    if ( *( p - 1 ) != '\\' && *( p - 1 ) != '/' )
        *p++ = '\\';
    if ( p - file == 3 )
        root = 1;                      /* is root directory */
    else
        root = 0;                      /* is not root directory */
    strcpy( p, "swp" );
    p += 3;

    if ( _dskspace( drive, &bytes, &clusters ) != 0 )
        return( 1 );                   /* invalid drive */



    _ulongdiv(&quotient, &remainder, swapsize, bytes);
    need = (int)quotient;

    if ( remainder )
        need++;
    if ( root == 0 )                   /* if subdirectory */
        need++;                        /* in case the directory needs space */
    if ( clusters < ( unsigned int )need )
        return( 1 );                   /* insufficient free disk space */

    do
    {
again:  tempno = ( ++tempno ) ? tempno : 1;
        if ( tempno == startno )
            return( 1 );               /* extremely unlikely */
        ltoa(( long )tempno, p, 10 );
    }
    while ( !_accessmode( file, 0 ));

/*
 *  The return code from _create will equal 80 if the user is running DOS 3.0
 *  or above and the file was created by another program between the access
 *  call and the _create call.
 */

    if (( rc = _create( file, handle )) == 80 )
        goto again;
    return( rc );
}

static int tempfile( file, handle )
char *file;
int *handle;
{
    register char *s = _swappath;
    register char *p = file;

    if ( s )
    {
        for ( ;; s++ )
        {
            while ( *s && *s != ';' )
                *p++ = *s++;
            if ( p > file )
            {
                if ( p == file + 1 || file[ 1 ] != ':' )
                {
                    memmove( file + 2, file, ( int )( p - file ));
                    *file = ( char )( _getdrv() + 'a' );
                    file[ 1 ] = ':';
                    p += 2;
                }
                if ( testfile( p, file, handle ) == 0 )
                    return( 0 );
                p = file;
            }
            if ( *s == '\0' )
                break;
        }
    }
    else                               /* try the current directory */
    {
        *p++ = ( char )( _getdrv() + 'a' );
        *p++ = ':';
        if ( testfile( p, file, handle ) == 0 )
            return( 0 );
    }

    errno = EACCES;
    return( 1 );
}

static int cmdenv( argv, envp, command, env, memory )
char **argv;
char **envp;
char *command;
char **env;
char **memory;
{
    register char **vp;
    unsigned int elen = 0;             /* environment length */
    char *p;
    int cnt;
    int len;

    /* construct environment */

    if ( envp == NULL )
    {
       char far *parent_env;
       char far *env_ptr;
       int nul_count;

       ASM mov ah, 0x62
       ASM int 0x21
       ASM push es
       ASM mov es, bx
       ASM mov ax, es:[0x2c]
       ASM pop es
       ASM mov word ptr parent_env, 0
       ASM mov word ptr parent_env + 2, ax

       env_ptr = parent_env;
       nul_count = 0;
       while(nul_count < 2)
       {
          if(*env_ptr)
          {
             nul_count = 0;
          }
          else
          {
             ++nul_count;
          }

          ++env_ptr;
          ++elen;
       }

       if ( elen > 32766 )        /* 32K - 2 */
       {
          errno = E2BIG;
          return( -1 );
       }

       if (( p = malloc(elen + 15 )) == NULL )
       {
           errno = ENOMEM;
           return( -1 );
       }
       *memory = p;

       *( unsigned int * )&p = *( unsigned int * )&p + 15 & ~15;
       *env = p;

       len = elen;
       while(len--)
       {
          *p++ = *parent_env++;
       }
    }
    else
    {
       for ( vp = envp; *vp; vp++ )
       {
           elen += strlen( *vp ) + 1;
           if ( elen > 32766 )        /* 32K - 2 */
           {
               errno = E2BIG;
               return( -1 );
           }
       }

       if (( p = malloc( ++elen + 15 )) == NULL )
       {
           errno = ENOMEM;
           return( -1 );
       }
       *memory = p;

       *( unsigned int * )&p = *( unsigned int * )&p + 15 & ~15;
       *env = p;

       for ( vp = envp; *vp; vp++ )
           p = strchr( strcpy( p, *vp ), '\0' ) + 1;

       *p = '\0';                         /* final element */
    }


    /* construct command-line */

    vp = argv;
    p = command + 1;
    cnt = 0;

    if (vp!=NULL &&  *vp )
    {
        while ( *++vp )
        {
            *p++ = ' ';
            cnt++;
            len = strlen( *vp );
            if ( cnt + len > 125 )
            {
                errno = E2BIG;
                free( *memory );
                return( -1 );
            }
            strcpy( p, *vp );
            p += len;
            cnt += len;
        }
    }

    *p = '\r';
    *command = ( char )cnt;

    return(( int )elen );              /* return environment length */
}

static int doxspawn( path, argv, envp )
char *path;                      /* file to be executed */
char *argv[];                    /* array of pointers to arguments */
char *envp[];                    /* array of pointers to environment strings */
{
    register int rc = 0;               /* assume do xspawn */
    int doswap = 0;                    /* assume do swap */
    int elen;                          /* environment length */
    char *memory;
    char *env;                         /* environment */
    char command[ 128 ];               /* command-line */
    long totalsize;                    /* parent and free memory in bytes */
    int handle;
    int pages;
    char file[ 79 ];
    char *mapbuf = NULL;               /* buffer for map information */

    /* construct the command-line and the environment */
    if (( elen = cmdenv( argv, envp, command, &env, &memory )) == -1 )
        return( -1 );

    if ( _swap == 0 )
    {
        if ( _useems == 0 )
        {
            if ( ems == 2 )
                ems = _chkems( "EMMXXXX0", &mapsize );
            if ( ems == 0 && ( mapbuf = malloc( mapsize )) == NULL )
            {
                errno = ENOMEM;
                free( memory );
                return( -1 );
            }
        }
        if (( rc = _xsize( _psp, &swapsize, &totalsize )) == 0 )
        {
            if ( _required == 0 ||
                totalsize - swapsize - 272 < long_shift_left(( long )_required , 10 ))
            {
                if ( ems == 0 && _useems == 0 )
                {
                    pages = ( int )long_shift_right( swapsize , 14);
                    if (long_shift_left(( long )pages , 14 ) < swapsize )
                        pages++;
                    if ( _savemap( mapbuf ) == 0 &&
                        _getems( pages, &handle ) == 0 )
                        *file = '\0';  /* use EMS */
                    else if ( tempfile( file, &handle ) != 0 )
                        rc = -1;       /* don't do xspawn */
                }
                else if ( tempfile( file, &handle ) != 0 )
                    rc = -1;           /* don't do xspawn */
            }
            else
                doswap = 1;            /* don't do swap */
        }
        else
        {
            errno = errtab[ rc ];
            rc = -1;                   /* don't do xspawn */
        }
    }
    else
        doswap = 1;                    /* don't do swap */

    if ( rc == 0 )
    {
        savevect();                    /* save current vectors */
        rc = _xspawn( path, command, env, vectab1, doswap, elen, file,
            handle );
        _setvect( vectab2 );           /* restore saved vectors */
            if ( rc == 0 )
                    rc = _getrc();             /* get child return code */
        else
        {
            errno = errtab[ rc ];
            rc = -1;
        }
        /*
         *  If EMS was used, restore the page-mapping state of the expanded
         *  memory hardware.
         */
        if ( doswap == 0 && *file == '\0' && _restmap( mapbuf ) != 0 )
        {
            errno = EACCES;
            rc = -1;
        }
    }

    if ( mapbuf )
        free( mapbuf );
    free( memory );
    return( rc );
}

int _spawnve(int modeflag, char *path, char *argv[], char * envp[])
{
    register char *p;
    char *s;
    int rc = -1;
    char buf[ 80 ];

    if ( modeflag != P_WAIT )
    {
        errno = EINVAL;
        return( -1 );
    }

    p = strrchr( path, '\\' );
    s = strrchr( path, '/' );
    if ( p == NULL && s == NULL )
        p = path;
    else if ( p == NULL || s > p )
        p = s;

    if ( strchr( p, '.' ))
    {
        if ( !_accessmode( path, 0 ))
            rc = doxspawn( path, argv, envp );
        /* If file not found, access will have set errno to ENOENT. */
    }
    else
    {
        strcpy( buf, path );
        strcat( buf, ".com" );
        if ( !_accessmode( buf, 0 ))
            rc = doxspawn( buf, argv, envp );
        else
        {
            strcpy( strrchr( buf, '.' ), ".exe" );
            if ( !_accessmode( buf, 0 ))
                rc = doxspawn( buf, argv, envp );
            /* If file not found, access will have set errno to ENOENT. */
        }
    }

    return( rc );
}


unsigned long long_shift_left(unsigned long value, char amount)
   {
   register unsigned int upper;
   register unsigned int lower;

   lower=(unsigned int)value;
   upper=*(unsigned int *)(((char *)(&value))+2);

   while(amount--)
      {
      upper<<=1;
      upper|=(lower&0x8000)>>15;
      lower<<=1;
      }

   value=lower;
   *(unsigned int *)(((char *)(&value))+2)=upper;
   return(value);
   }


unsigned long long_shift_right(unsigned long value, char amount)
   {
   register unsigned int upper;
   register unsigned int lower;

   lower=(unsigned int)value;
   upper=*(unsigned int *)(((char *)(&value))+2);

   while(amount--)
      {
      lower>>=1;
      lower|=(upper&0x0001)<<15;
      upper>>=1;
      }

   value=lower;
   *(unsigned int *)(((char *)(&value))+2)=upper;
   return(value);
   }

int _chdir(char *path)
   {
   int drive = 0;

   if(path[1]==':')
      {
      drive = (toupper(path[0]) - 'A');
      }

   _setdrvcd(drive, (char *) path);

   return(0);
   }


/* _accessmode() - Determines the access permissions of a file. Returns -1  */
/*             on failure, 0 on success. amode of 0 indicates the existance */
/*             of the file, amode 2 indicates write permission, amode 4     */
/*             indicates read permission, and amode 6 indicates read/write  */
/*             permission.                                                  */
int _accessmode(char *filename, int amode)
   {
   FILE *fp;
   char *mode;
   char len;

   /* If we are looking for the root directory */
   len = strlen(filename);
   if((len == 3 && filename[1] == ':' && filename[2] == '\\') ||
      (len == 1 && filename[0] == '\\'))
      {
      if(amode == 0)
         {
         int to_return = 0;

#ifdef LARGEDATA
         ASM push ds
         ASM lds dx, filename
#else
         ASM mov dx, filename
#endif
         ASM mov ax, 0x4300
         ASM int 0x21
         ASM jnc done
         ASM mov word ptr to_return, -1
done:
#ifdef LARGEDATA
         ASM pop ds
#endif
         return(to_return);
         }
      else
         {
         return(-1);
         }
      }

   /* If the file doesn't exit, we fail in any mode */
   if(_odfindfirst(filename,&_fblk,0x20|0x01|0x10)) return(-1);

   /* If the file does exist, then amode 0 is satisfied */
   if(amode == 0) return(0);

   /* If testing for an access permission, determine corresponding fopen() */
   /* mode */
   switch(amode)
      {
      case 2:
         mode = "a";
         break;
      case 4:
         mode = "r";
         break;
      default:
         mode = "r+";
      }

   /* Attempt to open the file, if unable to do so return failure */
   if((fp=fopen(filename,mode)) == NULL) return(-1);

   /* If file open was successful, close it again, and return success */
   fclose(fp);
   return(0);
   }
