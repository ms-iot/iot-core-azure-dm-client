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

    template<class T>
    static bool TryGetNumber(const std::wstring& path, T& value)
    {
        bool success = true;
        try
        {
            value = static_cast<T>(RunGetUInt(path));
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
    static bool TryGetBool(const std::wstring& path, bool& value);

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
