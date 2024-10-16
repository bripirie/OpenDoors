/* EX_VOTE4.C - The files ex_vote1.c thru ex_vote5.c demonstrate the         */
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

/* Include the OpenDoors header file. This line must be done in any program */
/* using OpenDoors.                                                         */
#include "opendoor.h"


/* Manifest constants used by EZVote */
#define NO_QUESTION              -1
#define NEW_ANSWER               -1

#define QUESTIONS_VOTED_ON        0x0001
#define QUESTIONS_NOT_VOTED_ON    0x0002

#define MAX_QUESTIONS             200
#define MAX_USERS                 30000
#define MAX_ANSWERS               15
#define QUESTION_STR_SIZE         71
#define ANSWER_STR_SIZE           31

#define USER_FILENAME             "EZVOTE.USR"
#define QUESTION_FILENAME         "EZVOTE.QST"

#define FILE_ACCESS_MAX_WAIT      20

#define QUESTION_PAGE_SIZE        17


/* Structure of records stored in the EZVOTE.USR file */
typedef struct
{
   char szUserName[36];
   char bVotedOnQuestion[MAX_QUESTIONS];
} tUserRecord;
              
tUserRecord CurrentUserRecord;
int nCurrentUserNumber;


/* Structure of records stored in the EZVOTE.QST file */
typedef struct
{
   char szQuestion[QUESTION_STR_SIZE];
   char aszAnswer[MAX_ANSWERS][ANSWER_STR_SIZE];
   int nTotalAnswers;
   unsigned int auVotesForAnswer[MAX_ANSWERS];
   unsigned int uTotalVotes;
   char bCanAddAnswers;
   char bDeleted;
   char szCreatorName[36];
   time_t lCreationTime;
} tQuestionRecord;


/* Prototypes for functions that form EZVote */
void VoteOnQuestion(void);
void ViewResults(void);
int GetQuestion(int nQuestion, tQuestionRecord *pQuestionRecord);
void AddQuestion(void);
void DeleteQuestion(void);
int ChooseQuestion(int nFromWhichQuestions, char *pszTitle, int *nLocation);
void DisplayQuestionResult(tQuestionRecord *pQuestionRecord);
int ReadOrAddCurrentUser(void);
void WriteCurrentUser(void);
FILE *ExculsiveFileOpen(char *pszFileName, char *pszMode);
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

   /* Call the EZVote function ReadOrAddCurrentUser() to read the current */
   /* user's record from the EZVote user file, or to add the user to the  */
   /* file if this is the first time that they have used EZVote.          */
   if(!ReadOrAddCurrentUser())
   {
      /* If unable to obtain a user record for the current user, then exit */
      /* the door after displaying an error message.                       */
      od_printf("Unable to access user file. File may be locked or full.\n\r");
      WaitForEnter();
      od_exit(1, FALSE);
   }   

   /* Loop until the user choses to exit the door. For each iteration of  */
   /* this loop, we display the main menu, get the user's choice from the */
   /* menu, and perform the appropriate action for their choice.          */

   for(;;)
   {
      /* Clear the screen */
      od_clr_scr();

      /* Display main menu */
      od_printf("`bright red`                    EZVote - OpenDoors 5.00 demonstration Door\n\r");
      od_printf("`dark red`");
      if(od_control.user_ansi || od_control.user_avatar)
      {
         od_repeat((unsigned char)196, 79);
      }
      else
      {
         od_repeat('-', 79);
      }
      od_printf("\n\r\n\r\n\r`dark green`");
      od_printf("                        [`bright green`V`dark green`] Vote on a question\n\r\n\r");
      od_printf("                        [`bright green`R`dark green`] View the results of question\n\r\n\r");
      od_printf("                        [`bright green`A`dark green`] Add a new question\n\r\n\r");
      
      /* If current user is the system operator, add a D function to permit */
      /* deletion of unwanted questions.                                    */
      if(strcmp(od_control.sysop_name, od_control.user_name) == 0)
      {
         od_printf("                        [`bright green`D`dark green`] Delete a question\n\r\n\r");
      }
      od_printf("                        [`bright green`P`dark green`] Page system operator for chat\n\r\n\r");
      od_printf("                        [`bright green`E`dark green`] Exit door and return to the BBS\n\r\n\r");
      od_printf("                        [`bright green`H`dark green`] End call (hangup)\n\r\n\r\n\r");
      od_printf("`bright white`Press the key corresponding to the option of your choice.\n\r`dark green`");

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
   tQuestionRecord QuestionRecord;
   char szNewAnswer[ANSWER_STR_SIZE];
   char szUserInput[3];
   FILE *fpFile;
   int nPageLocation = 0;

   /* Loop until the user chooses to return to the main menu, or until */
   /* there are no more questions to vote on.                          */
   for(;;)   
   {
      /* Allow the user to choose a question from the list of questions */
      /* that they have not voted on.                                   */
      nQuestion = ChooseQuestion(QUESTIONS_NOT_VOTED_ON,
         "                              Vote On A Question\n\r",
         &nPageLocation);
   

      /* If the user did not choose a question, return to main menu. */   
      if(nQuestion == NO_QUESTION)
      {
         return;
      }

      /* Read the question chosen by the user. */
      if(!GetQuestion(nQuestion, &QuestionRecord))
      {
         /* If unable to access file, return to main menu. */
         return;
      }
   
      /* Don't allow addition of new answers if maximum number of answers */
      /* have already been added.                                         */
      if(QuestionRecord.nTotalAnswers >= MAX_ANSWERS)
      {
         QuestionRecord.bCanAddAnswers = FALSE;
      }
   
      /* Loop until user makes a valid respose. */
      for(;;)
      {
         /* Display question to user. */

         /* Clear the screen. */
         od_clr_scr();

         /* Display question itself. */
         od_printf("`bright red`%s\n\r\n\r", QuestionRecord.szQuestion);

         /* Loop for each answer to the question. */   
         for(nAnswer = 0; nAnswer < QuestionRecord.nTotalAnswers; ++nAnswer)
         {
            /* Display answer number and answer. */
            od_printf("`bright green`%d. `dark green`%s\n\r",
               nAnswer + 1,
               QuestionRecord.aszAnswer[nAnswer]);
         }

         /* Display prompt to user. */
         od_printf("\n\r`bright white`Enter answer number, ");
         if(QuestionRecord.bCanAddAnswers)
         {
            od_printf("[A] to add your own response, ");
         }
         od_printf("[Q] to quit: `dark green`");
   
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
         else if (stricmp(szUserInput, "A") == 0
            && QuestionRecord.bCanAddAnswers)
         {
            /* ... Prompt for answer from user. */
            od_printf("`bright green`Please enter your new answer:\n\r");
            od_printf("`dark green`[------------------------------]\n\r ");
         
            /* Get string from user. */
            od_input_str(szNewAnswer, ANSWER_STR_SIZE - 1, ' ', 255);
         
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
         if(nAnswer < 0 || nAnswer >= QuestionRecord.nTotalAnswers)
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

      /* Add user's vote to question. */
   
      /* Open question file for exclusive access by this node. */
      fpFile = ExculsiveFileOpen(QUESTION_FILENAME, "r+b");
      if(fpFile == NULL)
      {
         /* If unable to access file, display error and return. */
         od_printf("Unable to access the question file.\n\r");
         WaitForEnter();
         return;
      }
   
      /* Read the answer record from disk, because it may have been changed. */
      /* by another node. */
      fseek(fpFile, (long)nQuestion * sizeof(tQuestionRecord), SEEK_SET);
      if(fread(&QuestionRecord, sizeof(tQuestionRecord), 1, fpFile) != 1)
      {
         /* If unable to access file, display error and return. */
         fclose(fpFile);
         od_printf("Umable to read from question file.\n\r");
         WaitForEnter();
         return;
      }
   
      /* If user entered their own answer, try to add it to the question. */
      if(nAnswer == NEW_ANSWER)
      {
         /* Check that there is still room for another answer. */
         if(QuestionRecord.nTotalAnswers >= MAX_ANSWERS)
         {
            fclose(fpFile);
            od_printf("Sorry, this question already has the maximum number of answers.\n\r");
            WaitForEnter();
            return;
         }
      
         /* Set answer number to number of new answer. */
         nAnswer = QuestionRecord.nTotalAnswers;
      
         /* Add 1 to total number of answers. */
         ++QuestionRecord.nTotalAnswers;
      
         /* Initialize new answer string and count. */
         strcpy(QuestionRecord.aszAnswer[nAnswer], szNewAnswer);
         QuestionRecord.auVotesForAnswer[nAnswer] = 0;
      }
   
      /* Add user's vote to question. */
      ++QuestionRecord.auVotesForAnswer[nAnswer];
      ++QuestionRecord.uTotalVotes;
   
      /* Write the question record back to the file. */
      fseek(fpFile, (long)nQuestion * sizeof(tQuestionRecord), SEEK_SET);
      if(fwrite(&QuestionRecord, sizeof(tQuestionRecord), 1, fpFile) != 1)
      {
         /* If unable to access file, display error and return. */
         fclose(fpFile);
         od_printf("Umable to write question to file.\n\r");
         WaitForEnter();
         return;
      }
   
      /* Close the question file to allow access by other nodes. */
      fclose(fpFile);
   
      /* Record that user has voted on this question. */
      CurrentUserRecord.bVotedOnQuestion[nQuestion] = TRUE;
   
      /* Open user file for exclusive access by this node. */
      fpFile = ExculsiveFileOpen(USER_FILENAME, "r+b");
      if(fpFile == NULL)
      {
         /* If unable to access file, display error and return. */
         od_printf("Unable to access the user file.\n\r");
         WaitForEnter();
         return;
      }

      /* Update the user's record in the user file. */
      fseek(fpFile, nCurrentUserNumber * sizeof(tUserRecord), SEEK_SET);
      if(fwrite(&CurrentUserRecord, sizeof(tUserRecord), 1, fpFile) != 1)
      {
         /* If unable to access file, display error and return. */
         fclose(fpFile);
         od_printf("Unable to write to user file.\n\r");
         WaitForEnter();
         return;
      }
   
      /* Close the user file to allow access by other nodes. */
      fclose(fpFile);
   
      /* Display the result of voting on this question to the user. */
      DisplayQuestionResult(&QuestionRecord);
   }
}


/* The ViewResults function is called when the user chooses the "view    */
/* results" command from the main menu. This function alows the user to  */
/* choose a question from the list of questions, and then displays the   */
/* results of voting on that question.                                   */
void ViewResults(void)
{
   int nChoice;
   tQuestionRecord QuestionRecord;
   int nPageLocation = 0;

   /* Loop until user chooses to return to main menu. */
   for(;;)
   {   
      /* Allow the user to choose a question from the list of questions that */
      /* they have already voted on.                                         */
      nChoice = ChooseQuestion(QUESTIONS_VOTED_ON,
         "                                 View Results\n\r", &nPageLocation);

      /* If the user did not choose a question, return to main menu. */   
      if(nChoice == NO_QUESTION)
      {
         return;
      }
   
      /* Read the specified question number from the question file. */
      if(!GetQuestion(nChoice, &QuestionRecord))
      {
         return;
      }
   
      /* Display the results for the selected question. */
      DisplayQuestionResult(&QuestionRecord);
   }
}


/* The GetQuestion function read the record for the specified question */
/* number from the question file.                                      */
int GetQuestion(int nQuestion, tQuestionRecord *pQuestionRecord)
{
   FILE *fpQuestionFile;

   /* Open the question file for exculsive access by this node. */
   fpQuestionFile = ExculsiveFileOpen(QUESTION_FILENAME, "r+b");
   if(fpQuestionFile == NULL)
   {
      /* If unable to access file, display error and return. */
      od_printf("Unable to access the question file.\n\r");
      WaitForEnter();
      return(FALSE);
   }
   
   /* Move to location of question in file. */
   fseek(fpQuestionFile, (long)nQuestion * sizeof(tQuestionRecord), SEEK_SET);
   
   /* Read the question from the file. */
   if(fread(pQuestionRecord, sizeof(tQuestionRecord), 1, fpQuestionFile) != 1)
   {
      /* If unable to access file, display error and return. */
      fclose(fpQuestionFile);
      od_printf("Umable to read from question file.\n\r");
      WaitForEnter();
      return(FALSE);;
   }
   
   /* Close the question file to allow access by other nodes. */
   fclose(fpQuestionFile);

   /* Return with success. */
   return(TRUE);
}
 

/* The AddQuestion() function is called when the user chooses the "add    */
/* question" option from the main menu. This function allows the user     */
/* to enter a new question, possible responses, and save the question for */
/* other users to vote on.                                                */
void AddQuestion(void)
{
   tQuestionRecord QuestionRecord;
   FILE *fpQuestionFile;

   /* Clear the screen. */
   od_clr_scr();
   
   /* Display screen header. */
   od_printf("`bright red`                                Add A Question\n\r");
   od_printf("`dark red`");
   if(od_control.user_ansi || od_control.user_avatar)
   {
      od_repeat((unsigned char)196, 79);
   }
   else
   {
      od_repeat('-', 79);
   }
   od_printf("\n\r\n\r");
   
   /* Obtain quesiton text from the user. */
   od_printf("`bright green`Enter Your Question (blank line cancels)\n\r");
   od_printf("`dark green`[----------------------------------------------------------------------]\n\r ");
   od_input_str(QuestionRecord.szQuestion, QUESTION_STR_SIZE - 1, ' ', 255);
   
   /* If question was empty, then return to main menu. */
   if(strlen(QuestionRecord.szQuestion) == 0)
   {
      return;
   }
   
   /* Display prompt for answers. */
   od_printf("\n\r`bright green`Enter Possible Answers (blank line when done)\n\r");
   od_printf("`dark green`   [------------------------------]\n\r");
   
   /* Loop, getting answers from user. */
   for(QuestionRecord.nTotalAnswers = 0;
       QuestionRecord.nTotalAnswers < MAX_ANSWERS;
       QuestionRecord.nTotalAnswers++)
   {
      /* Display prompt with answer number. */
      od_printf("`bright green`%2d: `dark green`", QuestionRecord.nTotalAnswers + 1);
      
      /* Get string from user. */
      od_input_str(QuestionRecord.aszAnswer[QuestionRecord.nTotalAnswers],
         ANSWER_STR_SIZE - 1, ' ', 255);
         
      /* If string was empty, then exit loop. */
      if(strlen(QuestionRecord.aszAnswer[QuestionRecord.nTotalAnswers]) == 0)
      {
         break;
      }
      
      /* Reset count of votes for this answer to zero. */
      QuestionRecord.auVotesForAnswer[QuestionRecord.nTotalAnswers] = 0;
   }
   
   /* If no answers were supplied, then cancel, returning to main menu. */
   if(QuestionRecord.nTotalAnswers == 0)
   {
      return;
   }

   /* Ask whether users should be able to add their own answers. */
   od_printf("\n\r`bright green`Should voters be able to add their own options? (Y/N) `dark green`");
   
   /* Get answer from user. */
   if(od_get_answer("YN") == 'Y')
   {
      /* If user pressed the 'Y' key. */
      od_printf("Yes\n\r\n\r");
      
      /* Record user's response. */
      QuestionRecord.bCanAddAnswers = TRUE;
   }
   else
   {
      /* If user pressed the 'N' key. */
      od_printf("No\n\r\n\r");

      /* Record user's response. */
      QuestionRecord.bCanAddAnswers = FALSE;
   }
   
   /* Confirm save of new question. */
   od_printf("`bright green`Do you wish to save this new question? (Y/N) `dark green`");
   
   /* If user does not want to save the question, return to main menu now. */
   if(od_get_answer("YN") == 'N')
   {
      return;
   }

   /* Set total number of votes for this question to 0. */   
   QuestionRecord.uTotalVotes = 0;
   
   /* Set creator name and creation time for this question. */
   strcpy(QuestionRecord.szCreatorName, od_control.user_name);
   QuestionRecord.lCreationTime = time(NULL);
   QuestionRecord.bDeleted = FALSE;
   
   /* Open question file for exclusive access by this node. */
   fpQuestionFile = ExculsiveFileOpen(QUESTION_FILENAME, "a+b");
   if(fpQuestionFile == NULL)
   {
      od_printf("Unable to access the question file.\n\r");
      WaitForEnter();
      return;
   }
   
   /* Determine number of records in question file. */
   fseek(fpQuestionFile, 0, SEEK_END);
   
   /* If question file is full, display message and return to main menu */
   /* after closing file.                                               */
   if(ftell(fpQuestionFile) / sizeof(tQuestionRecord) >= MAX_QUESTIONS)
   {
      fclose(fpQuestionFile);
      od_printf("Cannot add another question, EZVote is limisted to %d questions.\n\r", MAX_QUESTIONS);
      WaitForEnter();
      return;
   }
   
   /* Add new question to file. */
   if(fwrite(&QuestionRecord, sizeof(QuestionRecord), 1, fpQuestionFile) != 1)
   {
      fclose(fpQuestionFile);
      od_printf("Umable to write to question file.\n\r");
      WaitForEnter();
      return;
   }
   
   /* Close question file, allowing other nodes to access file. */
   fclose(fpQuestionFile);
}


/* The DeleteQuestion() function is called when the sysop chooses the   */
/* "delete question" option from the main menu. This function displays  */
/* a list of all questions, allowing the sysop to choose a question for */
/* deletion.                                                            */
void DeleteQuestion(void)
{
   int nQuestion;
   tQuestionRecord QuestionRecord;
   FILE *fpFile;
   int nPageLocation = 0;

   /* Check that user is system operator. */
   if(strcmp(od_control.user_name, od_control.sysop_name) != 0)
   {
      return;
   }

   /* Allow the user to choose a question from the list of all questions. */
   nQuestion = ChooseQuestion(QUESTIONS_NOT_VOTED_ON | QUESTIONS_VOTED_ON,
      "                              Delete A Question\n\r", &nPageLocation);

   /* If the user did not choose a question, return to main menu. */   
   if(nQuestion == NO_QUESTION)
   {
      return;
   }

   /* Read the question chosen by the user. */
   if(!GetQuestion(nQuestion, &QuestionRecord))
   {
      /* If unable to access file, return to main menu. */
      return;
   }

   /* Confirm deletion of this question. */
   od_printf("\n\r`bright green`Are you sure you want to delete the question:\n\r   `dark green`%s\n\r",
      QuestionRecord.szQuestion);
   od_printf("`bright green`[Y]es or [N]o?\n\r`dark green`");

   /* If user canceled deletion, return now. */   
   if(od_get_answer("YN") == 'N')
   {
      return;
   }

   /* Mark the question as being deleted. */
   QuestionRecord.bDeleted = TRUE;
   
   /* Open question file for exclusive access by this node. */
   fpFile = ExculsiveFileOpen(QUESTION_FILENAME, "r+b");
   if(fpFile == NULL)
   {
      /* If unable to access file, display error and return. */
      od_printf("Unable to access the question file.\n\r");
      WaitForEnter();
      return;
   }

   /* Write the question record back to the file. */
   fseek(fpFile, (long)nQuestion * sizeof(tQuestionRecord), SEEK_SET);
   if(fwrite(&QuestionRecord, sizeof(tQuestionRecord), 1, fpFile) != 1)
   {
      /* If unable to access file, display error and return. */
      fclose(fpFile);
      od_printf("Umable to write question to file.\n\r");
      WaitForEnter();
      return;
   }
   
   /* Close the question file to allow access by other nodes. */
   fclose(fpFile);
}


/* The ChooseQuestion() function provides a list of questions and allows   */
/* the user to choose a particular question, cancel back to the main menu, */ 
/* and page up and down in the list of questions. Depending upon the value */
/* of the nFromWhichQuestions parameter, this function will present a list */
/* of questions that the user has voted on, a list of questions that the   */
/* user has not voted on, or a list of all questions.                      */
int ChooseQuestion(int nFromWhichQuestions, char *pszTitle, int *nLocation)
{
   int nCurrent;
   int nFileQuestion = 0;
   int nPagedToQuestion = *nLocation;
   int nDisplayedQuestion = 0;
   char bVotedOnQuestion;
   char chCurrent;
   tQuestionRecord QuestionRecord;
   FILE *fpQuestionFile;
   static char szQuestionName[MAX_QUESTIONS][QUESTION_STR_SIZE];
   static int nQuestionNumber[MAX_QUESTIONS];
   
   /* Attempt to open question file. */
   fpQuestionFile = ExculsiveFileOpen(QUESTION_FILENAME, "r+b");

   /* If unable to open question file, assume that no questions have been */
   /* created.                                                            */
   if(fpQuestionFile == NULL)
   {
      /* Display "no questions yet" message. */
      od_printf("\n\rNo questions have been created so far.\n\r");
      
      /* Wait for user to press enter. */
      WaitForEnter();
      
      /* Indicate that no question has been chosen. */
      return(NO_QUESTION);
   }
   
   /* Loop for every question record in the file. */
   while(fread(&QuestionRecord, sizeof(QuestionRecord), 1, fpQuestionFile) == 1)
   {
      /* Determine whether or not the user has voted on this question. */
      bVotedOnQuestion = CurrentUserRecord.bVotedOnQuestion[nFileQuestion];
      
      /* If this is the kind of question that the user is choosing from */
      /* right now.                                                     */
      if((bVotedOnQuestion && (nFromWhichQuestions & QUESTIONS_VOTED_ON)) ||
         (!bVotedOnQuestion && (nFromWhichQuestions & QUESTIONS_NOT_VOTED_ON)))
      {
         /* If question is not deleted. */
         if(!QuestionRecord.bDeleted)
         {
            /* Add this question to list to be displayed. */
            strcpy(szQuestionName[nDisplayedQuestion],
               QuestionRecord.szQuestion);
            nQuestionNumber[nDisplayedQuestion] = nFileQuestion;
         
            /* Add one to number of questions to be displayed in list. */
            nDisplayedQuestion++;
         }
      }
      
      /* Move to next question in file. */
      ++nFileQuestion;
   }   
   
   /* Close question file to allow other nodes to access the file. */
   fclose(fpQuestionFile);

   /* If there are no questions for the user to choose, display an */
   /* appropriate message and return. */
   if(nDisplayedQuestion == 0)
   {
      /* If we were to list all questions. */
      if((nFromWhichQuestions & QUESTIONS_VOTED_ON)
         && (nFromWhichQuestions & QUESTIONS_NOT_VOTED_ON))
      {
         od_printf("\n\rThere are no questions.\n\r");
      }
      /* If we were to list questions that the user has voted on. */
      else if(nFromWhichQuestions & QUESTIONS_VOTED_ON)
      {
         od_printf("\n\rThere are no questions that you have voted on.\n\r");
      }
      /* Otherwise, we were to list questions that use has not voted on. */
      else
      {
         od_printf("\n\rYou have voted on all the questions.\n\r");
      }
      
      /* Wait for user to press enter key. */
      WaitForEnter();
      
      /* Return, indicating that no question was chosen. */
      return(NO_QUESTION);
   }

   /* Ensure that initial paged to location is within range. */
   while(nPagedToQuestion >= nDisplayedQuestion)
   {
      nPagedToQuestion -= QUESTION_PAGE_SIZE;
   }

   /* Loop, displaying current page of questions, until the user makes a */
   /* choice.                                                            */
   for(;;)
   {
      /* Clear the screen. */
      od_clr_scr();

      /* Display header. */
      od_printf("`bright red`");
      od_printf(pszTitle);
      od_printf("`dark red`");
      if(od_control.user_ansi || od_control.user_avatar)
      {
         od_repeat((unsigned char)196, 79);
      }
      else
      {
         od_repeat('-', 79);
      }
      od_printf("\n\r");
   
      /* Display list of questions on this page. */
      for(nCurrent = 0;
         nCurrent < QUESTION_PAGE_SIZE
         && nCurrent < (nDisplayedQuestion - nPagedToQuestion);
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
         od_printf("`bright green`%c.`dark green`", chCurrent);
         od_printf(" %s\n\r", szQuestionName[nCurrent + nPagedToQuestion]);
      }

      /* Display prompt for input. */
      od_printf("\n\r`bright white`[Page %d]  Choose a question or",
         (nPagedToQuestion / QUESTION_PAGE_SIZE) + 1);
      if(nPagedToQuestion < nDisplayedQuestion - QUESTION_PAGE_SIZE)
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
            if(nPagedToQuestion < nDisplayedQuestion - QUESTION_PAGE_SIZE)
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
            /* Get question number from key pressed. */
            if(chCurrent >= '1' && chCurrent <= '9')
            {
               nCurrent = chCurrent - '1';
            }
            else
            {
               nCurrent = (chCurrent - 'A') + 9;
            }
         
            /* Add current paged to position to user's choice. */
            nCurrent += nPagedToQuestion;

            /* If this is valid question number. */            
            if(nCurrent < nDisplayedQuestion)
            {
               /* Set caller's current question number. */
               *nLocation = nPagedToQuestion;
            
               /* Return actual question number in file. */
               return(nQuestionNumber[nCurrent]);
            }
         }
      }
   }
}


/* The DisplayQuestionResult() function is called to display the results */
/* of voting on a paricular question, and is passed the question record  */
/* of the question. This function is called when the user selects a      */
/* question using the "view results" option, and is also called after    */
/* the user has voted on a question, to display the results of voting on */
/* that question.                                                        */
void DisplayQuestionResult(tQuestionRecord *pQuestionRecord)
{
   int nAnswer;
   int uPercent;

   /* Clear the screen. */
   od_clr_scr();

   /* Check that there have been votes on this question. */
   if(pQuestionRecord->uTotalVotes == 0)
   {
      /* If there have been no votes for this question, display a message */
      /* and return.                                                      */
      od_printf("Nobody has voted on this question yet.\n\r");
      WaitForEnter();
      return;
   }

   /* Display question itself. */
   od_printf("`bright red`%s\n\r", pQuestionRecord->szQuestion);

   /* Display author's name. */
   od_printf("`dark red`Question created by %s on %s\n\r",
      pQuestionRecord->szCreatorName,
      ctime(&pQuestionRecord->lCreationTime));
   
   /* Display heading for responses. */
   od_printf("`bright green`Response                        Votes  Percent  Graph\n\r`dark green`");
   if(od_control.user_ansi || od_control.user_avatar)
   {
      od_repeat((unsigned char)196, 79);
   }
   else
   {
      od_repeat('-', 79);
   }
   od_printf("\n\r");

   /* Loop for each answer to the question. */   
   for(nAnswer = 0; nAnswer < pQuestionRecord->nTotalAnswers; ++nAnswer)
   {
      /* Determine percent of users who voted for this answer. */
      uPercent = (pQuestionRecord->auVotesForAnswer[nAnswer] * 100)
         / pQuestionRecord->uTotalVotes;
      
      /* Display answer, total votes and percentage of votes. */
      od_printf("`dark green`%-30.30s  %-5u  %3u%%     `bright white`",
         pQuestionRecord->aszAnswer[nAnswer],
         pQuestionRecord->auVotesForAnswer[nAnswer],
         uPercent);

      /* Display a bar graph corresponding to percent of users who voted */
      /* for this answer.                                                */
      if(od_control.user_ansi || od_control.user_avatar)
      {
         od_repeat((unsigned char)220, (unsigned char)((uPercent * 31) / 100));
      }
      else
      {
         od_repeat('=', (unsigned char)((uPercent * 31) / 100));
      }

      /* Move to next line. */
      od_printf("\n\r");
   }
   
   /* Display footer. */
   od_printf("`dark green`");
   if(od_control.user_ansi || od_control.user_avatar)
   {
      od_repeat((unsigned char)196, 79);
   }
   else
   {
      od_repeat('-', 79);
   }
   od_printf("\n\r");
   od_printf("`dark green`                         TOTAL: %u\n\r\n\r",
      pQuestionRecord->uTotalVotes);
   
   /* Wait for user to press enter. */
   WaitForEnter();
}


/* The ReadOrAddCurrentUser() function is used by EZVote to search the    */
/* EZVote user file for the record containing information on the user who */
/* is currently using the door. If this is the first time that the user   */
/* has used this door, then their record will not exist in the user file. */
/* In this case, this function will add a new record for the current      */
/* user. This function returns TRUE on success, or FALSE on failure.      */
int ReadOrAddCurrentUser(void)
{
   FILE *fpUserFile;
   int bGotUser = FALSE;
   int nQuestion;

   /* Attempt to open the user file for exclusize access by this node.     */
   /* This function will wait up to the pre-set amount of time (as defined */   
   /* near the beginning of this file) for access to the user file.        */
   fpUserFile = ExculsiveFileOpen(USER_FILENAME, "a+b");

   /* If unable to open user file, return with failure. */   
   if(fpUserFile == NULL)
   {
      return(FALSE);
   }

   /* Begin with the current user record number set to 0. */
   nCurrentUserNumber = 0;

   /* Loop for each record in the file */
   while(fread(&CurrentUserRecord, sizeof(tUserRecord), 1, fpUserFile) == 1)
   {
      /* If name in record matches the current user name ... */
      if(strcmp(CurrentUserRecord.szUserName, od_control.user_name) == 0)
      {
         /* ... then record that we have found the user's record, */
         bGotUser = TRUE;
         
         /* and exit the loop. */
         break;
      }

      /* Move user record number to next user record. */      
      nCurrentUserNumber++;
   }

   /* If the user was not found in the file, attempt to add them as a */
   /* new user if the user file is not already full.                  */
   if(!bGotUser && nCurrentUserNumber < MAX_USERS)
   {
      /* Place the user's name in the current user record. */
      strcpy(CurrentUserRecord.szUserName, od_control.user_name);
      
      /* Record that user hasn't voted on any of the questions. */
      for(nQuestion = 0; nQuestion < MAX_QUESTIONS; ++nQuestion)
      {
         CurrentUserRecord.bVotedOnQuestion[nQuestion] = FALSE;
      }
      
      /* Write the new record to the file. */
      if(fwrite(&CurrentUserRecord, sizeof(tUserRecord), 1, fpUserFile) == 1)
      {
         /* If write succeeded, record that we now have a valid user record. */
         bGotUser = TRUE;
      }
   }

   /* Close the user file to allow other nodes to access it. */
   fclose(fpUserFile);

   /* Return, indciating whether or not a valid user record now exists for */
   /* the user that is currently online.                                   */   
   return(bGotUser);
}


/* The WriteCurrentUser() function is called to save the information on the */
/* user who is currently using the door, to the EZVOTE.USR file.            */
void WriteCurrentUser(void)
{
   FILE *fpUserFile;

   /* Attempt to open the user file for exclusize access by this node.     */
   /* This function will wait up to the pre-set amount of time (as defined */   
   /* near the beginning of this file) for access to the user file.        */
   fpUserFile = ExculsiveFileOpen(USER_FILENAME, "r+b");

   /* If unable to access the user file, display an error message and */
   /* return.                                                         */
   if(fpUserFile == NULL)
   {
      od_printf("Unable to access the user file.\n\r");
      WaitForEnter();
      return;
   }
   
   /* Move to appropriate location in user file for the current user's */
   /* record. */
   fseek(fpUserFile, (long)nCurrentUserNumber * sizeof(tUserRecord), SEEK_SET);

   /* Write the new record to the file. */
   if(fwrite(&CurrentUserRecord, sizeof(tUserRecord), 1, fpUserFile) == 1)
   {
      /* If unable to write the record, display an error message. */
      fclose(fpUserFile);
      od_printf("Unable to update your user record file.\n\r");
      WaitForEnter();
      return;
   }
   
   /* Close the user file to allow other nodes to access it again. */
   fclose(fpUserFile);
}


/* This function opens the specified file in the specified mode for         */
/* exculsive access by this node; while the file is open, other nodes will  */
/* be unable to open the file. This function will wait for up to the number */
/* of seconds set by FILE_ACCESS_MAX_WAIT, which is defined near the        */
/* beginning of this file.                                                  */
FILE *ExculsiveFileOpen(char *pszFileName, char *pszMode)
{
   FILE *fpFile = NULL;
   time_t StartTime = time(NULL);

   /* Attempt to open the file while there is still time remaining. */    
   while((fpFile = fopen(pszFileName, pszMode)) == NULL
      && errno == EACCES
      && difftime(time(NULL), StartTime) < FILE_ACCESS_MAX_WAIT)
   {
      /* If we were unable to open the file, call od_kernal, so that    */
      /* OpenDoors can continue to respond to sysop function keys, loss */
      /* of connection, etc.                                            */
      od_kernal();
   }

   /* Return FILE pointer for opened file, if any. */   
   return(fpFile);
}


/* The WaitForEnter() function is used by EZVote to create its custom */
/* "Press [ENTER] to continue." prompt.                               */
void WaitForEnter(void)
{
   /* Display prompt. */
   od_printf("`bright white`Press [ENTER] to continue.\n\r");
   
   /* Wait for a Carriage Return or Line Feed character from the user. */
   od_get_answer("\n\r");
}
