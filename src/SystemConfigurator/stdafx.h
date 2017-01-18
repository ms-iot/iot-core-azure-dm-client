// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef AZURE_BLOB_SDK_FOR_ARM
// avoid conflicts with std::numeric_limits max
#define NOMINMAX
#endif // AZURE_BLOB_SDK_FOR_ARM

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <stdint.h>
#include <assert.h>
#include <ppltasks.h>
#include <windows.h>
