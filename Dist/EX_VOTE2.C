/* EX_VOTE2.C - The files ex_vote1.c thru ex_vote5.c demonstrate the         */
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


/* Include standard C header files required by EZVote. */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Include the OpenDoors header file. This line must be done in any program */
/* using OpenDoors.                                                         */
#include "opendoor.h"


/* Manifest constants used by EZVote */
#define NO_QUESTION              -1
#define NEW_ANSWER               -1

#define QUESTION_PAGE_SIZE        17


/* Prototypes for functions that form EZVote */
void VoteOnQuestion(void);
void ViewResults(void);
void AddQuestion(void);
void DeleteQuestion(void);
int ChooseQuestion(char *pszTitle);
void DisplayQuestionResult(void);
void WaitForEnter(void);


/* main() function - Program execution begins here. */
main()
{
   /* Variable to store user's choice from the menu */
   char chMenuChoice;
   char chYesOrNo;

   /* Initialize OpenDoors. This function call is optional, and can be used */
   /* to force OpenDoors to read the door informtion file and begin door    */
   /* operations. If a call to od_init() is not included in your program,   */
   /* OpenDoors initialization will be performed at the time of your first  */
   /* call to any OpenDoors function. */
   od_init();

   /* Loop until the user choses to exit the door. For each iteration of  */
   /* this loop, we display the main menu, get the user's choice from the */
   /* menu, and perform the appropriate action for their choice.          */

   for(;;)
   {
      /* Clear the screen */
      od_clr_scr();

      /* Display main menu */
      od_printf("                    EZVote - OpenDoors 5.00 demonstration Door\n\r");
      od_printf("-------------------------------------------------------------------------------\n\r\n\r\n\r");
      od_printf("                        [V] Vote on a question\n\r\n\r");
      od_printf("                        [R] View the results of question\n\r\n\r");
      od_printf("                        [A] Add a new question\n\r\n\r");
      
      /* If current user is the system operator, add a D function to permit */
      /* deletion of unwanted questions.                                    */
      if(strcmp(od_control.sysop_name, od_control.user_name) == 0)
      {
         od_printf("                        [D] Delete a question\n\r\n\r");
      }
      od_printf("                        [P] Page system operator for chat\n\r\n\r");
      od_printf("                        [E] Exit door and return to the BBS\n\r\n\r");
      od_printf("                        [H] End call (hangup)\n\r\n\r\n\r");
      od_printf("Press the key corresponding to the option of your choice.\n\r");

      /* Get the user's choice from the main menu. This choice may only be */
      /* V, R, A, D, P, E or H.                                            */
      chMenuChoice = od_get_answer("VRADPEH");

      /* Perform the appropriate action based on the user's choice */
      switch(chMenuChoice)
      {
         case 'V':
            /* Call EZVote's function to vote on question */
            VoteOnQuestion();
            break;
            
         case 'R':
            /* Call EZVote's function to view the results of voting */
            ViewResults();
            break;
            
         case 'A':
            /* Call EZVote's function to add a new question */
            AddQuestion();
            break;

         case 'D':
            /* Call EZVote's funciton to delete an existing question */
            DeleteQuestion();
            break;
            
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
      }
   }

   return(0);
}


/* EZVote calls the VoteOnQuestion() function when the user chooses the   */
/* vote command from the main menu. This function displays a list of      */
/* available topics, asks for the user's answer to the topic they select, */
/* and display's the results of voting on that topic.                     */
void VoteOnQuestion(void)
{
   int nQuestion;
   int nAnswer;
   char szNewAnswer[41];
   char szUserInput[3];
   
   /* Loop until the user chooses to return to the main menu, or until */
   /* there are no more questions to vote on.                          */
   for(;;)   
   {
      /* Allow the user to choose a question from the list of questions */
      /* that they have not voted on.                                   */
      nQuestion = ChooseQuestion("                              Vote On A Question\n\r");

      /* If the user did not choose a question, return to main menu. */
      if(nQuestion == NO_QUESTION)
      {
         return;
      }

      /* Loop until user makes a valid respose. */
      for(;;)
      {
         /* Display question to user. */

         /* Clear the screen. */
         od_clr_scr();

         /* Display question itself. */
         od_printf("The Question will appear here.\n\r\n\r");

         /* Loop for each answer to the question. */
         for(nAnswer = 0; nAnswer < 10; ++nAnswer)
         {
            /* Display answer number and answer. */
            od_printf("%d. This is a possible answer.\n\r", nAnswer + 1);
         }

         /* Display prompt to user. */
         od_printf("\n\rEnter answer number, [A] to add your own response, [Q] to quit: ");

         /* Get response from user. */
         od_input_str(szUserInput, 2, ' ', 255);
         /* Add a blank line. */
         od_printf("\n\r");

         /* If user entered Q, return to main menu. */
         if(stricmp(szUserInput, "Q") == 0)
         {
            return;
         }

         /* If user enetered A, and adding answers is premitted ... */
         else if (stricmp(szUserInput, "A") == 0)
         {
            /* ... Prompt for answer from user. */
            od_printf("Please enter your new answer:\n\r");
            od_printf("[------------------------------]\n\r ");

            /* Get string from user. */
            od_input_str(szNewAnswer, 30, ' ', 255);

            /* Record that user entered a new answer answer. */
            nAnswer = NEW_ANSWER;

            /* If user entered a valid answer, then exit loop. */
            if(strlen(szNewAnswer) > 0)
            {
               break;
            }
         }

         /* Otherwise, attempt to get answer number from user. */
         nAnswer = atoi(szUserInput) - 1;

         /* If user input is not a valid answer. */
         if(nAnswer < 0 || nAnswer >= 10)
         {
            /* Display message. */
            od_printf("That is not a valid response.\n\r");
            WaitForEnter();
         }
         else
         {
            /* Otherwise, exit loop. */
            break;
         }
      }

      /* Display the result of voting on this question to the user. */
      DisplayQuestionResult();
   }
}


/* The ViewResults function is called when the user chooses the "view    */
/* results" command from the main menu. This function alows the user to  */
/* choose a question from the list of questions, and then displays the   */
/* results of voting on that question.                                   */
void ViewResults(void)
{
   int nChoice;

   /* Loop until user chooses to return to main menu. */
   for(;;)
   {   
      /* Allow the user to choose a question from the list of questions that */
      /* they have already voted on.                                         */
      nChoice = ChooseQuestion("                                 View Results\n\r");

      /* If the user did not choose a question, return to main menu. */
      if(nChoice == NO_QUESTION)
      {
         return;
      }

      /* Display the results for the selected question. */
      DisplayQuestionResult();
   }
}
 

/* The AddQuestion() function is called when the user chooses the "add    */
/* question" option from the main menu. This function allows the user     */
/* to enter a new question, possible responses, and save the question for */
/* other users to vote on.                                                */
void AddQuestion(void)
{
   char szUserInput[71];
   int nAnswerNumber;

   /* Clear the screen. */
   od_clr_scr();
   
   /* Display screen header. */
   od_printf("                                Add A Question\n\r");
   od_printf("-------------------------------------------------------------------------------\n\r\n\r");
   
   /* Obtain quesiton text from the user. */
   od_printf("Enter Your Question (blank line cancels)\n\r");
   od_printf("[----------------------------------------------------------------------]\n\r ");
   od_input_str(szUserInput, 70, ' ', 255);
   
   /* If question was empty, then return to main menu. */
   if(strlen(szUserInput) == 0)
   {
      return;
   }
   
   /* Display prompt for answers. */
   od_printf("\n\rEnter Possible Answers (blank line when done)\n\r");
   od_printf("   [------------------------------]\n\r");
   
   /* Loop, getting answers from user. */
   for(nAnswerNumber = 1; nAnswerNumber <= 10; nAnswerNumber++)
   {
      /* Display prompt with answer number. */
      od_printf("%2d: ", nAnswerNumber);
      
      /* Get string from user. */
      od_input_str(szUserInput, 30, ' ', 255);
         
      /* If string was empty, then exit loop. */
      if(strlen(szUserInput) == 0)
      {
         break;
      }
   }
   
   /* If no answers were supplied, then cancel, returning to main menu. */
   if(nAnswerNumber == 1)
   {
      return;
   }

   /* Ask whether users should be able to add their own answers. */
   od_printf("\n\rShould voters be able to add their own options? (Y/N) ");
   
   /* Get answer from user. */
   if(od_get_answer("YN") == 'Y')
   {
      /* If user pressed the 'Y' key. */
      od_printf("Yes\n\r\n\r");
   }
   else
   {
      /* If user pressed the 'N' key. */
      od_printf("No\n\r\n\r");
   }
   
   /* Confirm save of new question. */
   od_printf("Do you wish to save this new question? (Y/N) ");

   /* Get response from user. */
   od_get_answer("YN");
}


/* The DeleteQuestion() function is called when the sysop chooses the   */
/* "delete question" option from the main menu. This function displays  */
/* a list of all questions, allowing the sysop to choose a question for */
/* deletion.                                                            */
void DeleteQuestion(void)
{
   int nQuestion;

   /* Check that user is system operator. */
   if(strcmp(od_control.user_name, od_control.sysop_name) != 0)
   {
      return;
   }

   /* Allow the user to choose a question from the list of all questions. */
   nQuestion = ChooseQuestion("                              Delete A Question\n\r");

   /* If the user did not choose a question, return to main menu. */   
   if(nQuestion == NO_QUESTION)
   {
      return;
   }

   /* Confirm deletion of this question. */
   od_printf("\n\rAre you sure you want to delete the question:\n\r");
   od_printf("    Question will appear here.\n\r");
   od_printf("[Y]es or [N]o?\n\r");

   /* Get response from user. */
   od_get_answer("YN");
}


/* The ChooseQuestion() function will provide a list of questions and will */
/* allow the user to choose a particular question, cancel back to the main */
/* menu, and page up and down in the list of questions.                    */
int ChooseQuestion(char *pszTitle)
{
   int nCurrent;
   int nPagedToQuestion = 0;
   char chCurrent;
   
   /* Loop, displaying current page of questions, until the user makes a */
   /* choice.                                                            */
   for(;;)
   {
      /* Clear the screen. */
      od_clr_scr();

      /* Display header. */
      od_printf(pszTitle);
      od_printf("-------------------------------------------------------------------------------\n\r");
   
      /* Display list of questions on this page. */
      for(nCurrent = 0;
         nCurrent < QUESTION_PAGE_SIZE;
         ++nCurrent)
      {
         /* Determine character to display for current line. */
         if(nCurrent < 9)
         {
            chCurrent = (char)('1' + nCurrent);
         }
         else
         {
            chCurrent = (char)('A' + (nCurrent - 9));
         }
      
         /* Display this question's title. */
         od_printf("%c. A Question's title will appear here\n\r", chCurrent);
      }

      /* Display prompt for input. */
      od_printf("\n\r[Page %d]  Choose a question or",
         (nPagedToQuestion / QUESTION_PAGE_SIZE) + 1);
      if(nPagedToQuestion < 80 - QUESTION_PAGE_SIZE)
      {
         od_printf(" [N]ext page,");
      }
      if(nPagedToQuestion > 0)
      {
         od_printf(" [P]revious page,");
      }
      od_printf(" [Q]uit.\n\r");
      
      /* Loop until the user makes a valid choice. */
      for(;;)
      {      
         /* Get input from user */
         chCurrent = (char)od_get_key(TRUE);
         chCurrent = (char)toupper(chCurrent);
      
         /* Respond to user's input. */
      
         /* If user pressed Q key. */
         if(chCurrent == 'Q')
         {
            /* Return without a choosing a question. */
            return(NO_QUESTION);
         }
      
         /* If user pressed P key. */
         else if(chCurrent == 'P')
         {
            /* If we are not at the first page. */
            if(nPagedToQuestion > 0)
            {
               /* Move paged to location up one page. */
               nPagedToQuestion -= QUESTION_PAGE_SIZE;
               
               /* Exit user input loop to display next page. */
               break;
            }
         }
      
         /* If user pressed N key. */
         else if(chCurrent == 'N')
         {
            /* If there is more questions after this page. */
            if(nPagedToQuestion < 80)
            {
               /* Move paged.to location down one page. */
               nPagedToQuestion += QUESTION_PAGE_SIZE;

               /* Exit user input loop to display next page. */
               break;
            }
         }
      
         /* Otherwise, check whether the user chose a valid question. */
         else if ((chCurrent >= '1' && chCurrent <= '9')
            || (chCurrent >= 'A' && chCurrent <= 'H'))
         {
            /* Return actual question number in file. */
            return(0);
         }
      }
   }
}


/* The DisplayQuestionResult() function is called to display the results */
/* of voting on a paricular question. This function is called when the   */
/* user selects a question using the "view results" option, and is also  */
/* called after the user has voted on a question, to display the results */
/* of voting on that question.                                           */
void DisplayQuestionResult(void)
{
   int nAnswer;

   /* Clear the screen. */
   od_clr_scr();

   /* Display question itself. */
   od_printf("The Question will appear here.\n\r");

   /* Display author's name. */
   od_printf("Question created by ?????? on ??????\n\r\n\r");
   
   /* Display heading for responses. */
   od_printf("Response                        Votes  Percent  Graph\n\r");
   od_printf("-------------------------------------------------------------------------------\n\r");

   /* Loop for each answer to the question. */   
   for(nAnswer = 0; nAnswer < 10; ++nAnswer)
   {
      /* Display answer, total votes and percentage of votes. */
      od_printf("??????????????????????????????  ?????  ???      ===============================\n\r");
   }
   
   /* Display footer. */
   od_printf("-------------------------------------------------------------------------------\n\r");
   od_printf("                         TOTAL: ???\n\r\n\r");
   
   /* Wait for user to press enter. */
   WaitForEnter();
}


/* The WaitForEnter() function is used by EZVote to create its custom */
/* "Press [ENTER] to continue." prompt.                               */
void WaitForEnter(void)
{
   /* Display prompt. */
   od_printf("Press [ENTER] to continue.\n\r");
   
   /* Wait for a Carriage Return or Line Feed character from the user. */
   od_get_answer("\n\r");
}
