#define WIN32_LEAN_AND_MEAN 

#pragma warning(disable : 4996)

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <WinSock2.h>
#include "EncoderDecoder.h"

#if !defined(_Wp64)
#define DWORD_PTR DWORD
#define LONG_PTR LONG
#define INT_PTR INT
#endif

#define numItems  10

// There is code stored at the bottom of this program to test that the encoder and decoder are properly working
// in order to use it just copy and paste it into the main after the initialization of the encoderdecoder

typedef struct _WarehouseInfo {
	char characters[3];					// string of length 2 + null terminator
	int fileLoc;						// this is the location of the characters in the file
} WarehouseInfo;

// Producer and Consumer functions, what the thread(prod and cons) will run
void producer();
void consumer(int);

//SHARED DATA//
HANDLE semFilledSpots; 
HANDLE semEmptySpots; 
HANDLE mutexLockWarehouse;
HANDLE mutexLockFile;
HANDLE hInFile;
HANDLE hOutFile;

// creating the warehouse size 10
WarehouseInfo warehouse[numItems];		// warehouse is an array of structs holding the information for each item
int frontWH = 0;						// we are making a first in and last out thus this is the front count
int backWH = 0;							// this is the back 
BOOL producerAlive;
char encryptionType;
LARGE_INTEGER fileSize;
long long int fileSizeUse;
BOOL consumerAlive = TRUE;


int _tmain(int argc, LPTSTR argv[]) {
	// checking to make sure we have a sufficient number of parameters
	/*if (argc != 5) {
		print("Insufficient # of parameters passed in the command line, please make sure it is similarily formatted as followed: \'Assignment7 org.txt out.txt E 4\'");
		return 1;
	}*/
	
	InitializeEncoderDecoder();			// This is called once for setup

	const char* inFile = argv[1];		// we need to get the file path from the command line but not modifying it so const is a safety
	const char* outFile = argv[2];		// getting the name of what our output file will be	
	int numConsumers = atoi(argv[4]);	// num of consumers which comes from the 5th parameter in the command line
	/*int producingCount;	*/				// this is the file divided by 2 so that we know how many times the producer will need to add to the warehouse
	encryptionType = argv[3][0];

	
	hInFile = CreateFile(inFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
	hOutFile = CreateFile(outFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	HANDLE hProducer; 
	HANDLE* hConsumer = (HANDLE*)malloc(numConsumers * sizeof(HANDLE)); 

	GetFileSizeEx(hInFile, &fileSize);
	fileSizeUse = fileSize.QuadPart/* - 2*/;

	/*producingCount = (fileSize.QuadPart)/2;*/
	semFilledSpots = CreateSemaphore(NULL, 0, numItems, NULL); 
	semEmptySpots = CreateSemaphore(NULL, numItems, numItems, NULL);
	mutexLockWarehouse = CreateMutex(NULL, FALSE, NULL);
	mutexLockFile = CreateMutex(NULL, FALSE, NULL);

	producerAlive = TRUE;

	hProducer = (HANDLE)_beginthreadex(NULL, 0, producer, hInFile, NULL, NULL);
	for (int i = 0; i < numConsumers; i++) {
		hConsumer[i] = (HANDLE)_beginthreadex(NULL, 0, consumer, i, NULL, NULL);
	}

	WaitForSingleObject(hProducer, INFINITE);
	WaitForMultipleObjects(numConsumers, hConsumer, TRUE, INFINITE);

	// Clean up handles
	CloseHandle(hProducer);
	for (int i = 0; i < numConsumers; i++) {
		CloseHandle(hConsumer[i]);
	}

	return 0;
}

void producer() {
	/*SetFilePointer(HANDLE, location, NULL, FILE_BEGIN);*/
	/*for (int i = 0; i < count; i++) {
		WaitForSingleObject(SemEmptySpots, INFINITE);
	}*/
	DWORD BIn;
	int curPos = 0;

	while(TRUE) {
		WaitForSingleObject(semEmptySpots, INFINITE);

		// read in 2 bytes and handle odd size file
		ReadFile(hInFile, warehouse[backWH].characters, 2, &BIn, NULL);
		if (BIn == 1) {
			//pad the last spot with a space
			warehouse[backWH].characters[1] = ' ';
			fileSizeUse = fileSizeUse + 1;
		}

		// setting the fileLoc for later writing out
		warehouse[backWH].fileLoc = curPos;

		// changing the curPos by 2 so we know where the bytes will be written out and changing backWH
		curPos += 2;
		backWH = (backWH + 1) % numItems;

		ReleaseSemaphore(semFilledSpots, 1, NULL);

		if (BIn == 0) {
			break;
		}
	}
	producerAlive = FALSE;
}

void consumer(int who) {
	DWORD Bout;
	char data[3];
	int fileLoc2;

	while (consumerAlive) {
		//wait for filled sem
		if (!WaitForSingleObject(semFilledSpots, 100)) {
			//lock warehouse
			WaitForSingleObject(mutexLockWarehouse, INFINITE);

			//grab data from front and pull in two characters
			strncpy(data, warehouse[frontWH].characters, 2);
			data[2] = '\0';
			fileLoc2 = warehouse[frontWH].fileLoc;

			//change front counter
			frontWH = (frontWH + 1) % numItems;

			//release(emptysem)
			ReleaseSemaphore(semEmptySpots, 1, NULL);

			//unlock warehouse
			ReleaseMutex(mutexLockWarehouse);

			//now encrypt the data something like Encrypt(warehouse[#].characters)
			// If to check if we will be encrypting or decrypting information
			if (tolower(encryptionType) == 'e') {
				// data needs to be encrypted
				Encrypt(data);
			}
			else if (tolower(encryptionType) == 'd') {
				// data needs to be decrypted
				Decrypt(data);
			}
			else {
				printf("Uh ohhh, looks like the 4th parameter in the command line is not an E or D, fix and try again.");
				system("pause");
				abort();
			}

			if (fileLoc2 >= fileSizeUse) {
				consumerAlive = FALSE;
				break;
			}

			//lock the file
			WaitForSingleObject(mutexLockFile, INFINITE);

			//setfilepointer to the file location
			SetFilePointer(hOutFile, fileLoc2, NULL, FILE_BEGIN);

			//then write out to the file at that location
			WriteFile(hOutFile, data, 2, &Bout, NULL);

			//unlock file
			ReleaseMutex(mutexLockFile);

			
		}
		//if (!producerAlive && frontWH == backWH) {
		//	break;  // Exit the consumer loop
		//}
	}
}

/*Testing the encoder decoder*/
	//char String[3];
	//String[0] = 'Z';  // first character to encrypt
	//String[1] = 'B';  // second character to encrypt
	//String[2] = '\0';
	//Encrypt(String); // Encrypts the two characters.  The new characters are (,n)
	//printf("the encrypted string is: %s\n", String);
	//Decrypt(String);  // This Decrypts the characters (,n) back to (ZB)
	//printf("the decrypted string is: %s", String);