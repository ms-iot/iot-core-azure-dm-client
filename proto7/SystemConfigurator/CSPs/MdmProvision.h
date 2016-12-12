#pragma once

#include <string>
#include <map>
#include <vector>

class MdmProvision
{
public:
    static void SetErrorVerbosity(bool verbosity) noexcept;

    // With sid
    static void RunSyncML(const std::wstring& sid, const std::wstring& inputSyncML, std::wstring& outputSyncML);

    static void RunAdd(const std::wstring& sid, const std::wstring& path, const std::wstring& value);

    static std::map<std::wstring, std::map<std::wstring, std::wstring>> RunGetStructData(const std::wstring& path);
    static std::wstring RunGetString(const std::wstring& sid, const std::wstring& path);
    static unsigned int RunGetUInt(const std::wstring& sid, const std::wstring& path);

    static void RunSet(const std::wstring& sid, const std::wstring& path, const std::wstring& value);
    static void RunSet(const std::wstring& sid, const std::wstring& path, unsigned int value);

    static void RunExec(const std::wstring& sid, const std::wstring& path);

    // Without sid
    static void RunAdd(const std::wstring& path, const std::wstring& value);

    static std::wstring RunGetString(const std::wstring& path);
    static unsigned int RunGetUInt(const std::wstring& path);

    static void RunSet(const std::wstring& path, const std::wstring& value);
    static void RunSet(const std::wstring& path, unsigned int value);

    static void RunExec(const std::wstring& path);

    // Helpers
    static std::wstring GetErrorMessage(const std::wstring& returnCodeString);
    static void ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse, int errorCode);
    static void ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse);

private:
    static bool s_errorVerbosity;
};
