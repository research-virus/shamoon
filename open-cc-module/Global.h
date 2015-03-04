// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __C2_GLOBAL__
#define __C2_GLOBAL__

#include <Windows.h>
#include <WinInet.h>

#define __TEST_ENVIRONMENT__ // For debug purposes

extern WCHAR g_szHostname[256];
extern WCHAR g_szWinDirW[100];
extern CHAR g_szWinDirA[100];

#endif