#pragma once

#define WIN32_LEAN_AND_MEAN 

#pragma warning(disable : 4996)

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <WinSock2.h>

char topleft[16][16];
char topright[16][16];
char bottomleft[16][16];
char bottomright[16][16];

void filltopleft()
{
	char letter = 0;
	for (int r = 0; r < 16; r++)
	{
		for (int c = 0; c < 16; c++)
		{
			topleft[r][c] = letter;
			letter++;
		}
	}

}
void filltopright()
{
	HANDLE fileIN;
	DWORD Bin;
	fileIN = CreateFile("table1.txt",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,

		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	for (int r = 0; r < 16; r++)
	{
		for (int c = 0; c < 16; c++)
		{
			ReadFile(fileIN, &(topright[r][c]), 1, &Bin, NULL);
		}
	}


}
void fillbottomleft()
{
	HANDLE fileIN;
	DWORD Bin;
	fileIN = CreateFile("table2.txt",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,

		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	for (int r = 0; r < 16; r++)
	{
		for (int c = 0; c < 16; c++)
		{
			ReadFile(fileIN, &(bottomleft[r][c]), 1, &Bin, NULL);
		}
	}

}
void fillbottomright()
{
	char letter = 0;
	for (int r = 0; r < 16; r++)
	{
		for (int c = 0; c < 16; c++)
		{
			bottomright[r][c] = letter;
			letter++;
		}
	}
}

void Encrypt(char data[2])
{
	char first = data[0];
	char second = data[1];
	int TRow, TCol;
	int BRow, BCol;
	Sleep(1);
	for (int r = 0; r < 16; r++)
	{
		for (int c = 0; c < 16; c++)
		{
			if (topleft[r][c] == first)
			{
				TRow = r;
				TCol = c;
			}
			if (bottomright[r][c] == second)
			{
				BRow = r;
				BCol = c;
			}
		}
	}
	data[0] = topright[TRow][BCol];
	data[1] = bottomleft[BRow][TCol];


}

void Decrypt(char data[3])
{
	Sleep(1);
	char first = data[0];
	char second = data[1];
	int TRow, TCol;
	int BRow, BCol;

	for (int r = 0; r < 16; r++)
	{
		for (int c = 0; c < 16; c++)
		{
			if (topright[r][c] == first)
			{
				TRow = r;
				TCol = c;
			}
			if (bottomleft[r][c] == second)
			{
				BRow = r;
				BCol = c;
			}
		}
	}
	data[0] = topleft[TRow][BCol];
	data[1] = bottomright[BRow][TCol];
}

void InitializeEncoderDecoder()
{
	filltopleft();
	filltopright();
	fillbottomleft();
	fillbottomright();
}