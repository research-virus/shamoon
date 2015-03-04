// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "Utils.h"

void _mbstowcs(WCHAR *pOutString, int nMaxSize, char *pInString)
{
	if(pInString == NULL || pOutString == NULL)
		return;
	
	memset(pOutString, 0, sizeof(WCHAR) * nMaxSize);
	
	for(; nMaxSize, *pInString; --nMaxSize)
		*(pOutString++) = *(pInString++);
	
	*pOutString = 0;
}

bool GetHostname(WCHAR *pHostName)
{
	if(pHostName == NULL)
		return false;
	
	WCHAR pTempHostName[256];
	char pHName[50];
	
	pTempHostName[0] = 0;
	memset(pHName, 0, 50);
	
	struct WSAData sWSAData;
	if(WSAStartup(257, &sWSAData))
	{
		WSACleanup();
		return false;
	}

	gethostname(pHName, 50);
	
	struct hostent *hHostByName = gethostbyname(pHName);
	for(INT32 nHost = 0; nHost < 10; nHost++)
	{
		if(hHostByName->h_addr_list[nHost] == NULL)
			break;
		
		struct in_addr sInAddr;
		memcpy(&sInAddr, hHostByName->h_addr_list[nHost], hHostByName->h_length);
		
		char *pINET = inet_ntoa(sInAddr);
		DWORD nINET = strlen(pINET);

		if(nINET < 20 && nINET > 0)
		{
			_mbstowcs(pTempHostName, nINET, pINET);
			break;
		}
	}
	
	WSACleanup();
	
	DWORD nHostLen = wcslen(pTempHostName);
	if(nHostLen == 0)
		return false;

	if(nHostLen >= 256)
		nHostLen = 255;
	
	wmemcpy(pHostName, pTempHostName, 2 * nHostLen);
	pHostName[nHostLen] = 0;
	
	return true;
}

int _wprintf(wchar_t *lpszDest, WCHAR *lpszFormat, ...)
{
	va_list va;
	
	va_start(va, lpszFormat);
	return _vswprintf(lpszDest, lpszFormat, va);
}