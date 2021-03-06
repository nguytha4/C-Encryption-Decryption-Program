// Thanh Nguyen
// CS344 / Spring 2019
// Assignment 4: otp_dec.c

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

// Function to check if there are any bad characters in the cipher or key files
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
	
	// Print the ASCII values of the buffer
	for (x = 0; x < len; x++) {
		
		// Cast the char into its ascii values
		ascii = (int) fileLine[x];
		
		// If the ascii value is less than 65 and not 32 or
		//		the ascii value is greater than 90
		if ((ascii < 65 && ascii != 32) || (ascii > 90)) {
			
			// Close the file and return true on character
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

// Function to check if the length of the key is less than the length of the cipher file
int shortkey(char * cipher, char * key) {
	
	// Variables to open file and check contents
	FILE * fp;
	char fileLine[70000];
	memset(fileLine, '\0', sizeof(fileLine));
	int cipherLen = -5;
	int keyLen = -5;
	
	// Open the cipher file
	fp = fopen(cipher, "r");
	
	if (fp == NULL) {
		fprintf(stderr, "Error: File '%s' could not be found in directory\n", cipher);
		exit(1);
	}
	
	// Get a line from the file
	fgets(fileLine, sizeof(fileLine), fp);
	
	// Get the length of the cipher file
	cipherLen = strlen(fileLine);
	
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
	
	// Get the length of the cipher file
	keyLen = strlen(fileLine);
	
	// Close the file
	fclose(fp);
	
	// If the key length is shorter than the cipher length
	if (keyLen < cipherLen) {
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
	
		fprintf(stderr,"USAGE: otp_dec cipher key port\n", argv[0]); 
		exit(1); 
	}
	
	/**************************************************************************************************/

	// Variables to hold the names of the cipher and key files
	char * cipherFileNm = NULL;
	char * keyFileNm = NULL;
	
	// Link the names of the cipher and key files to their string variables
	cipherFileNm = argv[1];
	keyFileNm = argv[2];
	
	// If there are any bad characters in either file
	if(badchar(cipherFileNm) == 1 || badchar(keyFileNm) == 1) {
		
		// Print an error to the user
		fprintf(stderr, "otp_dec error: input contains bad characters\n");
		exit(1);
	}
	
	// If the key length is shorter than the cipher length
	if (shortkey(cipherFileNm, keyFileNm) == 1) {
		
		// Print an error to the user
		fprintf(stderr, "Error: key ‘%s’ is too short\n", keyFileNm);
		exit(1);
	}
	
	/**************************************************************************************************/
	
	// Variables to hold contents of the cipher and key files
	char cipher[70000];
	memset(cipher, '\0', sizeof(cipher));
	char key[70000];
	memset(key, '\0', sizeof(key));
	
	// Variables to open file and check contents
	FILE * fp;
	int len;
	
	// Open the file
	fp = fopen(cipherFileNm, "r");
	
	if (fp == NULL) {
		fprintf(stderr, "Error: File '%s' could not be found in directory\n", cipherFileNm);
		exit(1);
	}
	
	// Get a line from the file
	fgets(cipher, sizeof(cipher), fp);
	
	// Close the file
	fclose(fp);
	
	// Remove the trailing newline and add the "@@" sentinel
	len = strlen(cipher);
	if(cipher[len-1] == '\n') {
		cipher[len-1] = '@';
		cipher[len] = '@';
	}
	
	// Open the file
	fp = fopen(keyFileNm, "r");
	
	if (fp == NULL) {
		fprintf(stderr, "Error: File '%s' could not be found in directory\n", keyFileNm);
		exit(1);
	}
	
	// Get a line from the file
	fgets(key, len, fp);
	
	// Close the file
	fclose(fp);
	
	// Add the "@@" sentinel
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
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(1); 
	}
	
	// Copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); 

	/**************************************************************************************************/

	// Set up the socket
	int socketFD;
	
	// Create the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	
	if (socketFD < 0) {
		fprintf(stderr, "otp_dec error: issue opening socket\n");
		exit(1);
	}
	
	/**************************************************************************************************/
		
	// Connect socket to address
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		
		fprintf(stderr, "otp_dec error: issue connecting to server\n");
		exit(1);
	}
		
	/**************************************************************************************************/

	// Need to ensure we are connecting to the right server
	int charsWritten = 0;
	
	do {
		
		// Let the server know that this client is trying to encode by sending an "decode" message
		charsWritten = send(socketFD, "decode", 6, 0); 
		
		// If we couldn't write to the socket
		if (charsWritten < 0) {
			fprintf(stderr, "otp_enc error: issue sending decode flag to server\n");
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
		fprintf(stderr, "otp_dec error: issue receiving confirmation message from server\n");
		exit(1);
	}
		
	// If the return message says incorrect
	if (strcmp(buffer, "incorrect") == 0) {
		
		fprintf(stderr, "Error: could not contact otp_dec_d on port %d\n", portNumber);
		exit(2);
	}

	/**************************************************************************************************/
	
	do {
	
		// Write cipher contents to the server
		charsWritten = send(socketFD, cipher, strlen(cipher), 0); 
		
		// If we couldn't write to the socket
		if (charsWritten < 0) {
			fprintf(stderr, "otp_enc error: issue sending cipher contents to server\n");
			exit(1);
		}
		
	} while (charsWritten < strlen(cipher));
	
	/**************************************************************************************************/

	// Clear out the buffer again for reuse
	memset(buffer, '\0', sizeof(buffer)); 
	
	// Read data from the socket, leaving \0 at end
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
	
	// If we couldn't read from the socket
	if (charsRead < 0) { 
		fprintf(stderr, "otp_dec error: issue receiving cipher-received success flag from server\n");
		exit(1);
	}

	/**************************************************************************************************/

	do {
		
		// Write key contents to the server
		charsWritten = send(socketFD, key, strlen(key), 0); 
		
		// If we couldn't write to the socket
		if (charsWritten < 0) {
			fprintf(stderr, "otp_dec error: issue sending key to server\n");
			exit(1);
		}

	} while (charsWritten < strlen(key));

	/**************************************************************************************************/
	
	// Receive the plaintext from the server
	char plaintext[70000], readBuffer[700];
	memset(plaintext, '\0', sizeof(plaintext));

	// As long as we haven't found the sentinel
	while (strstr(plaintext, "@@") == NULL) 
	{
		// Clear the buffer
		memset(readBuffer, '\0', sizeof(readBuffer)); 
		
		// Get a chunk of the client's message from the server
		charsRead = recv(socketFD, readBuffer, sizeof(readBuffer), 0); 
		
		// Add that chunk to what we have so far
		strcat(plaintext, readBuffer); 
				
		// Check for errors
		if (charsRead < 0) { 
			fprintf(stderr, "otp_dec error: issue receiving plaintext from server\n");
			exit(1);
		}
		
		if (charsRead == 0) {
			break;
		}
	
	}
	
	// Where is the sentinel
	int terminalLocation = strstr(plaintext, "@@") - plaintext; 
	
	// End the string early to wipe out the terminal
	plaintext[terminalLocation] = '\0'; 
	
	// Print the plaintext
	printf("%s\n", plaintext);
	
	/**************************************************************************************************/

	// Close the socket
	close(socketFD); 
	
	// Exit success
	exit(0);
}
