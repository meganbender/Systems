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

#define DATALEN 56
#define KEYLEN 8
typedef struct _RECORD {
	TCHAR key[KEYLEN];
	TCHAR data[DATALEN];
} RECORD;

#define RECSIZE sizeof (RECORD)
int _tmain(int argc, LPTSTR argv[])
{

	SECURITY_ATTRIBUTES stdOutSA = /* SA for inheritable handle. */
	{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

	HANDLE *hProc;  /* Pointer to an array of proc handles. */

	typedef struct { TCHAR tempFile[MAX_PATH]; } PROCFILE;

	TCHAR commandLine[200];
	HANDLE hInTempFile, hOutTempFile;
	RECORD data;
	HANDLE STDInput, STDOutput;
	LARGE_INTEGER FileSize;
	DWORD BIn, Bout;
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startUpSearch, startUp;
	STDInput = GetStdHandle(STD_INPUT_HANDLE);
	STDOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hInMap = NULL;
	HANDLE hOutMap = NULL;
	HANDLE fOut = NULL;

	int processes = atoi(argv[1]);

	GetFileSizeEx(STDInput, &FileSize);

	if (processes == 1)
	{
		HANDLE hInMapChild;
		HANDLE hOutMapChild;
		int front;
		int back;
		int end;
		if (argc == 7)
		{
			 hInMapChild = atoi(argv[2]);
			 hOutMapChild = atoi(argv[3]);



			front = atoi(argv[4]);
			back = atoi(argv[5]);
			end = atoi(argv[6]);
		}
		else
		{
			 hInMapChild = CreateFileMapping(STDInput, &stdOutSA, PAGE_READONLY, 0, 0, NULL);
			 hOutMapChild = CreateFileMapping(STDOutput, &stdOutSA, PAGE_READWRITE, FileSize.HighPart, FileSize.LowPart, NULL);
			 front = 0;
			 back = (FileSize.QuadPart) / 64 - 1;
			 end = (FileSize.QuadPart) / 64 - 1;


		}


		RECORD * pInFile = MapViewOfFile(hInMapChild, FILE_MAP_READ, 0, 0, FileSize.QuadPart);
		DWORD e = GetLastError();
		RECORD * pOutFile = MapViewOfFile(hOutMapChild, FILE_MAP_WRITE, 0, 0, FileSize.QuadPart);
		e = GetLastError();


		for (int x = front; x <= back; x++)
		{
			pOutFile[end] = pInFile[x];
			end--;
		}
		UnmapViewOfFile(pInFile);
		UnmapViewOfFile(pOutFile);
		CloseHandle(hInMapChild);
		CloseHandle(hOutMapChild);

	}
	else
	{


		hProc = malloc(processes * sizeof(HANDLE));


		GetStartupInfo(&startUpSearch);
		GetStartupInfo(&startUp);

		fOut = CreateFile("tempfile.txt",
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &stdOutSA,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		hInMap = CreateFileMapping(STDInput, &stdOutSA, PAGE_READONLY, 0, 0, NULL);
		hOutMap = CreateFileMapping(fOut, &stdOutSA, PAGE_READWRITE, FileSize.HighPart, FileSize.LowPart, NULL);

		int totalitems = (FileSize.QuadPart) / 64;
		int numitems = (FileSize.QuadPart) / 64 / processes;
		int extra_items = (FileSize.QuadPart) / 64 % processes;

		int front = 0;
		int back = numitems - 1;
		if (extra_items)
			back++;
		int end = (FileSize.QuadPart) / 64 - 1;

		for (int iProc = 0; iProc < processes; iProc++) {

			sprintf(commandLine, _T("MMReverse 1 %d %d %d %d %d"), hInMap, hOutMap, front, back, end);

			if (!CreateProcess(NULL, commandLine, NULL, NULL,
				TRUE, 0, NULL, NULL, &startUpSearch, &processInfo))
				printf("ProcCreate failed.");

			hProc[iProc] = processInfo.hProcess;
			end -= (back - front) + 1;
			front = back + 1;
			back += numitems;

			if (iProc + 1 < extra_items)
			{
				back++;
			}

		}

		WaitForMultipleObjects(processes, hProc, TRUE, INFINITE);

		RECORD * pOutFile = MapViewOfFile(hOutMap, FILE_MAP_READ, 0, 0, FileSize.QuadPart);
		for (int x = 0; x < totalitems; x++)
		{
			WriteFile(STDOutput, &pOutFile[x], RECSIZE, &Bout, NULL);
		}
		UnmapViewOfFile(pOutFile);
		CloseHandle(hOutMap);
		CloseHandle(fOut);
		free(hProc);

		if (!DeleteFile("tempfile.txt"))
		{
			int e = GetLastError();
			if (e == ERROR_ACCESS_DENIED)
				printf("Did not delete");
		}
	}




	return 0;
}