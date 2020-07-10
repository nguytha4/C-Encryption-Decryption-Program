// Thanh Nguyen
// CS344 / Spring 2019
// Assignment 4: otp_enc.c

/**************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

/**************************************************************************************************/

// Function to check if there are any bad characters in the plaintext or key files
int badchar (char * fileIn) {
	
	// Variables to open file and check contents
	FILE * fp;
	char fileLine[70000];
	memset(fileLine, '\0', sizeof(fileLine));
	int len;
	int x = 0;
	int ascii = -5;
	
	// Open the file
	fp = fopen(fileIn, "r");
	
	if (fp == NULL) {
		fprintf(stderr, "Error: File '%s' could not be found in directory\n", fileIn);
		exit(1);
	}
	
	// Get a line from the file
	fgets(fileLine, sizeof(fileLine), fp);
	
	// Remove the trailing newline
	len = strlen(fileLine);
	if(fileLine[len-1] == '\n') {
		
		fileLine[len-1] = '\0';
	}
	
	// Decrement the string length
	len--;
	
	// Get the ASCII values of the buffer
	for (x = 0; x < len; x++) {
		
		// Cast the char into its ascii values
		ascii = (int) fileLine[x];
		
		// If the ascii value is less than 65 and not 32 or
		//		the ascii value is greater than 90
		if ((ascii < 65 && ascii != 32) || (ascii > 90)) {
			
			// Close the file and return true on invalid character
			fclose(fp);
			return 1;
		}
	}
	
	// Close the file
	fclose(fp);
	
	// Return false on bad characters
	return 0;
}

/**************************************************************************************************/

// Function to check if the length of the key is less than the length of the plaintext file
int shortkey(char * plaintext, char * key) {
	
	// Variables to open file and check contents
	FILE * fp;
	char fileLine[70000];
	memset(fileLine, '\0', sizeof(fileLine));
	int plaintextLen = -5;
	int keyLen = -5;
	
	// Open the plaintext file
	fp = fopen(plaintext, "r");
	
	if (fp == NULL) {
		fprintf(stderr, "Error: File '%s' could not be found in directory\n", plaintext);
		exit(1);
	}
	
	// Get a line from the file
	fgets(fileLine, sizeof(fileLine), fp);
	
	// Get the length of the plaintext file
	plaintextLen = strlen(fileLine);
	
	// Close the file
	fclose(fp);

	// Open the key file
	fp = fopen(key, "r");
	
	if (fp == NULL) {
		fprintf(stderr, "Error: File '%s' could not be found in directory\n", key);
		exit(1);
	}
	
	// Get a line from the file
	fgets(fileLine, sizeof(fileLine), fp);
	
	// Get the length of the plaintext file
	keyLen = strlen(fileLine);
	
	// Close the file
	fclose(fp);
	
	// If the key length is shorter than the plaintext length
	if (keyLen < plaintextLen) {
		// Return true for short key
		return 1;
	}
	
	// Return false for short key
	return 0;
}

/**************************************************************************************************/

int main(int argc, char *argv[])
{
	
	// Check usage & args
	if (argc < 4) { 
	
		fprintf(stderr,"USAGE: otp_enc plaintext key port\n", argv[0]); 
		exit(1); 
	}
	
	/**************************************************************************************************/

	// Variables to hold the names of the plaintext and key files
	char * plaintextFileNm = NULL;
	char * keyFileNm = NULL;
	
	// Link the names of the plaintext and key files to their string variables
	plaintextFileNm = argv[1];
	keyFileNm = argv[2];
	
	// If there are any bad characters in either file
	if(badchar(plaintextFileNm) == 1 || badchar(keyFileNm) == 1) {
		
		// Print an error to the user
		fprintf(stderr, "otp_enc error: input contains bad characters\n");
		exit(1);
	}
	
	// If the key length is shorter than the plaintext length
	if (shortkey(plaintextFileNm, keyFileNm) == 1) {
		
		// Print an error to the user
		fprintf(stderr, "Error: key ‘%s’ is too short\n", keyFileNm);
		exit(1);
	}
	
	/**************************************************************************************************/
	
	// Variables to hold contents of the plaintext and key files
	char plaintext[70000];
	memset(plaintext, '\0', sizeof(plaintext));
	char key[70000];
	memset(key, '\0', sizeof(key));
	
	// Variables to open file and check contents
	FILE * fp;
	int len;
	
	// Open the plaintext file
	fp = fopen(plaintextFileNm, "r");
	
	if (fp == NULL) {
		fprintf(stderr, "Error: File '%s' could not be found in directory\n", plaintextFileNm);
		exit(1);
	}
	
	// Get a line from the file
	fgets(plaintext, sizeof(plaintext), fp);
	
	// Close the file
	fclose(fp);
	
	// Remove the trailing newline and add the "@@" sentinel to the plaintext contents
	len = strlen(plaintext);
	if(plaintext[len-1] == '\n') {
		plaintext[len-1] = '@';
		plaintext[len] = '@';
	}
	
	// Open the key file
	fp = fopen(keyFileNm, "r");
	
	if (fp == NULL) {
		fprintf(stderr, "Error: File '%s' could not be found in directory\n", keyFileNm);
		exit(1);
	}
	
	// Get a line from the file
	fgets(key, len, fp);
	
	// Close the file
	fclose(fp);
	
	// Remove the trailing newline and add the "@@" sentinel to the key
	key[len-1] = '@';
	key[len] = '@';
	
	/**************************************************************************************************/

	// Get the port number, convert to an integer from a string
	int portNumber;
	portNumber = atoi(argv[3]); 
	
	/**************************************************************************************************/

	// Set up the server address struct and clear it out
	struct sockaddr_in serverAddress;
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 
	
	// Create a network-capable socket
	serverAddress.sin_family = AF_INET;
	
	// Store the port number
	serverAddress.sin_port = htons(portNumber); 
	
	/***************************************************************/
	
	// Address of target
	struct hostent* serverHostInfo;
	
	// Convert the machine name into a special form of address
	serverHostInfo = gethostbyname("localhost"); 
	
	if (serverHostInfo == NULL) { 
		fprintf(stderr, "otp_enc error: no such host\n"); 
		exit(1); 
	}
	
	// Copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); 

	/**************************************************************************************************/

	// Set up the socket
	int socketFD;
	
	// Create the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	
	// If there was an error creating the socket, inform the user
	if (socketFD < 0) {
		fprintf(stderr, "otp_enc error: issue opening socket\n");
		exit(1);
	}
	
	/**************************************************************************************************/
		
	// Connect socket to address
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		
		fprintf(stderr, "otp_enc error: issue connecting to server\n");
		exit(1);
	}
		
	/**************************************************************************************************/
	
	// Need to ensure we are connecting to the right server
	int charsWritten = 0;
	
	do {
		
		// Let the server know that this client is trying to encode by sending an "encode" message
		charsWritten = send(socketFD, "encode", 6, 0); 
		
		// If we couldn't write to the socket
		if (charsWritten < 0) {
			fprintf(stderr, "otp_enc error: issue sending encode flag to server\n");
			exit(1);
		}
		
	} while (charsWritten < 6);
	
	/**************************************************************************************************/
	
	// Get return message from server
	char buffer[70000];
	memset(buffer, '\0', sizeof(buffer)); 
	int charsRead;
	
	// Clear out the buffer again for reuse
	memset(buffer, '\0', sizeof(buffer)); 
	
	// Read data from the socket, leaving \0 at end
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
	
	// If we couldn't read from the socket
	if (charsRead < 0) { 
		fprintf(stderr, "otp_enc error: issue receiving confirmation message from server\n");
		exit(1);
	}
		
	// If the return message says incorrect
	if (strcmp(buffer, "incorrect") == 0) {
		
		fprintf(stderr, "Error: could not contact otp_enc_d on port %d\n", portNumber);
		exit(2);
	}
	
	/**************************************************************************************************/
	
	do {
		
		// Write plaintext contents to the server
		charsWritten = send(socketFD, plaintext, strlen(plaintext), 0); 
		
		// If we couldn't write to the socket
		if (charsWritten < 0) {
			fprintf(stderr, "otp_enc error: issue sending plaintext contents to server\n");
			exit(1);
		}
		
	} while (charsWritten < strlen(plaintext));
	
	/**************************************************************************************************/

	// Clear out the buffer again for reuse
	memset(buffer, '\0', sizeof(buffer)); 
	
	// Read data from the socket, leaving \0 at end
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
	
	// If we couldn't read from the socket
	if (charsRead < 0) { 
		fprintf(stderr, "otp_enc error: issue receiving plaintext-received success flag from server\n");
		exit(1);
	}

	/**************************************************************************************************/

	do {
		
		// Write key contents to the server
		charsWritten = send(socketFD, key, strlen(key), 0); 
		
		// If we couldn't write to the socket
		if (charsWritten < 0) {
			fprintf(stderr, "otp_enc error: issue sending key to server\n");
			exit(1);
		}

	} while (charsWritten < strlen(key));

	/**************************************************************************************************/
	
	// Receive the cipher from the server
	char cipher[70000], readBuffer[700];
	memset(cipher, '\0', sizeof(cipher));

	// As long as we haven't found the sentinel
	while (strstr(cipher, "@@") == NULL) 
	{
		// Clear the buffer
		memset(readBuffer, '\0', sizeof(readBuffer)); 
		
		// Get a chunk of the client's message from the server
		charsRead = recv(socketFD, readBuffer, sizeof(readBuffer), 0); 
		
		// Add that chunk to what we have so far
		strcat(cipher, readBuffer); 
				
		// Check for errors
		if (charsRead < 0) { 
			fprintf(stderr, "otp_enc error: issue receiving cipher from server\n");
			exit(1);
		}
		
		if (charsRead == 0) {
			break;
		}
	
	}
	
	// Where is the sentinel
	int terminalLocation = strstr(cipher, "@@") - cipher; 
	
	// End the string early to wipe out the terminal
	cipher[terminalLocation] = '\0'; 
	
	// Print the cipher
	printf("%s\n", cipher);
	
	/**************************************************************************************************/

	// Close the socket
	close(socketFD); 
	
	// Exit success
	exit(0);
}
