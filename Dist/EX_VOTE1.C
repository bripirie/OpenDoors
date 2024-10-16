/* EX_VOTE1.C - The files ex_vote1.c thru ex_vote5.c demonstrate the         */
/*              possible steps in the development of a door program using    */
/*              OpenDoors. The EZVote door program allows users to create    */
/*              questions or surveys for other users to respond to. Users    */
/*              are also able to view the results of voting on each topic    */
/*              after having voted on the topic themselves.                  */
/*                                                                           */
/*              To recompile this program, follow the instructions in the    */
/*              OpenDoors manual. Be sure to set your compiler to use the    */
/*              large memory model, and add the ODOORL.LIB file to your      */
/*              project/makefile.                                            */
/*                                                                           */
/*              Each of the ex_vote?.c files shows a further incremental     */
/*              step in the construction of the EZVote door program, as      */
/*              follows:                                                     */
/*                                                                           */
/*              EX_VOTE1.C - Demonstrates the basic elements of any door     */
/*                           program. Contains the code to handle display    */
/*                           of main menu, responding to the user's choice   */
/*                           from the main menu, and common commands such    */
/*                           as returning to the BBS and paging the system   */
/*                           operator. Demonstrates basics of displaying     */
/*                           text and retrieving input from user.            */
/*                                                                           */
/*              EX_VOTE2.C - Adds the user interface code to handle the main */
/*                           menu commands specific to EZVote, such as       */
/*                           answering questions, viewing the results of     */
/*                           questions, and adding new questions.            */
/*                           Demonstrates the use of OpenDoors functions     */
/*                           such as od_input_str() for allowing the user to */
/*                           input a sring of characters, and od_get_key()   */
/*                           for inputting any character from the user. Also */
/*                           introduces the od_control structure for         */
/*                           obtaining information about the user and the    */
/*                           system that the door program is running on.     */
/*                                                                           */
/*              EX_VOTE3.C - Adds the underlying file access functionality   */
/*                           that is specific to EZVote. EZVote uses a       */
/*                           relatively complex file structure in order to   */
/*                           track which questions each user has voted on,   */
/*                           and in order to allow a large number (200)      */
/*                           question records to be stored in the file.      */
/*                                                                           */
/*              EX_VOTE4.C - Adds color to display and demonstrates the use  */
/*                           of ANSI/AVATAR/RIP specific features.           */
/*                                                                           */
/*              EX_VOTE5.C - Adds support for the OpenDoors configuration    */
/*                           file system, which provides automatic support   */
/*                           for a wide variety of configurable options.     */
/*                           EZVote adds its own configuration options to    */
/*                           control program characteristics such as whether */
/*                           or not the user is premitted to create their    */
/*                           own questions. Also adds support for the        */
/*                           OpenDoors log file system which records major   */
/*                           actions taken by the user. In addition, this    */
/*                           step enables the OpenDoors multiple-personality */
/*                           system and adds other finishing touches.        */


/* Include the OpenDoors header file. This line must be done in any program */
/* using OpenDoors.                                                         */
#include "opendoor.h"


/* main() function - Program execution begins here. */
main()
{
   /* Variable to store user's choice from the menu */
   char chMenuChoice;
   char chYesOrNo;

   /* Loop until the user choses to exit the door. For each iteration of  */
   /* this loop, we display the main menu, get the user's choice from the */
   /* menu, and perform the appropriate action for their choice.          */

   for(;;)
   {
      /* Clear the screen */
      od_clr_scr();

      /* Display main menu */
      od_printf("                     EZVote - OpenDoors 5.00 demonstration Door\n\r");
      od_printf("-------------------------------------------------------------------------------\n\r\n\r\n\r");
      od_printf("                        [V] Vote on a question\n\r\n\r");
      od_printf("                        [R] View the results of question\n\r\n\r");
      od_printf("                        [A] Add a new question\n\r\n\r");
      od_printf("                        [P] Page system operator for chat\n\r\n\r");
      od_printf("                        [E] Exit door and return to the BBS\n\r\n\r");
      od_printf("                        [H] End call (hangup)\n\r\n\r\n\r");
      od_printf("Press the key corresponding to the option of your choice.\n\r");

      /* Get the user's choice from the main menu. This choice may only be */
      /* V, R, A, P, E or H.                                               */
      chMenuChoice = od_get_answer("VRAPEH");

      /* Perform the appropriate action based on the user's choice */
      switch(chMenuChoice)
      {
         case 'P':
            /* If the user pressed P, allow them page the system operator. */
            od_page();
            break;

         case 'E':
            /* If the user pressed E, exit door and return to BBS. */
            od_exit(0, FALSE);
            break;

         case 'H':
            /* If the user pressed H, ask whether they wish to hangup. */
            od_printf("\n\rAre you sure you wish to hangup? (Y/N) ");

            /* Get user's response */
            chYesOrNo = od_get_answer("YN");

            /* If user answered yes, exit door and hangup */
            if(chYesOrNo == 'Y')
            {
               od_exit(0, TRUE);
            }
            break;

         default:
            /* If the user made any other choice from the menu, then it was */
            /* a choice which we don't yet support. */

            /* Display a notification message. */
            od_printf("This feature isn't finished in this version of EZVote.\n\r");
            od_printf("(See one of the later versions included in your OpenDoors package.)\n\r\n\r");
            od_printf("Press any key to continue.\n\r");

            /* Wait for the user to press a key to continue. */
            od_get_key(TRUE);
      }
   }
   
   return(0);
}
