/* EX_HELLO.C - Example of a trivial OpenDoors door program. Demonstrates     */
/*              just how simple a fully functional door program can be. See   */
/*              manual for instructions on how to compile this program.       */


#include "opendoor.h"                    /* Required in any OpenDoors program */


main()
   {                                                     /* Display a message */
   od_printf("Hello world! This is a very simple door program.\n\r");
   od_printf("Press any key to return to the BBS!\n\r");

   od_get_key(TRUE);                          /* Wait for user to press a key */

   return(0);                                  /* Exit door, returning to BBS */
   }
