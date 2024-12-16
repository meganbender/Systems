#define WIN32_LEAN_AND_MEAN 

#pragma warning(disable : 4996)

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <WinSock2.h>
#include <math.h>

#if !defined(_Wp64)
#define DWORD_PTR DWORD
#define LONG_PTR LONG
#define INT_PTR INT
#endif

#define DATALEN 56
#define KEYLEN 8
typedef struct _RECORD {
	TCHAR key[KEYLEN];
	TCHAR data[DATALEN];
} RECORD;

#define RECSIZE sizeof (RECORD)

RECORD ReadRecord(HANDLE STDInput, int spot_o) {
	DWORD BIn;
	RECORD temp;
	LARGE_INTEGER spot;
	spot.QuadPart = spot_o * 64;
	SetFilePointerEx(STDInput, spot, NULL, FILE_BEGIN);
	ReadFile(STDInput, &temp, RECSIZE, &BIn, NULL);
	return temp;
}

void WriteRecord(HANDLE STDOutput, int spot_o, RECORD to_write) {
	DWORD Bout;

	LARGE_INTEGER spot;
	spot.QuadPart = spot_o * 64;
	SetFilePointerEx(STDOutput, spot, NULL, FILE_BEGIN);
	WriteFile(STDOutput, &to_write, RECSIZE, &Bout, NULL);
	//return temp;
}

HANDLE CreateTempFile(TCHAR* prefix, TCHAR* tempFileName) {
	SECURITY_ATTRIBUTES stdOutSA = /* SA for inheritable handle. */ 
	{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };  

	if (GetTempFileName(_T("."), prefix, 0, tempFileName) != 0) {
		return CreateFile(tempFileName, 
			GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, &stdOutSA, 
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
	}
	else {
		printf("Error creating file name!"); 
		return INVALID_HANDLE_VALUE;  
	}
}

void innerMerge(HANDLE hLRec, HANDLE hRRec, HANDLE STDOut)
{
	LARGE_INTEGER lRecNum, rRecNum;  //number of records in left and right
	GetFileSizeEx(hLRec, &lRecNum); 
	GetFileSizeEx(hRRec, &rRecNum); 
	RECORD lRec, rRec;

	int curList1 = lRecNum.QuadPart / 64;	/* int endList1 = mid;*/
	int curList2 = rRecNum.QuadPart / 64;	/* int endList2 = high;*/
	int curDest = 0; 
	int leftIdx = 0;
	int rightIdx = 0;

	while ((leftIdx < curList1) && (rightIdx < curList2))
	{
		lRec = ReadRecord(hLRec, leftIdx);
		rRec = ReadRecord(hRRec, rightIdx); 

		if (strcmp(lRec.key, rRec.key) <= 0)	//src[curList1] <= src[curList2])
		{
			WriteRecord(STDOut, curDest, lRec);	// dest[curDest] = src[curList1];
			curDest++;
			leftIdx++; 
		}
		else
		{
			WriteRecord(STDOut, curDest, rRec); // dest[curDest++] = src[curList2++]; 
			curDest++;
			rightIdx++;
		}
	}
	if (leftIdx >= curList1) //firt list is empty
	{
		while (rightIdx < curList2)	{
			rRec = ReadRecord(hRRec, rightIdx);
			WriteRecord(STDOut, curDest, rRec); //dest[curDest] = src[curList2];
			curDest++; 
			rightIdx++;
		}
	}
	else if (rightIdx >= curList2) //second list is empty
	{
		while (leftIdx < curList1)
		{
			lRec = ReadRecord(hLRec, leftIdx);	//dest[curDest] = src[curList1];
			WriteRecord(STDOut, curDest, lRec);
			curDest++;
			leftIdx++;
		}
	}
}

// this time we are not ahjving a pointer to an array but instead a file
int partition(int start, int end, HANDLE STDOut)
{
	int mid = (start + end) / 2;

	RECORD startRec, endRec, pivot; 
	pivot = ReadRecord(STDOut, mid); //arrParam[mid]	// arrParam[0] = arrParam[mid];
	startRec = ReadRecord(STDOut, start);
	endRec = ReadRecord(STDOut, end); 

	WriteRecord(STDOut, mid, startRec);	// arrParam[mid] = arrParam[start];
	
	

	while (start != end)
	{
		while ((start != end) && strcmp(endRec.key, pivot.key) > 0)	//(arrParam[end] > arrParam[0]))
		{
			end--;
			endRec = ReadRecord(STDOut, end); 
		}
		if (start == end)
		{
			break;
		}
		WriteRecord(STDOut, start, endRec);		//arrParam[start] = arrParam[end];
		start++;
		startRec = ReadRecord(STDOut, start);
		while ((start != end) && strcmp(startRec.key, pivot.key) < 0)// (arrParam[start] < arrParam[0]))
		{
			start++;
			startRec = ReadRecord(STDOut, start); 
		}
		if (start == end)
		{
			break;
		}
		WriteRecord(STDOut, end, startRec);		// arrParam[end] = arrParam[start];
		end--;
		endRec = ReadRecord(STDOut, end);
	}
	WriteRecord(STDOut, start, pivot);		//arrParam[start] = arrParam[0];
	return start;
}

void quickSort_func(int low, int high, HANDLE STDOut)
{
	int pivot = partition(low, high, STDOut); 

	if (low < pivot - 1)  //if we have more than 1 element to the left
	{
		quickSort_func(low, pivot - 1, STDOut); 
	}
	if (high > pivot + 1)  //if there is 1 or more elements to the right
	{
		quickSort_func(pivot + 1, high, STDOut);  
	}
}

typedef struct {
	HANDLE hInFile;
	HANDLE hOutFile;
	STARTUPINFO startUpInfo;
} ProcFileInfo; 

typedef struct {
	TCHAR tempFile[MAX_PATH];
} PROCFILE;

int _tmain(int argc, LPTSTR argv[])
{
	PROCFILE* randFiles = NULL; // needed for an array holding my random files
	
	RECORD data;
	HANDLE STDInput, STDOutput; 
	PROCFILE* InFile; /* Pointer to array of temp file names. */
	PROCFILE* OutFile; /* Pointer to array of temp file names. */
	HANDLE hRInTmp = NULL, hROutTmp = NULL, hLInTmp = NULL, hLOutTmp = NULL;  
	LARGE_INTEGER FileSize;
	DWORD BIn, Bout;
	PROCESS_INFORMATION processInfo;
	// it was mentioned that we didnt need both startU and startUpSearch so i will do the same but still apply for a left and right version
	STARTUPINFO startUp, lStartUp, rStartUp;	// ONLY NEED A LEFT AND A RIGHT! hRInStartUp, hROutStartUp, hLInStartUp, hLOutStartUp; 
	STDInput = GetStdHandle(STD_INPUT_HANDLE);
	STDOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	TCHAR commandLine[200]; 
	HANDLE* hProc;  /* Pointer to an array of proc handles. */ 
	HANDLE* hOutFiles; 
	LARGE_INTEGER spot; 
	HANDLE lProc = NULL;
	HANDLE rProc = NULL; 
	


	int processes; 

	// the following portion obtains how many cores your computer has
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int numCPU = sysinfo.dwNumberOfProcessors;

	GetFileSizeEx(STDInput, &FileSize);

	if (argc == 1) {
		// here we will handle the processors and stuff this means we were given nothnig
		// in the command line but the name of the process
		// get the number of processors and find the next power of 2
		processes = numCPU; 
		if (ceil(log2(processes)) != floor(log2(processes)))  
		{
			// the unsigned int before the function will force the answer to be an unsigned int instead of a floating point
			processes = pow(2, ceil(log2(processes)));	// this will adjust our # of processes to the next power of 2
		}
	}
	else {
		processes = atoi(argv[1]); 
	}

	GetFileSizeEx(STDInput, &FileSize); 

	if (processes == 1)
	{
		
		//void insertionSort(int arr[], int n)
		//{
		//	int i, key, j;
		//	for (i = 1; i < n; i++)
		//	{
		//		key = arr[i];
		//		j = i - 1;

		//		/* Move elements of arr[0..i-1], that are
		//		greater than key, to one position ahead
		//		of their current position */
		//		while (j >= 0 && arr[j] > key)
		//		{
		//			arr[j + 1] = arr[j];
		//			j = j - 1;
		//		}
		//		arr[j + 1] = key;
		//	}
		//}


		LARGE_INTEGER spot;
		int size = (FileSize.QuadPart / 64); 
		int i, j;
		RECORD key, J_Item;

		// following is copying stdin to stdout
		for (int x = 0; x < size; x++)  // Fill the output file (You can't sort the input file!!!)
		{
			ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
			WriteFile(STDOutput, &data, RECSIZE, &Bout, NULL);
		}

		quickSort_func(0, size - 1, STDOutput); 
	}
	else
	{
		GetStartupInfo(&startUp);
		GetStartupInfo(&lStartUp);
		GetStartupInfo(&rStartUp);

		randFiles = malloc(4 * sizeof(PROCFILE));	// an array of my random files

		processes /= 2;

		sprintf(commandLine, "FileInsertionSort %d", processes);

		//if (GetTempFileName(_T("."), _T("In"), 0, InFile[iProc].tempFile) == 0)
		//{
		//	printf("Error creating file name!");
		//	return 1;
		//}

		//if (GetTempFileName(_T("."), _T("Out"), 0, OutFile[iProc].tempFile) == 0)
		//{
		//	printf("Error creating file name!");
		//	return 1;
		//}


		//hInTempFile = /* This handle is inheritable */
		//	CreateFile(InFile[iProc].tempFile,
		//		GENERIC_READ | GENERIC_WRITE,
		//		FILE_SHARE_READ | FILE_SHARE_WRITE, &stdOutSA,
		//		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


		//hOutTempFile = /* This handle is inheritable */
		//	CreateFile(OutFile[iProc].tempFile,
		//		GENERIC_READ | GENERIC_WRITE,
		//		FILE_SHARE_READ | FILE_SHARE_WRITE, &stdOutSA,
		//		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		//hOutFiles[iProc] = hOutTempFile;

		// similar to above but create we are creating 4 files for each level, so here are the lin lout rin and rout for the level
		hLInTmp = CreateTempFile(_T("LIn"), randFiles[0].tempFile);
		if (hLInTmp == INVALID_HANDLE_VALUE) {
			return 1;
		}

		hLOutTmp = CreateTempFile(_T("LOut"), randFiles[1].tempFile);
		if (hLOutTmp == INVALID_HANDLE_VALUE) {
			return 1;
		}

		hRInTmp = CreateTempFile(_T("RIn"), randFiles[2].tempFile);
		if (hRInTmp == INVALID_HANDLE_VALUE) {
			return 1;
		}

		hROutTmp = CreateTempFile(_T("ROut"), randFiles[3].tempFile);
		if (hROutTmp == INVALID_HANDLE_VALUE) {
			return 1;
		}

		// now that we made the files we need to split the data in half and put it into their respective portions
		// we will put the info into the Lin and Rin

		// figuring out the mid point and taking into account odd amount of data
		int mid = ((FileSize.QuadPart) / 64) / 2;
		//spot.QuadPart = 0; 

		//fill in the left in temp file if there is an odd number add it here
		//SetFilePointerEx(STDInput, spot, NULL, FILE_BEGIN); 
		if (((FileSize.QuadPart) / 64) % 2 != 0) {
			for (int i = 0; i <= mid; i++) {  // <= will grab the mid as well since it was an uneven amount of data and store in the left side 
				ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
				WriteFile(hLInTmp, &data, RECSIZE, &Bout, NULL);
			}
		}
		else {
			for (int i = 0; i < mid; i++) { // change to less than because that will grab and equal amount of items
				ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
				WriteFile(hLInTmp, &data, RECSIZE, &Bout, NULL);
			}
		}
		//fill in the right 
		//SetFilePointerEx(STDInput, spot, NULL, FILE_BEGIN);
		for (int i = 0; i < mid; i++) { 
			ReadFile(STDInput, &data, RECSIZE, &BIn, NULL); 
			WriteFile(hRInTmp, &data, RECSIZE, &Bout, NULL);  
		}

		/*	startUpSearch.dwFlags = STARTF_USESTDHANDLES;
			startUpSearch.hStdOutput = hOutTempFile;
			startUpSearch.hStdError = hOutTempFile;
			startUpSearch.hStdInput = hInTempFile;*/

		// again do the same as above but for our four files
		lStartUp.dwFlags = STARTF_USESTDHANDLES;
		lStartUp.hStdOutput = hLOutTmp;
		lStartUp.hStdError = hLOutTmp;
		lStartUp.hStdInput = hLInTmp;

		rStartUp.dwFlags = STARTF_USESTDHANDLES;
		rStartUp.hStdOutput = hROutTmp;
		rStartUp.hStdError = hROutTmp;
		rStartUp.hStdInput = hRInTmp;

		// SET FILE POINTERS
		spot.QuadPart = 0;  // quadpart is a 64-bit integer

		SetFilePointerEx(hLInTmp, spot, NULL, FILE_BEGIN);
		SetFilePointerEx(hRInTmp, spot, NULL, FILE_BEGIN);

		// next we had 
	/*	if (!CreateProcess(NULL, commandLine, NULL, NULL,
			TRUE, 0, NULL, NULL, &startUpSearch, &processInfo))
			printf("ProcCreate failed.");

		hProc[iProc] = processInfo.hProcess;*/
		// we are creating the process that will spawn off tchildren
		if (!CreateProcess(NULL, commandLine, NULL, NULL, 
			TRUE, 0, NULL, NULL, &lStartUp, &processInfo)) 
			printf("ProcCreate failed.");

		lProc = processInfo.hProcess; 

		if (!CreateProcess(NULL, commandLine, NULL, NULL, 
			TRUE, 0, NULL, NULL, &rStartUp, &processInfo))
			printf("ProcCreate failed.");

		rProc = processInfo.hProcess; 

		//CloseHandle(hLInTmp);
		//CloseHandle(hRInTmp);
		// now we are setting in place the wait for single object attribute which will have us wait for processes to wait for their children
		WaitForSingleObject(lProc, INFINITE);  
		WaitForSingleObject(rProc, INFINITE); 

		STDOutput = GetStdHandle(STD_OUTPUT_HANDLE); 

		innerMerge(hLOutTmp, hROutTmp, STDOutput); 

		CloseHandle(hRInTmp); 
		CloseHandle(hROutTmp); 
		CloseHandle(hLInTmp); 
		CloseHandle(hLOutTmp); 

		for (int i = 0; i < 4; i++) { 
			if (!DeleteFile(randFiles[i].tempFile)) { 
				printf("Cannot delete the temp file."); 
				return 1;
			} 
		}
	}
	return 0;
}