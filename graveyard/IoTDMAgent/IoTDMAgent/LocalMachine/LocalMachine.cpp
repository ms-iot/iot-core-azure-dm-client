/*
Copyright 2017 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "stdafx.h"
#include <windows.h>
#include "LocalMachine.h"
#include "CSPs\MdmProvision.h"
#include "CSPs\RebootCSP.h"
#include "CSPs\RemoteWipeCSP.h"
#include "CSPs\UpdateCSP.h"
#include "CSPs\PolicyCSP.h"

using namespace std;

wstring LocalMachine::GetOSVersionString()
{
    return Utils::GetOSVersionString();
}

void LocalMachine::GetMemoryInfoMB(unsigned int& totalMB, unsigned int& availableMB)
{
    TRACE("LocalMachine::GetMemoryInfoMB()");

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex))
    {
        throw DMException("Error: GlobalMemoryStatusEx() failed.");
    }

    totalMB = static_cast<unsigned int>(statex.ullTotalPhys) / (1024 * 1024);
    availableMB = static_cast<unsigned int>(statex.ullAvailPhys) / (1024 * 1024);
}

void LocalMachine::GetStorageInfoMB(unsigned int& totalMB, unsigned int& availableMB)
{
    TRACE("LocalMachine::GetStorageInfoMB()");

    ULARGE_INTEGER totalBytes = { 0 };
    ULARGE_INTEGER availableBytes = { 0 };

    if (!GetDiskFreeSpaceEx(NULL, NULL, &totalBytes, &availableBytes))
    {
        throw DMException("Error: GetDiskFreeSpaceEx() failed.");
    }

    totalMB = static_cast<unsigned int>(totalBytes.QuadPart / (1024 * 1024));
    availableMB = static_cast<unsigned int>(availableBytes.QuadPart / (1024 * 1024));
}
