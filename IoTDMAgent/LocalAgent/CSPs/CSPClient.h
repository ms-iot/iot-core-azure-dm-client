#pragma once

#include <string>

class CSPClient
{
public:
    static bool RunSyncML(const std::wstring& sid, const std::wstring& inputSyncML, std::wstring& outputSyncML);
    static bool RunAdd(const std::wstring& sid, const std::wstring& path, std::wstring& value);
    static bool RunGet(const std::wstring& sid, const std::wstring& path, std::wstring& value);
    static bool RunGet(const std::wstring& sid, const std::wstring& path, unsigned int& value);
    static bool RunSet(const std::wstring& sid, const std::wstring& path, const std::wstring& value);
    static bool RunSet(const std::wstring& sid, const std::wstring& path, unsigned int value);

protected:
    static bool RunAdd(const std::wstring& path, std::wstring& value);
    static bool RunGet(const std::wstring& path, std::wstring& value);
    static bool RunGet(const std::wstring& path, unsigned int& value);
    static bool RunSet(const std::wstring& path, const std::wstring& value);
    static bool RunSet(const std::wstring& path, unsigned int value);

    static bool ParseGetResults(const std::wstring& requestSyncML, const std::wstring& resultSyncML, std::wstring& results);
    static bool ParseSetResults(const std::wstring& requestSyncML, const std::wstring& resultSyncML);
    static bool ParseAddResults(const std::wstring& requestSyncML, const std::wstring& resultSyncML);
};