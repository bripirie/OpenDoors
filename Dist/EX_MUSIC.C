/* EX_MUSIC.C - Example program that demonstrates how to play "ANSI" music    */
/*              and sound effects in an OpenDoors door. See manual for        */
/*              instructions on how to compile this program.                  */



#include "opendoor.h"                    /* Required in any OpenDoors program */

#include <string.h>

void play_sound(char *string);          /* Functions for playing "ANSI" music */
char test_sound(void);

char sound_enabled=TRUE;     /* Variable indicates whether or not sound is on */



main()                                     /* Program's execution begins here */
   {                                          /* Display introductory message */
   od_printf("This is a simple door program that will play the song Happy Birthday\n\r");
   od_printf("tune on the remote system, if the user's terminal program supports ANSI\n\r");
   od_printf("music. Music is not played on the local speaker, as BBS system operators\n\r");
   od_printf("do not wish to have the BBS computer making sounds at any time of the day\n\r");
   od_printf("or night. However, the program can easily be modified to also echo sound to\n\r");
   od_printf("the local speaker.\n\r\n\r");


   test_sound();        /* Test whether user's terminal supports "ANSI" music */


   od_clr_scr();                                          /* Clear the screen */
   od_printf("\n\rHappy Birthday!\n\r");                 /* Display a message */


                         /* If "ANSI" sound is available, play Happy Birthday */
   play_sound("MBT120L4MFMNO4C8C8DCFE2C8C8DCGF2C8C8O5CO4AFED2T90B-8B-8AFGF2");
   play_sound("00m");                   /* Reset sound after finished playing */


   od_printf("\n\rPress any key to return to BBS...\n\r");  /* Display prompt */
   od_get_key(TRUE);                          /* Wait for user to press a key */
   return(0);                                                    /* Exit door */
   }



/* Function to test whether the user's terminal program supports ANSI music.
 * You can either do this every time the user uses your door, or only the first
 * time they use the door, saving the result in a data file.
 */

char test_sound(void)
   {
   char response;            /* Variable to store user's response to question */

                                       /* Display description of test to user */
   od_printf("We need to know whether or not your terminal program supports ANSI music.\n\r");
   od_printf("In order to test this, we will send a short ANSI music sequence. We will then\n\r");
   od_printf("ask whether or not you heard any sound.\n\r");
   od_printf("Press any key to begin this test... ");

   od_get_key(TRUE);                 /* Wait for user to press a key to begin */
   od_printf("\n\r\n\r");

   sound_enabled=TRUE;                            /* Temporarily enable sound */
   play_sound("MBT120L4MFMNO4C8C8DC");            /* Send sound test sequence */
   play_sound("00m");                      /* Reset sound after finished test */

   od_clr_scr();             /* Clear screen and ask whether user heard sound */
   od_printf("Did you just hear sound from your speaker? (Y/n)");
   do
      {
      response=(char)od_get_key(TRUE);
   } while (response!='y' && response!='n' && response!='Y' && response!='N');
   od_printf("\n\r\n\r");

                        /* Set ANSI music on/off according to user's response */
   sound_enabled = (char)(response=='y' || response=='Y');
   
   return(sound_enabled);
   }



/* Function to play "ANSI" music or sound effects. The play_sound() function
 * can be called with a string of 0 to 250 characters. The caracters of the
 * string define what sounds should be played on the remote speaker, as
 * follows:
 *
 *      A - G       Musical Notes
 *      # or +      Following A-G note means sharp
 *      -           Following A-G note means flat
 *      <           Move down one octave
 *      >           Move up one octave
 *      .           Period acts as dotted note (extend note duration by 3/2)
 *      MF          Music Foreground (pause until finished playing music)
 *      MB          Music Background (continue while music plays)
 *      MN          Music note duration Normal (7/8 of interval between notes)
 *      MS          Music note duration Staccato
 *      ML          Music note duration Legato
 *      Ln          Length of note (n=1-64, 1=whole note, 4=quarter note, etc)
 *      Pn          Pause length (same n values as Ln above)
 *      Tn          Tempo, n=notes/minute (n=32-255, default n=120)
 *      On          Octave number (n=0-6, default n=4)
 */

void play_sound(char *string)
   {
   char musicString[255]={27,'[','\0'};        /* Beginning of music sequence */

   if(!sound_enabled) return;                /* Abort if sound is not enabled */

   strcat(musicString,string);         /* Generate sequence to send to remote */
   musicString[strlen(musicString)+1]='\0';
   musicString[strlen(musicString)]=0x0e;

   od_disp(musicString,strlen(musicString),FALSE);   /* Transmit the sequence */
   }
