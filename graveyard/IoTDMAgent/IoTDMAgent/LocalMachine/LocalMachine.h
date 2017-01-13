#pragma once

#include <string>
#include <vector>

class LocalMachine
{
public:
    static std::wstring GetOSVersionString();
    static void GetMemoryInfoMB(unsigned int& totalMB, unsigned int& availableMB);
    static void GetStorageInfoMB(unsigned int& totalMB, unsigned int& availableMB);
};