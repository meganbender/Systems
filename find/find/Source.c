#define WIN32_LEAN_AND_MEAN 

#pragma warning(disable : 4996)

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <WinSock2.h>
#include <string.h> 

#if !defined(_Wp64)
#define DWORD_PTR DWORD
#define LONG_PTR LONG
#define INT_PTR INT
#endif

char* data;
DWORD fileSize;

typedef struct info
{
	char pattern[50];
	int ptrnCount;
	int ptrnLen;
	int arraySize;
	HANDLE hThread;
}ThreadRec, * pinfo_rec;

void findPattern(pinfo_rec);	// this function will look for the pattern

int _tmain(int argc, LPSTR argv[]) {
	printf("argc = %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}
	printf("Argument 1: %s\n", argv[1]); 
	////create a block of memory to hold the info from text file in stdin
	////this can be used just like an array!!!
	HANDLE hSTDIn = GetStdHandle(STD_INPUT_HANDLE);

	fileSize = GetFileSize(hSTDIn, NULL);
	data = (char*)malloc(fileSize);
	
	DWORD BIn; 
	if (!ReadFile(hSTDIn, data, fileSize, &BIn, NULL) || BIn != fileSize) {
		fprintf(stderr, "Failed to read file data.\n");
		free(data);
		return EXIT_FAILURE;
	}

	//creating a dynamic array of records
	ThreadRec* threadInfo;
	threadInfo = malloc(sizeof(ThreadRec) * (argc - 1));
	int numPatterns = argc - 1;

	//populating the array with the information
	for (int i = 0; i < numPatterns; i++) {

		threadInfo[i].ptrnCount = 0;					//initializing the count to 0
		//strcpy(threadInfo[i].pattern, argv[i + 1]);		//copying the string into the pattern for latter
		strncpy(threadInfo[i].pattern, argv[i + 1], sizeof(threadInfo[i].pattern) - 1); 
		threadInfo[i].pattern[sizeof(threadInfo[i].pattern) - 1] = '\0'; 
		threadInfo[i].ptrnLen = strlen(threadInfo[i].pattern);//strlen(argv[i + 1]);	//setting the length of the patter		
		threadInfo[i].hThread = (HANDLE)_beginthreadex(NULL, 0, findPattern, &threadInfo[i], CREATE_SUSPENDED, NULL);  //getting handle to thread
	}

	//loop through the created threads and resume them this is technically unnecessary
	for (int i = 0; i < numPatterns; i++) {
		ResumeThread(threadInfo[i].hThread);
	}

	//since the number of patterns being looked for can change we will create a for loop to also make hadle
	//the waitforsingleobject
	for (int i = 0; i < numPatterns; i++) {
		WaitForSingleObject(threadInfo[i].hThread, INFINITE);
	}

	for (int i = 0; i < numPatterns; i++) {
		printf("%d %s\n", threadInfo[i].ptrnCount, threadInfo[i].pattern);
	}

	system("pause");
	free(data);
	return 0;
}
//
//void findPattern(pinfo_rec data) {
//	int size = sizeof(data);
//
//	for (int pos = 0; pos < size - data->ptrnLen; pos++) {
//		if(strcmp(&data[pos], data->pattern))
//	}
//}

void findPattern(void* param) {
	pinfo_rec patternInfo = (pinfo_rec)param;

	for (int pos = 0; pos <= fileSize - patternInfo->ptrnLen; pos++) {
		if (strncmp(&data[pos], patternInfo->pattern, patternInfo->ptrnLen) == 0) {
			patternInfo->ptrnCount++;
		}
	}
}