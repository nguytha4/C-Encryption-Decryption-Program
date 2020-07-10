/* Thanh Nguyen */
/* CS344 / Spring 2019 */
/* Assignment #4: keygen */

/***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/***************************************************************************/

int main( int argc, char *argv[] )  {

	/* Check to make sure the user only entered one argument */
	if (argc != 2) {
		printf("Please enter a single integer as an argument.\n");
		exit(0);
	}

	else {
		
		// Get user's input as a string
		char * userInput = NULL;
		userInput = argv[1];
		
		/* check to see if what user entered is a number */
		int i = 0;
		for (i = 0; i < strlen(userInput); i++) {
			if (isdigit(userInput[i]) == 0) {
				printf("Argument can only be an integer.\n");
				exit(0);
			}
		}
		
		/* convert the user's argument into the length of the string that we'll be generating */
		int strLen = atoi(argv[1]);
		
		/* variables to generate numbers to represent ASCII values to concatonate into a string */
		int randNum = -5;
		char randChar;
		char key[80000];
		memset(key, '\0', sizeof(key));
		
		/* randomize seed */
		srand(time(NULL));
		
		/* generate random numbers representing the ASCII range and then add them to the final buffer */
		for (i = 0; i < strLen; i++) {
			randNum = rand() % (91 + 1 - 65) + 65;
			
			/* if the ASCII value is 91, convert it to 32 to represent the space character */
			if (randNum == 91)
				randNum = 32;
			randChar = (char)randNum;
			key[i] = randChar;
		}
		
		/* print the final key */
		printf("%s\n", key);
		
		// Exit success
		exit(0);
	}

}