#pragma once

#include <string>
#include <map>

class SyncML
{
public:
    static std::wstring ErrorMessage(const std::wstring& returnCodeString);
    static bool IsSuccess(const std::wstring& returnCodeString);
    static void ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse, const std::wstring& errorCode);
    static void SetErrorVerbosity(bool verbosity);
private:
    static void InitializeErrorMessages();
    static bool s_errorVerbosity;
    static std::map<std::wstring, std::wstring> s_errorCodeToMessage;
};