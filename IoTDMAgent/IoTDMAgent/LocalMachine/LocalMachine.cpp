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
