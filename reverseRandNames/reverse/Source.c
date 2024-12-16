#define WIN32_LEAN_AND_MEAN 

#define NOATOM
#define NOCLIPBOARD
#define NOCOMM
#define NOCTLMGR
#define NOCOLOR
#define NODEFERWINDOWPOS
#define NODESKTOP
#define NODRAWTEXT
#define NOEXTAPI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOTIME
#define NOIMM
#define NOKANJI
#define NOKERNEL
#define NOKEYSTATES
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMSG
#define NONCMESSAGES
#define NOPROFILER
#define NORASTEROPS
#define NORESOURCE
#define NOSCROLL
//#define NOSERVICE		/* Windows NT Services */
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOSYSPARAMS
#define NOTEXTMETRIC
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINDOWSTATION
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define OEMRESOURCE
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

	HANDLE* hProc;  /* Pointer to an array of proc handles. */
	HANDLE* hOutFiles;
	typedef struct { TCHAR tempFile[MAX_PATH]; } PROCFILE;
	PROCFILE* InFile; /* Pointer to array of temp file names. */
	PROCFILE* OutFile; /* Pointer to array of temp file names. */
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

	int processes = atoi(argv[1]);

	GetFileSizeEx(STDInput, &FileSize);

	if (processes == 1)
	{

		for (int x = FileSize.QuadPart - 64; x >= 0; x -= 64)
		{
			LARGE_INTEGER spot;
			spot.QuadPart = x;
			SetFilePointerEx(STDInput, spot, NULL, FILE_BEGIN);
			ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
			WriteFile(STDOutput, &data, RECSIZE, &Bout, NULL);
		}

	}
	else
	{
		InFile = malloc(processes * sizeof(PROCFILE));
		OutFile = malloc(processes * sizeof(PROCFILE));
		hProc = malloc(processes * sizeof(HANDLE));
		hOutFiles = malloc(processes * sizeof(HANDLE));

		GetStartupInfo(&startUpSearch);
		GetStartupInfo(&startUp);

		sprintf(commandLine, _T("reverse 1"));

		for (int iProc = 0; iProc < processes; iProc++) {



			if (GetTempFileName(_T("."), _T("In"), 0, InFile[iProc].tempFile) == 0)
			{
				printf("Error creating file name!");
				return 1;
			}

			if (GetTempFileName(_T("."), _T("Out"), 0, OutFile[iProc].tempFile) == 0)
			{
				printf("Error creating file name!");
				return 1;
			}


			hInTempFile = /* This handle is inheritable */
				CreateFile(InFile[iProc].tempFile,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE, &stdOutSA,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


			hOutTempFile = /* This handle is inheritable */
				CreateFile(OutFile[iProc].tempFile,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE, &stdOutSA,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			hOutFiles[iProc] = hOutTempFile;

			int numitems = (FileSize.QuadPart) / 64 / processes;

			for (int x = 0; x < numitems; x++)
			{
				ReadFile(STDInput, &data, RECSIZE, &BIn, NULL);
				WriteFile(hInTempFile, &data, RECSIZE, &Bout, NULL);
			}

			startUpSearch.dwFlags = STARTF_USESTDHANDLES;
			startUpSearch.hStdOutput = hOutTempFile;
			startUpSearch.hStdError = hOutTempFile;
			startUpSearch.hStdInput = hInTempFile;

			if (!CreateProcess(NULL, commandLine, NULL, NULL,
				TRUE, 0, NULL, NULL, &startUpSearch, &processInfo))
				printf("ProcCreate failed.");

			hProc[iProc] = processInfo.hProcess;


			CloseHandle(hInTempFile);

		}

		/*	for (int iProc = 0; iProc < processes; iProc += MAXIMUM_WAIT_OBJECTS)
				WaitForMultipleObjects(min(MAXIMUM_WAIT_OBJECTS, processes - iProc),
					&hProc[iProc], TRUE, INFINITE);*/

					/*for (int iProc = 0; iProc < processes; iProc++) {
						WaitForSingleObject(hProc[iProc], INFINITE);
					}*/

		WaitForMultipleObjects(processes, hProc, TRUE, INFINITE);

		for (int iProc = processes - 1; iProc >= 0; iProc--) {
			LARGE_INTEGER spot;
			spot.QuadPart = 0;
			SetFilePointerEx(hOutFiles[iProc], spot, NULL, FILE_BEGIN);
			do
			{
				ReadFile(hOutFiles[iProc], &data, RECSIZE, &BIn, NULL);
				if (BIn > 0)
					WriteFile(STDOutput, &data, RECSIZE, &Bout, NULL);

			} while (BIn > 0);
			CloseHandle(hOutFiles[iProc]);

			if (!DeleteFile(InFile[iProc].tempFile))
			{
				printf("Cannot delete temp file.");
				return 1;
			}
			if (!DeleteFile(OutFile[iProc].tempFile))
			{
				printf("Cannot delete temp file.");
				return 1;
			}
		}
	}

	//system("pause");
	return 0;
}