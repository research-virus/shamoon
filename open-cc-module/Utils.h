// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __C2_UTILS__
#define __C2_UTILS__

#include <memory>
#include <Windows.h>

void _mbstowcs(WCHAR *pOutString, int nMaxSize, char *pInString);
bool GetHostname(WCHAR *pHostName);
int _wprintf(wchar_t *pDest, WCHAR *pFormat, ...);

#endif