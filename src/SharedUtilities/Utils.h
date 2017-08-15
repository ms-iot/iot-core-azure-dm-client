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

#include <stdio.h>
#include <string>
#include <vector>
#include <deque>
#include <thread>
#include <time.h>
#include <sstream>
#include <windows.h>
#include "StringUtils.h"
#include "AutoCloseHandle.h"

#define IoTDMRegistryRoot L"Software\\Microsoft\\IoTDM"
#define IoTDMRegistryLastRebootCmd L"LastRebootCmd"
#define IoTDMRegistryWindowsUpdateRebootAllowed L"WindowsUpdateRebootAllowed"
#define IoTDMRegistryWindowsUpdatePolicySectionReporting L"WindowsUpdatePolicySectionReporting"
#define IoTDMRegistryEventTracing IoTDMRegistryRoot L"\\EventTracingConfiguration";
#define IoTDMRegistryEventTracingLogFileFolder L"LogFileFolder"
#define IoTDMRegistryReportToDeviceTwin L"ReportToDeviceTwin"
#define IoTDMRegistryTrue L"True"
#define IoTDMRegistryFalse L"False"

namespace Utils
{
    typedef std::function<void(std::vector<std::wstring>&, std::wstring&)>& ELEMENT_HANDLER;

    // Sid helper
    std::wstring GetSidForAccount(const wchar_t* userAccount);

    // Replaces invalid characters (like .) with _ so that the string can be used
    // as a json property name.
    std::wstring ToJsonPropertyName(const std::wstring& propertyName);

    // System helpers
    std::wstring GetCurrentDateTimeString();
    std::wstring GetDateTimeString(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute, unsigned int second);
    std::wstring GetOSVersionString();
    std::wstring GetEnvironmentVariable(const std::wstring& variableName);
    std::wstring GetSystemRootFolder();
    std::wstring GetProgramDataFolder();

    // Xml helpers
    void ReadXmlValue(const std::wstring& resultSyncML, const std::wstring& targetXmlPath, std::wstring& value);
    void ReadXmlStructData(const std::wstring& resultSyncML, ELEMENT_HANDLER handler);

    // Registry helpers
    void WriteRegistryValue(const std::wstring& subKey, const std::wstring& propName, const std::wstring& propValue);
    LSTATUS TryReadRegistryValue(const std::wstring& subKey, const std::wstring& propName, std::wstring& propValue);
    std::wstring ReadRegistryValue(const std::wstring& subKey, const std::wstring& propName);
    std::wstring ReadRegistryValue(const std::wstring& subKey, const std::wstring& propName, const std::wstring& propDefaultValue);

    // File helpers
    bool FileExists(const std::wstring& fullFileName);
    void EnsureFolderExists(const std::wstring& folder);

    // Process helpers
    void LaunchProcess(const std::wstring& commandString, unsigned long& returnCode, std::string& output);
    std::wstring GetProcessExePath(DWORD processID);
    bool IsProcessRunning(const std::wstring& processName);

    // Threading helpers
    class JoiningThread
    {
    public:
        std::thread& operator=(std::thread&& t)
        {
            _thread = std::move(t);
            return _thread;
        }

        void Join()
        {
            if (_thread.joinable())
            {
                _thread.join();
            }
        }

        ~JoiningThread()
        {
            Join();
        }
    private:
        std::thread _thread;
    };

    class AutoCloseSID : public AutoCloseBase<PSID>
    {
    public:
        AutoCloseSID() :
            AutoCloseBase(NULL, [](PSID h) { CloseHandle(h); return TRUE; })
        {}

        AutoCloseSID(PSID&& handle) :
            AutoCloseBase(std::move(handle), [](PSID h) { FreeSid(h); return TRUE; })
        {}

    private:
        AutoCloseSID(const AutoCloseSID &);            // prevent copy
        AutoCloseSID& operator=(const AutoCloseSID&);  // prevent assignment
    };

    class AutoCloseACL : public AutoCloseBase<PACL>
    {
    public:
        AutoCloseACL() :
            AutoCloseBase(NULL, [](PSID h) { CloseHandle(h); return TRUE; })
        {}

        AutoCloseACL(PACL&& handle) :
            AutoCloseBase(std::move(handle), [](PACL h) { LocalFree(h); return TRUE; })
        {}

    private:
        AutoCloseACL(const AutoCloseACL &);            // prevent copy
        AutoCloseACL& operator=(const AutoCloseACL&);  // prevent assignment
    };

    class AutoCloseServiceHandle : public AutoCloseBase<SC_HANDLE>
    {
    public:
        AutoCloseServiceHandle() :
            AutoCloseBase(NULL, [](SC_HANDLE h) { CloseServiceHandle(h); return TRUE; })
        {}

        AutoCloseServiceHandle(SC_HANDLE&& handle) :
            AutoCloseBase(std::move(handle), [](SC_HANDLE h) { CloseServiceHandle(h); return TRUE; })
        {}

    private:
        AutoCloseServiceHandle(const AutoCloseServiceHandle&);            // prevent copy
        AutoCloseServiceHandle& operator=(const AutoCloseServiceHandle&);  // prevent assignment
    };

    void LoadFile(const std::wstring& fileName, std::vector<char>& buffer);
    void Base64ToBinary(const std::wstring& encrypted, std::vector<char>& decrypted);
    std::wstring ToBase64(std::vector<char>& buffer);
    std::wstring FileToBase64(const std::wstring& fileName);
}
