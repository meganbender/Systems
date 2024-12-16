#define WIN32_LEAN_AND_MEAN 

#pragma warning(disable : 4996)

#include <tchar.h>
#include <stdio.h>
#include <WinSock2.h>
#include <string.h>
#include <stdlib.h>

#if !defined(_Wp64)
#define DWORD_PTR DWORD
#define LONG_PTR LONG
#define INT_PTR INT
#define size 25
#define MAX_PROCESSES 10			// this is to help later with creating and looping through the array of structs
#endif

// creating a struct in order to hold process information so we can dynamically create the menus
// and allow the user to enter whatever process they want
typedef struct {
	char procName[size];
	PROCESS_INFORMATION procInfo;	// store the entire pi
	BOOL using;						// set this to false when we decide to not use it anymore or maybe we would be able
									// to just completly remove it from the array later on
} ProcessInfo; 

void startProcess(const char* command, ProcessInfo* processInfo, int* procCount) {
	STARTUPINFO si; 
	GetStartupInfo(&si);			//populates the startup info  

	// the following will create a fork bomb!!!!!!
	if (strcmp(processInfo->procName, "Assign1_MeganB") == 0) {
		for (int i = 2; i >= 0; i--) {
			if (i == 2) {
				printf("\nAre you sure you want to continue?\n");
				system("pause");
			}
			else if (i == 1) {
				printf("\nAgain, last chance... are you positive you want to continue??\n");
				system("pause");
			}
			else {
				printf("\nwow. Seriously... you really want to continue ahhh thats fine I guess.\n");
				system("pause");
			}
		}

		PROCESS_INFORMATION pi[MAX_PROCESSES];

		for (int i = 0; i < *procCount; i++) {
			if (!CreateProcess(NULL, 	 	// No module name (use command line) 
				"Assign1_MeganB",	   		 	// Command line, when researching I found that this is setting the typedef to char* and since we want to take whatever the user input is we will follow with command which is passed in
				NULL,				  	 	// Process handle not inheritable 
				NULL,				  	 	// Thread handle not inheritable 
				FALSE,			 	 	 	// Set handle inheritance to FALSE 
				0,				 		 	// No creation flags 
				NULL,			 	 	 	// Use parent's environment block 
				NULL,			 	 	 	// Use parent's starting directory  
				&si,		 		 	 	// Pointer to STARTUPINFO structure 
				&pi[i]) 	  	// Pointer to PROCESS_INFORMATION structure 
				)
			{
				printf("\nCould not start %s\n", command);
				processInfo->using = FALSE;
				//system("pause");
			}
			else {
				printf("\nStarted %s\n", command);
				processInfo->using = TRUE;
				//system("pause");
			}

			WaitForSingleObject(pi[i].hProcess, INFINITE); 
		}

		

		return;
	}
	else {
		char str[100];

		if (!CreateProcess(NULL,		// No module name (use command line) 
			(LPSTR)command,	 			// Command line, when researching I found that this is setting the typedef to char* and since we want to take whatever the user input is we will follow with command which is passed in
			NULL,						// Process handle not inheritable 
			NULL,						// Thread handle not inheritable 
			FALSE,						// Set handle inheritance to FALSE 
			0,							// No creation flags 
			NULL,						// Use parent's environment block 
			NULL,						// Use parent's starting directory  
			&si,						// Pointer to STARTUPINFO structure 
			&processInfo->procInfo)		// Pointer to PROCESS_INFORMATION structure 
			)
		{
			printf("\nCould not start %s\n", command); 
			processInfo->using = FALSE; 
			//system("pause");
		}
		else { 
			printf("\nStarted %s\n", command); 
			processInfo->using = TRUE; 
			//system("pause");
		}
	}
}

// later on when individuals choose a program to terminate instead of just setting something to false and just not display it
// with the number. Instead try to remove it completely from the array and repopulate the array
void terminateProcess(ProcessInfo* processInfoArr, int index, int* procCount) {
	if (processInfoArr[index].using) {

		// make sure its a handle to the process not the thread!
		if (!TerminateProcess(processInfoArr[index].procInfo.hProcess, 0)) {
			printf("\nError, failed to terminate the process: %s\n", processInfoArr[index].procName);
		}
		else {
			processInfoArr[index].using = FALSE;	// setting the using to false because we have terminated this process
			printf("\nSuccesfully terminated the process: %s\n", processInfoArr[index].procName);

			// Now I want to add a way to shift everything in the array 
			// i got stuck here when needing to make the procCount a pointer so it was never recognizing the correct value
			if (*procCount == 1) {
				// we should just set everything back to null; when looking into this i found a macro that
				// will set everything in a block of memory to 0 so I'm going to use it for when we have only one element left
				ZeroMemory(&processInfoArr[index], sizeof(ProcessInfo));	// ZeroMemory(buffer, sizeof(buffer));
			}
			else {
				for (int i = index; i < *procCount - 1; i++) {
					processInfoArr[i] = processInfoArr[i + 1];
				}
			}
			(*procCount)--; //NEEDED SO EVERYTHING GETS UPDATED
		}
	}
}

void createMenu(ProcessInfo* processInfoArr, int procCount) {
	// creating a "dynamic" menu basically it will kind of update while the program is running mainly due to us just completly removing
	// and updating the processInfoArr when we terminate a process
	
	printf("\nCurrent Processes Running:\n");

	for (int i = 0; i < procCount && i < MAX_PROCESSES; i++) { 
		if (processInfoArr[i].using) {
			printf("%d) Terminate (%s)\n", i + 1, processInfoArr[i].procName);
		}
	}
	printf("Q) Quit and Terminate all children\n\t->");
}

void terminateAllProcesses(ProcessInfo* processInfoArr, int* procCount) {
	if (*procCount > 0) {
		for (int i = *procCount - 1; i >= 0; i--) { 
			terminateProcess(processInfoArr, i, procCount);
		}
	}
}

int _tmain(int argc, LPTSTR argv[])
{
	// okay here i keep running into the problem if the user has entered more than the set maximum number of processes
	// after some research I found that we can use a conditional way of setting proc count, which technically is just an
	// if/else statement but it looks way cleaner!!!!!
	int procCount = (argc - 1 < MAX_PROCESSES) ? argc - 1 : MAX_PROCESSES;
	char userInput[size];
	int userMenuChoice = NULL;

	//printf("argc->%d\nargv[0]->%s\nargv[1]->%s\n", argc, argv[0], argv[1]);

	//we need to create an array of structs in order to dynamically create the pi information
	ProcessInfo processInfoArr[MAX_PROCESSES];

	// here we will continue to populate the array of structs; create a for loop that will populate the information
	// when copying the process information we will need to use strcpy() and runa for loop through the argv[]
	// then make sure to add a null at the end of the string name
	// the && shows we have multiple test conditions that only allows our for loop to continue running if they are satisfied 
	// (one condition for making sure we do not run into memory we arent allowed to access and one to make sure we only take the max number of processes allowed)
	for (int i = 0; i < procCount && i < MAX_PROCESSES; i++) { 
		strcpy(processInfoArr[i].procName, argv[i + 1]); 
		startProcess(argv[i + 1], &processInfoArr[i], &procCount); 
	}

	//// needed for checking
	//for (int i = 0; i < procCount; i++) {
	//	
	//	printf("item%d->%s\n", i, processInfoArr[i].procName);
	//}
	

	//ResumeThread(pi.hThread);
	//system("pause");

	/*WaitForSingleObject(pi.hProcess, 2000);*/

	do
	{
		createMenu(processInfoArr, procCount);
		fgets(userInput, sizeof(userInput), stdin);	// this is how we do stdin in C similar to cin in C++
		// we need to remove the \n that is added by fgets
		userInput[strcspn(userInput, "\n")] = 0;

		// now since the user can input either a number or a letter we need to convert the input to an integer
		userMenuChoice = atoi(userInput); 
		if (strcmp(tolower(userInput), "q") == 0) {
			break;
		}
		if (userMenuChoice > 0 && processInfoArr[userMenuChoice - 1].using) {
			terminateProcess(processInfoArr, userMenuChoice - 1, &procCount);
		}
		else {
			printf("Invalid choice, try again.\n");
		}


	} while (1);

	terminateAllProcesses(processInfoArr, &procCount);
	
	/*int value = TerminateProcess(pi.hProcess, 0);
	int error = GetLastError();
	pi.hProcess=OpenProcess(PROCESS_ALL_ACCESS, TRUE, pi.dwProcessId);
	value = TerminateProcess(pi.hProcess, 0);
	error = GetLastError();*/
	return 0;
}