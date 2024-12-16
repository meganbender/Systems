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

//AGAIN THIS IS THE ORIGINAL USED FOR LAST ASSIGNMENT WHEN DEALING WITH SORTING AND MERGING INFO FROM FILES
// WE ARE NOW DOING IT WITH MEMORY MAPS
//// CHANGE THIS BACK TO USING ARRAYS BECUASE WE ARE USING MAPVIEW
//void innerMerge(HANDLE hLRec, HANDLE hRRec, HANDLE STDOut)
//{
//	LARGE_INTEGER lRecNum, rRecNum;  //number of records in left and right
//	GetFileSizeEx(hLRec, &lRecNum); 
//	GetFileSizeEx(hRRec, &rRecNum); 
//	RECORD lRec, rRec;
//
//	int curList1 = lRecNum.QuadPart / 64;	/* int endList1 = mid;*/
//	int curList2 = rRecNum.QuadPart / 64;	/* int endList2 = high;*/
//	int curDest = 0; 
//	int leftIdx = 0;
//	int rightIdx = 0;
//
//	while ((leftIdx < curList1) && (rightIdx < curList2))
//	{
//		lRec = ReadRecord(hLRec, leftIdx);
//		rRec = ReadRecord(hRRec, rightIdx); 
//
//		if (strcmp(lRec.key, rRec.key) <= 0)	//src[curList1] <= src[curList2])
//		{
//			WriteRecord(STDOut, curDest, lRec);	// dest[curDest] = src[curList1];
//			curDest++;
//			leftIdx++; 
//		}
//		else
//		{
//			WriteRecord(STDOut, curDest, rRec); // dest[curDest++] = src[curList2++]; 
//			curDest++;
//			rightIdx++;
//		}
//	}
//	if (leftIdx >= curList1) //firt list is empty
//	{
//		while (rightIdx < curList2)
//		{
//			rRec = ReadRecord(hRRec, rightIdx);
//			WriteRecord(STDOut, curDest, rRec); //dest[curDest] = src[curList2];
//			curDest++; 
//			rightIdx++;
//		}
//	}
//	else if (rightIdx >= curList2) //second list is empty
//	{
//		while (leftIdx < curList1)
//		{
//			lRec = ReadRecord(hLRec, leftIdx);	//dest[curDest] = src[curList1];
//			WriteRecord(STDOut, curDest, lRec);
//			curDest++;
//			leftIdx++;
//		}
//	}
//}
///// NEW ONE /////
void innerMerge(RECORD src[], RECORD dest[], int low, int mid, int high)
{
	int curList1 = low; int endList1 = mid;
	int curList2 = mid + 1; int endList2 = high;
	int curDest = low;

	while ((curList1 <= endList1) && (curList2 <= endList2))
	{
		if (strcmp(src[curList1].key, src[curList2].key) <= 0)
		{
			dest[curDest] = src[curList1];
			curDest++;
			curList1++;
		}

		else
		{
			dest[curDest] = src[curList2];
			curDest++;
			curList2++; 
		}
	}
	if (curList1 >= endList1) //firt list is empty
	{
		while (curList2 <= endList2)
		{
			dest[curDest] = src[curList2];
			curDest++; curList2++;
		}
	}
	else if (curList2 >= endList2) //second list is empty
	{
		while (curList1 <= endList1)
		{
			dest[curDest] = src[curList1];
			curDest++; curList1++;
		}
	}
}

//THIS WAS  THE PARTITION USED WHEN DEALING WITH CHILD PROCESSES AND SORTING INFO IN FILES
// NOW WE CAN GO BACK TO USING START END AND SUCH JUST LIKE WE WERE USING AN ARRAY, BECAUSE THE
//// MAP VIEWS WORK JUST LIKE ARRAYS
//int partition(int start, int end, HANDLE STDOut)
//{
//	int mid = (start + end) / 2;
//
//	RECORD startRec, endRec, pivot; 
//	pivot = ReadRecord(STDOut, mid); //arrParam[mid]	// arrParam[0] = arrParam[mid];
//	startRec = ReadRecord(STDOut, start);
//	endRec = ReadRecord(STDOut, end); 
//
//	WriteRecord(STDOut, mid, startRec);	// arrParam[mid] = arrParam[start];
//	
//	
//
//	while (start != end)
//	{
//		while ((start != end) && strcmp(endRec.key, pivot.key) > 0)	//(arrParam[end] > arrParam[0]))
//		{
//			end--;
//			endRec = ReadRecord(STDOut, end); 
//		}
//		if (start == end)
//		{
//			break;
//		}
//		WriteRecord(STDOut, start, endRec);		//arrParam[start] = arrParam[end];
//		start++;
//		startRec = ReadRecord(STDOut, start);
//		while ((start != end) && strcmp(startRec.key, pivot.key) < 0)// (arrParam[start] < arrParam[0]))
//		{
//			start++;
//			startRec = ReadRecord(STDOut, start); 
//		}
//		if (start == end)
//		{
//			break;
//		}
//		WriteRecord(STDOut, end, startRec);		// arrParam[end] = arrParam[start];
//		end--;
//		endRec = ReadRecord(STDOut, end);
//	}
//	WriteRecord(STDOut, start, pivot);		//arrParam[start] = arrParam[0];
//	return start;
//}

// NEW VERSION
int partition(int start, int end, RECORD outfile[])  //this could be shorter by possibly using boolean operators
{
	int mid = (start + end) / 2;

	RECORD pivot = outfile[mid];  
	outfile[mid] = outfile[start]; 

	while (start != end)
	{
		while ((start != end) && (strcmp(outfile[end].key, pivot.key) > 0))
		{
			//outfile[start] = outfile[end];	
			end--;
		}
		if (start == end)
		{
			break;
		}
		outfile[start] = outfile[end];
		start++;
		while ((start != end) && (strcmp(outfile[start].key, pivot.key) < 0))
		{
			//outfile[end] = outfile[start]; 
			start++;
		}
		if (start == end)
		{
			break;
		}
		outfile[end] = outfile[start];
		end--;
	}
	outfile[start] = pivot;
	return start;
}

//fix
void quickSort_func(int low, int high, RECORD outFile[])
{
	int pivot = partition(low, high, outFile);

	if (low < pivot - 1)  //if we have more than 1 element to the left
	{
		quickSort_func(low, pivot - 1, outFile);
	}
	if (high > pivot + 1)  //if there is 1 or more elements to the right
	{
		quickSort_func(pivot + 1, high, outFile);
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
	SECURITY_ATTRIBUTES stdOutSA = /* SA for inheritable handle. */ 
	{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE }; 

	//PROCFILE* randFiles = NULL; // needed for an array holding my random files
	
	//RECORD data;
	HANDLE STDInput, STDOutput;
	LARGE_INTEGER FileSize;
	//DWORD BIn, Bout;
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startUp, startUpSearch;// , lStartUp, rStartUp;	// ONLY NEED A LEFT AND A RIGHT! hRInStartUp, hROutStartUp, hLInStartUp, hLOutStartUp; 
	STDInput = GetStdHandle(STD_INPUT_HANDLE);
	STDOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	TCHAR commandLine[200]; 
	HANDLE hInMap = NULL; 
	HANDLE hOutMap = NULL; 
	HANDLE tmpFile = NULL; 
	HANDLE* hProc = NULL;
	HANDLE mm_src = NULL;
	HANDLE mm_dest = NULL;
	HANDLE hInFile = NULL;

	//for assignment 4
	//HANDLE hInMapChild; 
	//HANDLE hOutMapChild; 
	//int front; 
	int end; 
	//int totalitems = NULL;
	//int numitems = NULL;
	//int extra_items = NULL;

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

	if (processes == 1) {
		HANDLE hDestMap;
		HANDLE hSrcMap;
		int low, high; 
		

		if (argc == 2) {
			//create memory map to standard output mm_stdout
			hSrcMap = CreateFileMapping(STDInput, &stdOutSA, PAGE_READONLY, 0, 0, NULL); 
			hDestMap = CreateFileMapping(STDOutput, &stdOutSA, PAGE_READWRITE, FileSize.HighPart, FileSize.LowPart, NULL); 

			//set low to zero and set high to filesize64
			low = 0;
			high = (FileSize.QuadPart) / 64;
			end = (FileSize.QuadPart) / 64;

			//create mapview of each   
			RECORD* pSrcFile = MapViewOfFile(hSrcMap, FILE_MAP_READ, 0, 0, FileSize.QuadPart);   
			RECORD* pDestFile = MapViewOfFile(hDestMap, FILE_MAP_WRITE, 0, 0, FileSize.QuadPart);   

			//copy all of the data from standard input to mm_stdout
			for (int x = low; x <= high; x++) 
			{
				pDestFile[x] = pSrcFile[x]; 
				//end--; 
			}

			//do quicksort on mm_stdout from positions low to high (create mapwiew)
			quickSort_func(0, high - 1, pDestFile);

			//close everything
			UnmapViewOfFile(pSrcFile); 
			UnmapViewOfFile(pDestFile); 
			CloseHandle(hSrcMap); 
			CloseHandle(hDestMap); 
		}
		else {
			//mm_dest = argv[2]
			hDestMap = atoi(argv[2]); //3

			//low = argv[4]
			low = atoi(argv[4]); 

			//high = argv[5]
			high = atoi(argv[5]); 

			//create mapview 
			//RECORD* pSrcFile = MapViewOfFile(hSrcMap, FILE_MAP_READ, 0, 0, FileSize.QuadPart); 
			RECORD* pDestFile = MapViewOfFile(hDestMap, FILE_MAP_WRITE, 0, 0, FileSize.QuadPart); 

			//do quicksort on mm_dest from positions low to high (create a view of the map)
			quickSort_func(0, high, pDestFile); //high-1
		} 
	}
	else {
		hProc = malloc(processes * sizeof(HANDLE));
		GetStartupInfo(&startUpSearch); 
		GetStartupInfo(&startUp);
		int low, mid, high;

		if (argc == 2) {
			// create temp file
			tmpFile = CreateFile("tempfile.txt", 
				GENERIC_READ | GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &stdOutSA, 
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 

			// create memory map to temp file mm_source
			mm_src = CreateFileMapping(tmpFile, &stdOutSA, PAGE_READWRITE, FileSize.HighPart, FileSize.LowPart, NULL); 

			// create memory map to standard output mm_dest 
			mm_dest = CreateFileMapping(STDOutput, &stdOutSA, PAGE_READWRITE, FileSize.HighPart, FileSize.LowPart, NULL); 

			// now make mapviews to act like arrays
			RECORD* pMM_src = MapViewOfFile(mm_src, FILE_MAP_WRITE, 0, 0, FileSize.QuadPart); 
			RECORD* pMM_dest = MapViewOfFile(mm_dest, FILE_MAP_WRITE, 0, 0, FileSize.QuadPart); 

			// copy all data from std in to both memory maps make sure that the one value is set to 0 to begin
			// mapping at the beginning
				// make a map and mapview of std in in order to map it to the src and dest
			hInFile = CreateFileMapping(STDInput, &stdOutSA, PAGE_READONLY, 0, 0, NULL);  
			RECORD* pInFile = MapViewOfFile(hInFile, FILE_MAP_READ, 0, 0, FileSize.QuadPart); 

			processes /= 2;
			int high = (FileSize.QuadPart) / 64 - 1;

			for (int i = 0; i <= high; i++)
			{
				pMM_src[i] = pInFile[i];
				pMM_dest[i] = pInFile[i];
			}

			// set low to zero
			low = 0; 
			mid = (low + high) / 2;
			int low2 = low;
			int mid2 = mid;
			// maybe handle this with some type of loop
				//create command line and pass the process number, both memory maps and the low and high indexes that the child needs
				// this will have about 7 (i think its 6) arguments
			sprintf(commandLine, _T("FileInsertionSort %d %d %d %d %d"), processes, mm_src, mm_dest, low2, mid2); 

				//spawn process for the left
			if (!CreateProcess(NULL, commandLine, NULL, NULL,  
				TRUE, 0, NULL, NULL, &startUpSearch, &processInfo))
				printf("ProcCreate failed.");

			hProc[0] = processInfo.hProcess;

				//spawn process for the right
			low2 = mid2;  
			mid2 = high; 

			sprintf(commandLine, _T("FileInsertionSort %d %d %d %d %d"), processes, mm_src, mm_dest, low2, mid2);

			if (!CreateProcess(NULL, commandLine, NULL, NULL, 
				TRUE, 0, NULL, NULL, &startUpSearch, &processInfo)) 
				printf("ProcCreate failed.");

			hProc[1] = processInfo.hProcess;

			//wait for both children to finish
			WaitForSingleObject(hProc[0], INFINITE); 
			WaitForSingleObject(hProc[1], INFINITE); 

			//merge data from mm_source to mm_dest at this point both sorted arrays are in mm_src. The first array is at indexes
			// low to mid and the second is at indexes mid+1 to high (to do this create a view t each map)
			innerMerge(pMM_src, pMM_dest, low, mid, high); 

			//unmap and close handles
			UnmapViewOfFile(pMM_src);
			UnmapViewOfFile(pMM_dest);
			CloseHandle(mm_src);
			CloseHandle(mm_dest);
			CloseHandle(hInFile);

			//delete temp file
			if (!DeleteFile("tempfile.txt"))
			{
				printf("Error deleting temp file!");
			}
		}
		else {
			int low2, mid2;
			processes /= 2; 
			

			//mm_src = argv[2]
			mm_src = atoi(argv[3]); 

			//mm_dest = argv[3]
			mm_dest = atoi(argv[2]); 

			//low = argv[4]
			low = atoi(argv[4]); 

			//high = argv[5]
			high = atoi(argv[5]); 

			//mid = (low+high)/2
			mid = (low + high) / 2;

			//always create those mapviewsssss
			RECORD* pMM_src = MapViewOfFile(mm_src, FILE_MAP_WRITE, 0, 0, FileSize.QuadPart); 
			RECORD* pMM_dest = MapViewOfFile(mm_dest, FILE_MAP_WRITE, 0, 0, FileSize.QuadPart);

			//create command line; pass the process number, mm_dest, mm_src, low, mid; pass this will be for the LEFT CHILD
			low2 = low; 
			mid2 = mid; 
			sprintf(commandLine, _T("FileInsertionSort %d %d %d %d %d"), processes, mm_dest, mm_src, low2, mid2);	 

			//spawn process for left child
			if (!CreateProcess(NULL, commandLine, NULL, NULL,	
				TRUE, 0, NULL, NULL, &startUpSearch, &processInfo))	
				printf("ProcCreate failed.");

			hProc[0] = processInfo.hProcess;	

			//create command line; pass the process number, mm_dest, mm_src, low, mid; pass this will be for the RIGHT CHILD
			low2 = mid;
			mid2 = high;
			sprintf(commandLine, _T("FileInsertionSort %d %d %d %d %d"), processes, mm_dest, mm_src, low2, mid2);	

			//spawn process for right child
			if (!CreateProcess(NULL, commandLine, NULL, NULL,
				TRUE, 0, NULL, NULL, &startUpSearch, &processInfo))
				printf("ProcCreate failed.");	

			hProc[1] = processInfo.hProcess;

			//wait for each child to finish	
			WaitForSingleObject(hProc[0], INFINITE);	
			WaitForSingleObject(hProc[1], INFINITE);	

			//merge data from mm_src to mm_des; arrays should be sorted in mm_src; 
			// first array is at indexes low to mid and the second is at indexes mid+1 to high (create mapview)
			innerMerge(pMM_dest, pMM_src, low, mid, high);	

			//close handles and mapviews
			UnmapViewOfFile(pMM_src);	
			UnmapViewOfFile(pMM_dest);	
			CloseHandle(mm_src);	
			CloseHandle(mm_dest);	
		}
	}

	return 0;
}