#define WIN32_LEAN_AND_MEAN 

#pragma warning(disable : 4996)

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <WinSock2.h>

#if !defined(_Wp64)
#define DWORD_PTR DWORD
#define LONG_PTR LONG
#define INT_PTR INT
#endif

HANDLE STDIn, STDOut;
HANDLE* transFiles;
HANDLE* hThreads;
LARGE_INTEGER FileSize;
DWORD BIn; 
int numAccounts;

typedef struct ACCOUNT_INFO_REC {
    int accountNumber;
    int balance;
} ACCOUNT_INFO, *P_ACT_INFO;

ACCOUNT_INFO* accountInfoRec;
CRITICAL_SECTION* accountsCS;  //critical section for accounts

WORD Transactions(HANDLE*);

int _tmain(int argc, LPSTR argv[]) {
	//testing command line
	/*printf("argc = %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}*/
	// make sure to use critical sections for each thread
	DWORD BOut;
	numAccounts; 
	int numTransFiles = argc - 1;

	STDIn = GetStdHandle(STD_INPUT_HANDLE);
	STDOut = GetStdHandle(STD_OUTPUT_HANDLE); 
	GetFileSizeEx(STDIn, &FileSize);
	
	numAccounts = FileSize.QuadPart / 8;

	//dynamic array of the accounts and their info
	accountInfoRec = malloc(numAccounts * sizeof(ACCOUNT_INFO));

	//dynamic array of the critical sections for each account
	accountsCS = malloc(numAccounts * sizeof(CRITICAL_SECTION)); 

	//dynamic array of the transaction file names
	transFiles = malloc(numTransFiles * sizeof(HANDLE));

	//dynamic array of the handles for the threads completing completing the transaciton work
	hThreads = malloc(numTransFiles * sizeof(HANDLE));

	//making sure we actually read everything in 
	if (!ReadFile(STDIn, accountInfoRec, sizeof(ACCOUNT_INFO) * numAccounts, &BIn, NULL)) {
		//make it in if we did not succesfully read te file
		printf("Error reading file or unexpected file size.\n");
		return 0;
	}

	//used for debugging
	/*printf("Read %ld bytes. Expected: %ld bytes.\n", BIn, numAccounts * sizeof(ACCOUNT_INFO));
	if (BIn != numAccounts * sizeof(ACCOUNT_INFO)) {
		printf("Warning: Did not read the expected number of bytes.\n");
	}*/

	for (int x = 0; x < numAccounts; x++)
	{
		InitializeCriticalSection(&accountsCS[x]);
	}

	//aslo used for debugging
	/*printf("\nPrinted data after reading from file:\n");
	for (int i = 0; i < numAccounts; ++i) {
		printf("Account %d: %ld balance\n", account_info_rec[i].accountNumber, account_info_rec[i].balance); 
	}*/

	//get the transaction files
	for (int i = 1; i < argc; i++)
	{
		transFiles[i-1] = CreateFile(argv[i],
				GENERIC_READ, 0, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL);	
	}

	//begin the threads to complete the transactions
	for (int i = 0; i < numTransFiles; i++)
	{
		hThreads[i] = (HANDLE)_beginthreadex(NULL, 0, Transactions, transFiles[i], 0, NULL);
		if (hThreads[i] == NULL)
			printf("Could not start up thread# %d !", i);
	}

	//have the parent program wait for the 
	for (int i = 0; i < numTransFiles; i++)
	{
		WaitForSingleObject(hThreads[i], INFINITE);
	}

	char* outputFileName = argv[argc - 1]; 

	//now writing out to a binary file
	for (int i = 0; i < numAccounts; i++) {
		if (!WriteFile(STDOut, &accountInfoRec[i], sizeof(ACCOUNT_INFO), &BOut, NULL)) {
			printf("Error writing to output file");
			return;
		}
	}

	//this kept writing straight to my output file since i started to redirect so i just commented it out
	/*system("pause");*/

	for (int i = 0; i < argc - 1; i++) {
		CloseHandle(transFiles[i]);
	}
	for (int i = 0; i < numAccounts; i++) {
		DeleteCriticalSection(&accountsCS[i]);
	}
	free(accountInfoRec);  
	free(accountsCS); 
	free(transFiles); 
	 
	return 0;
}

WORD Transactions(HANDLE* hCurFile) {
	//local variables
	LARGE_INTEGER fileSize2; 
	GetFileSizeEx(hCurFile, &fileSize2);
	int numtrans = fileSize2.QuadPart / 8;
	int accountIndex;

	for (int i = 0; i < numtrans; i++) {
		/*RECORD r1;
		ReadFile(FileHandle, &r1, sizeof(RECORD), &BIn, NULL);*/
		ACCOUNT_INFO acctTransInfo; 
		if (!ReadFile(hCurFile, &acctTransInfo, sizeof(ACCOUNT_INFO), &BIn, NULL)) {
			printf("Error reading the transaction file!");
			return 0;
		}

		accountIndex = NULL;
		for (int j = 0; j < numAccounts; j++) {
			if (accountInfoRec[j].accountNumber == acctTransInfo.accountNumber) {
				accountIndex = j;
				break;
			}
		}

		EnterCriticalSection(&accountsCS[accountIndex]);

		__try {
			//here we need to add the change to the account balance
			accountInfoRec[accountIndex].balance += acctTransInfo.balance;
		}
		__finally {
			LeaveCriticalSection(&accountsCS[accountIndex]);	
		}
	}

	return 1;
}