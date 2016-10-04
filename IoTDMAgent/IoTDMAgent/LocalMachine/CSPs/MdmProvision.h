#pragma once

#include <string>
#include <map>

class MdmProvision
{
public:
    static void SetErrorVerbosity(bool verbosity) noexcept;

    static void RunSyncML(const std::wstring& sid, const std::wstring& inputSyncML, std::wstring& outputSyncML);
    static void RunAdd(const std::wstring& sid, const std::wstring& path, const std::wstring& value);
    static std::wstring RunGetString(const std::wstring& sid, const std::wstring& path);
    static unsigned int RunGetUInt(const std::wstring& sid, const std::wstring& path);
    static void RunSet(const std::wstring& sid, const std::wstring& path, const std::wstring& value);
    static void RunSet(const std::wstring& sid, const std::wstring& path, unsigned int value);

    static void RunAdd(const std::wstring& path, const std::wstring& value);
    static std::wstring RunGetString(const std::wstring& path);
    static unsigned int RunGetUInt(const std::wstring& path);
    static void RunSet(const std::wstring& path, const std::wstring& value);
    static void RunSet(const std::wstring& path, unsigned int value);

    static std::wstring GetErrorMessage(const std::wstring& returnCodeString);
    static void ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse, int errorCode);
    static void ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse);

private:
    static bool s_errorVerbosity;
};
