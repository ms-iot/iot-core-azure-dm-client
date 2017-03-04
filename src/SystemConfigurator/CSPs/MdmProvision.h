#pragma once

#include "Utils.h"
#include <string>

class MdmProvision
{
public:
    static void SetErrorVerbosity(bool verbosity) noexcept;

    // With sid
    static void RunSyncML(const std::wstring& sid, const std::wstring& inputSyncML, std::wstring& outputSyncML);

    static void RunAdd(const std::wstring& sid, const std::wstring& path, const std::wstring& value);
    static void RunAddData(const std::wstring& sid, const std::wstring& path, const std::wstring& value, const std::wstring& type = L"chr");
    static void RunAddDataBase64(const std::wstring& sid, const std::wstring& path, const std::wstring& value);

    static void RunDelete(const std::wstring& sid, const std::wstring& path);

    static void RunGetStructData(const std::wstring& path, Utils::ELEMENT_HANDLER handler);
    static std::wstring RunGetString(const std::wstring& sid, const std::wstring& path);
    static std::wstring RunGetBase64(const std::wstring& sid, const std::wstring& path);
    static unsigned int RunGetUInt(const std::wstring& sid, const std::wstring& path);
    static bool RunGetBool(const std::wstring& sid, const std::wstring& path);

    static void RunSet(const std::wstring& sid, const std::wstring& path, const std::wstring& value);
    static void RunSet(const std::wstring& sid, const std::wstring& path, int value);
    static void RunSet(const std::wstring& sid, const std::wstring& path, bool value);
    static void RunSetBase64(const std::wstring& sid, const std::wstring& path, const std::wstring& value);

    static void RunExec(const std::wstring& sid, const std::wstring& path);

    // Without sid
    static void RunAdd(const std::wstring& path, const std::wstring& value);
    static void RunAddData(const std::wstring& path, const std::wstring& value);
    static void RunAddDataBase64(const std::wstring& path, const std::wstring& value);
    static void RunAddData(const std::wstring& path, int value);
    static void RunAddData(const std::wstring& path, bool value);

    static void RunDelete(const std::wstring& path);

    static std::wstring RunGetString(const std::wstring& path);
    static bool TryGetString(const std::wstring& path, std::wstring& value);
    static std::wstring RunGetBase64(const std::wstring& path);
    static unsigned int RunGetUInt(const std::wstring& path);

    template<class T>
    static bool TryGetNumber(const std::wstring& path, std::wstring& value)
    {
        bool success = true;
        try
        {
            T number = static_cast<T>(RunGetUInt(path));
            value = Utils::MultibyteToWide(to_string(number).c_str());
        }
        catch (DMException& e)
        {
            success = false;
            TRACEP(L"Error: GetString() - path     : ", path.c_str());
            TRACEP("Error: GetString() - exception: ", e.what());
        }
        return success;
    }

    static bool RunGetBool(const std::wstring& path);

    static void RunSet(const std::wstring& path, const std::wstring& value);
    static void RunSet(const std::wstring& path, int value);
    static void RunSet(const std::wstring& path, bool value);
    static void RunSetBase64(const std::wstring& path, const std::wstring& value);

    static void RunExec(const std::wstring& path);

    // Helpers
    static std::wstring GetErrorMessage(const std::wstring& returnCodeString);
    static void ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse, int errorCode);
    static void ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse);

private:
    static bool s_errorVerbosity;
};
