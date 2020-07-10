// Thanh Nguyen
// CS344 / Spring 2019
// Assignment 4: otp_dec_d.c

/**************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

/**************************************************************************************************/

int main(int argc, char *argv[])
{
	
	/**************************************************************************************************/

	// Check usage & args
	if (argc < 2) { 
	
		fprintf(stderr,"USAGE: %s port\n", argv[0]); 
		exit(1); 
	}
	
	/**************************************************************************************************/

	// Get the port number, convert to an integer from a string
	int portNumber;
	portNumber = atoi(argv[1]);
	
	/**************************************************************************************************/

	// Set up the address struct for this process (the server)
	struct sockaddr_in serverAddress;
	
	// Clear out the address struct
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); 
	
	// Create a network-capable socket
	serverAddress.sin_family = AF_INET; 
	
	// Store the port number
	serverAddress.sin_port = htons(portNumber); 
	
	// Any address is allowed for connection to this process
	serverAddress.sin_addr.s_addr = INADDR_ANY; 

	/**************************************************************************************************/

	// Set up the socket
	int listenSocketFD;

	// Create the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
	
	if (listenSocketFD < 0) {
		fprintf(stderr, "otp_dec_d error: unable to open listening socket\n");
		exit(1);
	}
	
	/**************************************************************************************************/

	// Enable the socket to begin listening
	// Connect socket to port
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		fprintf(stderr, "otp_dec_d error: unable to bind socket\n");
		exit(1);
	}
		
	// Flip the socket on - it can now receive up to 5 connections
	listen(listenSocketFD, 5);
	
	/**************************************************************************************************/

	// Variables for fork
	pid_t pid = -5;			// to hold pid of child
	int childExitStatus;	// check exit status of child process
	
	// Continually accept connections
	while(1) 
	{
		
		/**************************************************************************************************/

		// Accept a connection, blocking if one is not available until one connects
		socklen_t sizeOfClientInfo;
		struct sockaddr_in clientAddress;
		
		// Get the size of the address for the client that will connect
		sizeOfClientInfo = sizeof(clientAddress); 
		
		int establishedConnectionFD;
		
		// Accept
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
		
		if (establishedConnectionFD < 0) {
			fprintf(stderr, "otp_dec_d error: unable to accept connection\n");
			exit(1);
		}
		
		/**************************************************************************************************/
		
		// fork
		pid = fork();
		
		// if the fork fails, notify user
		if (pid == -1) {
			fprintf(stderr, "otp_dec_d error: fork failed\n");
			exit(1);
		}
		
		// child process
		else if (pid == 0) {
		
			/**************************************************************************************************/
			
			// Receive the initial flag from the client to check that it is connecting to the right server
			char buffer[70000];
			memset(buffer, '\0', sizeof(buffer));
			int charsRead;
			int charsWritten;
	
			// Read data from the socket, leaving \0 at end
			charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0); 
			
			// If we couldn't read from the socket
			if (charsRead < 0) { 
				fprintf(stderr, "otp_dec_d error: issue receiving encode/decode flag from client\n");
				exit(1);
			}
			
			/**************************************************************************************************/
			
			// Check to see if client is not trying to decode
			if (strcmp(buffer, "decode") != 0) {

				// Continue to try to send "incorrect" message back to client 
				do {
					
					// Send a "incorrect" message back to the client
					charsWritten = send(establishedConnectionFD, "incorrect", 9, 0);
					
					// Print an error if we are unable to send the message back to the client
					if (charsWritten < 0) {
						fprintf(stderr, "otp_enc_d error: issue sending wrong server message to client\n");
						exit(1);
					}
				} while (charsWritten < 9);
				
				// Have the child exit since the client is hitting incorrect server
				exit(0);
			}
			
			else {
				
				// Continue to try to send "correct" message back to client 
				do {
					
					// Send a "incorrect" message back to the client
					charsWritten = send(establishedConnectionFD, "correct", 7, 0);
					
					// Print an error if we are unable to send the message back to the client
					if (charsWritten < 0) {
						fprintf(stderr, "otp_dec_d error: issue sending correct server message to client\n");
						exit(1);
					}
				} while (charsWritten < 7);
			}
			
			/**************************************************************************************************/
			
			// Variables to get the message from the client
			char cipher[70000], key[70000], readBuffer[700];
			
			// Clear the buffer
			memset(cipher, '\0', sizeof(cipher));
			
			// As long as we haven't found the sentinel
			while (strstr(cipher, "@@") == NULL) 
			{
				// Clear the buffer
				memset(readBuffer, '\0', sizeof(readBuffer)); 
				
				// Get a chunk of the client's message from the server
				charsRead = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer), 0); 
				
				// Add that chunk to what we have so far
				strcat(cipher, readBuffer); 
								
				// Check for errors
				if (charsRead < 0) { 
					fprintf(stderr, "otp_dec_d error: issue receiving cipher from client\n");
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
			
			/**************************************************************************************************/

			// Continue to try to send success message back to client 
			do {
				
				// Send a success message back to the client
				charsWritten = send(establishedConnectionFD, "Success", 7, 0);
				
				// Print an error if we are unable to send the success message back to the client
				if (charsWritten < 0) {
					fprintf(stderr, "otp_dec_d error: issue sending cipher-receive success to client\n");
					exit(1);
				}
			} while (charsWritten < 7);
			
			/**************************************************************************************************/
			
			// Clear the buffer
			memset(key, '\0', sizeof(key));
			
			// As long as we haven't found the sentinel
			while (strstr(key, "@@") == NULL) 
			{
				// Clear the buffer
				memset(readBuffer, '\0', sizeof(readBuffer)); 
				
				// Get a chunk of the client's message from the server
				charsRead = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer), 0); 
				
				// Add that chunk to what we have so far
				strcat(key, readBuffer); 
								
				// Check for errors
				if (charsRead < 0) { 
					fprintf(stderr, "otp_dec_d error: issue receiving key from client\n");
					exit(1);
				}
				
				if (charsRead == 0) {
					break;
				}
			
			}
			
			// Where is the sentinel
			terminalLocation = strstr(key, "@@") - key; 
			
			// End the string early to wipe out the terminal
			key[terminalLocation] = '\0'; 
			
			/**************************************************************************************************/
			
			// Decode
			// int array to hold ascii values 
			int asciiArr[70000];
			int ascii;
			char c;
			
			char plaintext[70000];
			memset(plaintext, '\0', sizeof(plaintext));
			
			// Convert cipher characters into ascii values and store into table
			int i = 0;
			for (i = 0; i < strlen(cipher); i++) {
			
				// Cast each char into its ascii value
				ascii = (int) cipher[i];
				
				// Convert the ascii value into its 0-index value
				// If the character is a space, set it 0-index value to 26
				if (ascii == 32) {
					ascii = 26;
				}
				else {
					ascii = ascii - 65;
				}
				
				asciiArr[i] = ascii;
				
			}
			
			// Convert key characters into ascii values and add to values in table
			for (i = 0; i < strlen(key); i++) {
				
				// Cast each char into its ascii value
				ascii = (int) key[i];
				
				// Convert the ascii value into its 0-index value
				// If the character is a space, set it 0-index value to 26
				if (ascii == 32) {
					ascii = 26;
				}
				else {
					ascii = ascii - 65;
				}
								
				// Subtract the ascii value the existing ascii value in the array
				asciiArr[i] = asciiArr[i] - ascii;
								
				// Mod the ascii sum
				asciiArr[i] = asciiArr[i] % 27;
				
				if (asciiArr[i] < 0) {
					asciiArr[i] = asciiArr[i] + 27;
				}
								
				// Convert back to real ascii value
				if (asciiArr[i] == 26) {
					asciiArr[i] = 32;
				}
				else {
					asciiArr[i] = asciiArr[i] + 65;
				}
				
				// Convert to char value
				c = asciiArr[i];
				
				// Build plaintext from chars
				plaintext[i] = c;
			}
			
			// Prepare the plaintext by adding sentinel to it
			int len = strlen(plaintext);
			
			// Add the "@@" sentinel
			plaintext[len] = '@';
			plaintext[len+1] = '@';
			
			/**************************************************************************************************/
			
			do {
				
				// Return the plaintext back to the client
				charsWritten = send(establishedConnectionFD, plaintext, strlen(plaintext), 0); 
				
				// Return an error if we are unable to send back plaintext
				if (charsWritten < 0) {
					fprintf(stderr, "otp_enc_d error: issue sending plaintext to client\n");
					exit(1);
				}
			} while (charsWritten < strlen(plaintext));
				
			/**************************************************************************************************/
			
			// Close the existing socket which is connected to the client
			close(establishedConnectionFD); 
			
			// Exit success
			exit(0);
		
		}
		
		// parent process
		else {
			
			// Reap background child processes
			while(1) {
				
				// If there are no more children to reap, break
				if (waitpid(-1, &childExitStatus, WNOHANG) == 0) {
					break;
				}
			}
		}
	}
	
	/**************************************************************************************************/
	
	// Close the listening socket
	close(listenSocketFD); 
	
	// Exit success
	exit(0);
}
