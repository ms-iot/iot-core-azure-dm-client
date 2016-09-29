#pragma once

#include <string>
#include <map>

class MdmProvision
{
public:
    static bool RunSyncML(const std::wstring& sid, const std::wstring& inputSyncML, std::wstring& outputSyncML);
    static bool RunAdd(const std::wstring& sid, const std::wstring& path, std::wstring& value);
    static bool RunGet(const std::wstring& sid, const std::wstring& path, std::wstring& value);
    static bool RunGet(const std::wstring& sid, const std::wstring& path, unsigned int& value);
    static bool RunSet(const std::wstring& sid, const std::wstring& path, const std::wstring& value);
    static bool RunSet(const std::wstring& sid, const std::wstring& path, unsigned int value);

    static bool RunAdd(const std::wstring& path, std::wstring& value);
    static bool RunGet(const std::wstring& path, std::wstring& value);
    static bool RunGet(const std::wstring& path, unsigned int& value);
    static bool RunSet(const std::wstring& path, const std::wstring& value);
    static bool RunSet(const std::wstring& path, unsigned int value);

    static bool IsSuccessful(const std::wstring& requestSyncML, const std::wstring& resultSyncML);

    static void SetErrorVerbosity(bool verbosity);
    static std::wstring GetErrorMessage(const std::wstring& returnCodeString);
    static void ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse, const std::wstring& errorCode);
    static void ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse);

private:
    static void InitializeErrorMessages();
    static bool s_errorVerbosity;
    static std::map<std::wstring, std::wstring> s_errorCodeToMessage;
};
