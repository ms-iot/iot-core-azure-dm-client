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
#include "stdafx.h"
// ToDo: Need to move this to the precompiled header.
#include <windows.h>
#include <psapi.h>
#include <wrl/client.h>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm> 
#include <xmllite.h>
#include <fstream>
#include <Sddl.h>
#include "Utils.h"
#include "DMException.h"
#include "Logger.h"

// WTSQueryUserToken
#include "Wtsapi32.h"
// EnumProcesses
#include "Psapi.h"
// SHGetFolderPath
#include "Shlobj.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace Windows::ApplicationModel;
using namespace Windows::Data::Json;
using namespace Windows::Management::Deployment;
using namespace Windows::System::Profile;

#define ERROR_PIPE_HAS_BEEN_ENDED 109

namespace Utils
{
    void GetDmUserInfo(TOKEN_HANDLER handler)
    {
        const size_t processNameLength = wcslen(IoTDMSihostExe);
        vector<DWORD> spProcessIds(1024);
        DWORD bytesReturned = 0;
        WCHAR imageFileName[MAX_PATH];

        if (EnumProcesses(
            &spProcessIds.front(),
            static_cast<unsigned int>(spProcessIds.size() * sizeof(DWORD)),
            &bytesReturned))
        {
            auto actualProcessIds = bytesReturned / sizeof(unsigned int);

            for (unsigned int i = 0; i < actualProcessIds; i++)
            {
                DWORD error = 0;
                AutoCloseHandle processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, spProcessIds[i]);
                if (processHandle.Get() == INVALID_HANDLE_VALUE) continue;

                auto imageFileNameLength = GetProcessImageFileNameW(processHandle.Get(), imageFileName, _countof(imageFileName));
                if ((imageFileNameLength < processNameLength) || (_wcsicmp(IoTDMSihostExe, &imageFileName[imageFileNameLength - processNameLength]) != 0)) continue;

                AutoCloseHandle processTokenHandle;
                error = OpenProcessToken(processHandle.Get(), TOKEN_READ, processTokenHandle.GetAddress());
                if (FAILED(error))
                {
                    TRACEP(L"OpenProcessToken failed. Code: ", error);
                    continue;
                }

                DWORD sessionID = 0;
                DWORD size = 0;
                if (!GetTokenInformation(processTokenHandle.Get(), TokenSessionId, &sessionID, sizeof(sessionID), &size))
                {
                    TRACEP(L"GetTokenInformation(TokenSessionId) failed. Code: ", GetLastError());
                    continue;
                }

                BYTE buffer[SECURITY_MAX_SID_SIZE];
                PTOKEN_USER tokenUser = reinterpret_cast<PTOKEN_USER>(buffer);
                DWORD tokenUserSize = sizeof(buffer);
                if (!GetTokenInformation(processTokenHandle.Get(), TokenUser, tokenUser, tokenUserSize, &tokenUserSize))
                {
                    TRACEP(L"GetTokenInformation(TokenUser) failed. Code: ", GetLastError());
                    continue;
                }

                handler(processTokenHandle.Get(), tokenUser);
                return;
            }
        }
        else
        {
            throw DMExceptionWithErrorCode("EnumProcesses failed.", GetLastError());
        }

        throw DMExceptionWithErrorCode("GetDmUserInfo: no user process found.", E_FAIL);
    }

    wstring GetDmUserSid() 
    {
        wstring sid(L"");
        GetDmUserInfo([&sid](HANDLE /*token*/, PTOKEN_USER tokenUser) {
            WCHAR *pCOwner = NULL;
            if (ConvertSidToStringSid(tokenUser->User.Sid, &pCOwner))
            {
                sid = pCOwner;
                LocalFree(pCOwner);
            }
            else
            {
                throw DMExceptionWithErrorCode("ConvertSidToStringSid failed.", GetLastError());
            }
        });

        return sid;
    }

    wstring GetDmUserName() 
    {
        wstring name(L"");
        GetDmUserInfo([&name](HANDLE /*token*/, PTOKEN_USER tokenUser) {
            DWORD cchDomainName = 0, cchAccountName = 0;
            SID_NAME_USE AccountType = SidTypeUnknown;
            LookupAccountSid(NULL, tokenUser->User.Sid, NULL, &cchAccountName, NULL, &cchDomainName, &AccountType);
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                vector<wchar_t> DomainName(cchDomainName);
                vector<wchar_t> AccountName(cchAccountName);
                if (LookupAccountSid(NULL, tokenUser->User.Sid, AccountName.data(), &cchAccountName, DomainName.data(), &cchDomainName, &AccountType))
                {
                    name = AccountName.data();
                }
                else
                {
                    throw DMExceptionWithErrorCode("LookupAccountSid failed.", GetLastError());
                }
            }
            else
            {
                throw DMExceptionWithErrorCode("LookupAccountSid(NULL) failed.", GetLastError());
            }
        });

        return name;
    }

    wstring GetDmTempFolder()
    {
        WCHAR szPath[MAX_PATH];
        wstring folder(L"");

        DWORD result = GetTempPath(MAX_PATH, szPath);
        if (result)
        {
            folder = szPath;
        }
        else
        {
            throw DMExceptionWithErrorCode("GetTempPath failed.", GetLastError());
        }

        return folder;
    }

    wstring GetDmUserFolder()
    {
        // this works on IoT Core and IoT Enterprise (not IoT Enterprise 
        // Mobile ... SHGetFolderPath not implemented there)
        wstring folder(L"");
        GetDmUserInfo([&folder](HANDLE token, PTOKEN_USER /*tokenUser*/) {
            WCHAR szPath[MAX_PATH];
            HRESULT hr = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, token, 0, szPath);
            if (SUCCEEDED(hr))
            {
                folder = szPath;
                folder += L"\\Temp\\";
            }
            else
            {
                TRACEP(L"SHGetFolderPath failed. Code: ", hr);
            }
        });

        return folder;
    }

    wstring GetCurrentDateTimeString()
    {
        SYSTEMTIME systemTime;
        GetLocalTime(&systemTime);

        return GetDateTimeString(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
    }

    wstring GetDateTimeString(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute, unsigned int second)
    {
        basic_ostringstream<wchar_t> formattedTime;
        formattedTime << setw(2) << setfill(L'0') << year
            << L'-' << setw(2) << setfill(L'0') << month
            << L'-' << setw(2) << setfill(L'0') << day
            << L'T' << setw(2) << setfill(L'0') << hour
            << L':' << setw(2) << setfill(L'0') << minute
            << L':' << setw(2) << setfill(L'0') << second;
        return formattedTime.str();
    }

    void ReadXmlStructData(IStream* resultSyncML, ELEMENT_HANDLER handler)
    {
        wstring uriPath = L"SyncML\\SyncBody\\Results\\Item\\Source\\LocURI\\";
        wstring dataPath = L"SyncML\\SyncBody\\Results\\Item\\Data\\";
        wstring itemPath = L"SyncML\\SyncBody\\Results\\Item\\";

        wstring emptyString = L"";
        auto value = emptyString;
        auto uri = emptyString;

        ComPtr<IXmlReader> xmlReader;

        HRESULT hr = CreateXmlReader(__uuidof(IXmlReader), (void**)xmlReader.GetAddressOf(), NULL);
        if (FAILED(hr))
        {
            throw DMExceptionWithErrorCode("Error: Failed to create xml reader.", hr);
        }

        hr = xmlReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
        if (FAILED(hr))
        {
            throw DMExceptionWithErrorCode("Error: XmlReaderProperty_DtdProcessing() failed.", hr);
        }

        hr = xmlReader->SetInput(resultSyncML);
        if (FAILED(hr))
        {
            throw DMExceptionWithErrorCode("Error: SetInput() failed.", hr);
        }

        deque<wstring> pathStack;
        wstring currentPath;

        // Read until there are no more nodes
        XmlNodeType nodeType;
        while (S_OK == (hr = xmlReader->Read(&nodeType)))
        {
            switch (nodeType)
            {
            case XmlNodeType_Element:
            {
                const wchar_t* prefix = NULL;
                UINT prefixSize = 0;

                hr = xmlReader->GetPrefix(&prefix, &prefixSize);
                if (FAILED(hr))
                {
                    throw DMExceptionWithErrorCode("Error: GetPrefix() failed.", hr);
                }

                const wchar_t* localName;
                hr = xmlReader->GetLocalName(&localName, NULL);
                if (FAILED(hr))
                {
                    throw DMExceptionWithErrorCode("Error: GetLocalName() failed.", hr);
                }

                wstring elementName;
                if (prefixSize > 0)
                {
                    elementName = prefix;
                    elementName += L":";
                }
                elementName += localName;

                if (!xmlReader->IsEmptyElement())
                {
                    pathStack.push_back(elementName);

                    // rebuild the current path.
                    currentPath = L"";
                    for (auto& it : pathStack)
                    {
                        currentPath += it + L"\\";
                    }
                    if (itemPath == currentPath)
                    {
                        value = emptyString;
                        uri = emptyString;
                    }
                }
            }
            break;
            case XmlNodeType_EndElement:
            {
                const wchar_t* prefix = NULL;
                UINT prefixSize = 0;

                hr = xmlReader->GetPrefix(&prefix, &prefixSize);
                if (FAILED(hr))
                {
                    throw DMExceptionWithErrorCode("Error: GetPrefix() failed.", hr);
                }

                const wchar_t* localName = NULL;
                hr = xmlReader->GetLocalName(&localName, NULL);
                if (FAILED(hr))
                {
                    throw DMExceptionWithErrorCode("Error: GetLocalName() failed.", hr);
                }

                if (itemPath == currentPath)
                {
                    vector<wstring> uriTokens;
                    wstringstream ss(uri);
                    wstring s;

                    while (getline(ss, s, L'/')) 
                    {
                        uriTokens.push_back(s);
                    }

                    handler(uriTokens, value);

                    value = emptyString;
                    uri = emptyString;
                }
                pathStack.pop_back();
                // rebuild the current path.
                currentPath = L"";
                for (auto& it : pathStack)
                {
                    currentPath += it + L"\\";
                }

            }
            break;
            case XmlNodeType_Text:
            case XmlNodeType_Whitespace:
            {
                const wchar_t* valueText = NULL;
                hr = xmlReader->GetValue(&valueText, NULL);
                if (FAILED(hr))
                {
                    throw DMExceptionWithErrorCode("Error: GetValue() failed.", hr);
                }

                if (uriPath == currentPath)
                {
                    uri = valueText;
                }
                else if (dataPath == currentPath)
                {
                    value = valueText;
                }
            }
            break;
            }
        }
    }
    
    void ReadXmlValue(IStream* resultSyncML, const wstring& targetXmlPath, wstring& value)
    {
        ComPtr<IXmlReader> xmlReader;

        HRESULT hr = CreateXmlReader(__uuidof(IXmlReader), (void**)xmlReader.GetAddressOf(), NULL);
        if (FAILED(hr))
        {
            TRACEP(L"Error: Failed to create xml reader. Code :", hr);
            throw DMExceptionWithErrorCode(hr);
        }

        hr = xmlReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
        if (FAILED(hr))
        {
            TRACEP(L"Error: XmlReaderProperty_DtdProcessing() failed. Code :\n", hr);
            throw DMExceptionWithErrorCode(hr);
        }

        hr = xmlReader->SetInput(resultSyncML);
        if (FAILED(hr))
        {
            TRACEP(L"Error: SetInput() failed. Code :\n", hr);
            throw DMExceptionWithErrorCode(hr);
        }

        deque<wstring> pathStack;
        wstring currentPath;

        // Read until there are no more nodes
        bool valueFound = false;
        bool pathFound = false;
        XmlNodeType nodeType;
        while (S_OK == (hr = xmlReader->Read(&nodeType)) && !valueFound)
        {
            switch (nodeType)
            {
            case XmlNodeType_Element:
            {
                const wchar_t* prefix = NULL;
                UINT prefixSize = 0;

                hr = xmlReader->GetPrefix(&prefix, &prefixSize);
                if (FAILED(hr))
                {
                    TRACEP(L"Error: GetPrefix() failed. Code :\n", hr);
                    throw DMExceptionWithErrorCode(hr);
                }

                const wchar_t* localName;
                hr = xmlReader->GetLocalName(&localName, NULL);
                if (FAILED(hr))
                {
                    TRACEP(L"Error: GetLocalName() failed. Code :\n", hr);
                    throw DMExceptionWithErrorCode(hr);
                }

                wstring elementName;
                if (prefixSize > 0)
                {
                    elementName = prefix;
                    elementName += L":";
                }
                elementName += localName;

                if (!xmlReader->IsEmptyElement())
                {
                    pathStack.push_back(elementName);

                    // rebuild the current path.
                    currentPath = L"";
                    for (auto& it : pathStack)
                    {
                        currentPath += it + L"\\";
                    }
                    if (targetXmlPath == currentPath)
                    {
                        pathFound = true;
                    }
                }
            }
            break;
            case XmlNodeType_EndElement:
            {
                const wchar_t* prefix = NULL;
                UINT prefixSize = 0;

                hr = xmlReader->GetPrefix(&prefix, &prefixSize);
                if (FAILED(hr))
                {
                    TRACEP(L"Error: GetPrefix() failed. Code :", hr);
                    throw DMExceptionWithErrorCode(hr);
                }

                const wchar_t* localName = NULL;
                hr = xmlReader->GetLocalName(&localName, NULL);
                if (FAILED(hr))
                {
                    TRACEP(L"Error: GetLocalName() failed. Code :", hr);
                    throw DMExceptionWithErrorCode(hr);
                }

                pathStack.pop_back();
            }
            break;
            case XmlNodeType_Text:
            case XmlNodeType_Whitespace:
            {
                const wchar_t* valueText = NULL;
                hr = xmlReader->GetValue(&valueText, NULL);
                if (FAILED(hr))
                {
                    TRACEP(L"Error: GetValue() failed. Code :", hr);
                    throw DMExceptionWithErrorCode(hr);
                }

                if (targetXmlPath == currentPath)
                {
                    value = valueText;
                    valueFound = true;
                }
            }
            break;
            }
        }

        if (!pathFound)
        {
            TRACEP(L"Error: Failed to read: ", targetXmlPath.c_str());
            throw DMException("ReadXmlValue: path not found");
        }
    }

    void ReadXmlStructData(const wstring& resultSyncML, Utils::ELEMENT_HANDLER handler)
    {
        DWORD bufferSize = static_cast<DWORD>(resultSyncML.size() * sizeof(resultSyncML[0]));
        char* buffer = (char*)GlobalAlloc(GMEM_FIXED, bufferSize);
        memcpy(buffer, resultSyncML.c_str(), bufferSize);

        ComPtr<IStream> dataStream;
        HRESULT hr = ::CreateStreamOnHGlobal(buffer, TRUE /*delete on release*/, dataStream.GetAddressOf());
        if (FAILED(hr))
        {
            GlobalFree(buffer);
            throw DMExceptionWithErrorCode(hr);
        }
        ReadXmlStructData(dataStream.Get(), handler);

        // GlobalFree() is not needed since 'delete on release' is enabled.
        // GlobalFree(buffer);
    }

    void ReadXmlValue(const wstring& resultSyncML, const wstring& targetXmlPath, wstring& value)
    {
        DWORD bufferSize = static_cast<DWORD>(resultSyncML.size() * sizeof(resultSyncML[0]));
        char* buffer = (char*)GlobalAlloc(GMEM_FIXED, bufferSize);
        memcpy(buffer, resultSyncML.c_str(), bufferSize);

        ComPtr<IStream> dataStream;
        HRESULT hr = ::CreateStreamOnHGlobal(buffer, TRUE /*delete on release*/, dataStream.GetAddressOf());
        if (FAILED(hr))
        {
            GlobalFree(buffer);
            throw DMExceptionWithErrorCode(hr);
        }
        ReadXmlValue(dataStream.Get(), targetXmlPath, value);

        // GlobalFree() is not needed since 'delete on release' is enabled.
        // GlobalFree(buffer);
    }

    void WriteRegistryValue(const wstring& subKey, const wstring& propName, const wstring& propValue)
    {
        LSTATUS status;
        HKEY hKey = NULL;
        status = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            subKey.c_str(),
            0,      // reserved
            NULL,   // user-defined class type of this key.
            0,      // default; non-volatile
            KEY_ALL_ACCESS,
            NULL,   // inherit security descriptor from parent.
            &hKey,
            NULL    // disposition [optional, out]
        );
        if (status != ERROR_SUCCESS) {
            throw DMExceptionWithErrorCode(status);
        }

        status = RegSetValueEx(hKey, propName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(propValue.c_str()), (static_cast<unsigned int>(propValue.size()) + 1) * sizeof(propValue[0]));
        if (status != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            throw DMExceptionWithErrorCode(status);
        }

        RegCloseKey(hKey);
    }

    void WriteRegistryValue(const wstring& subKey, const wstring& propName, unsigned long propValue)
    {
        LSTATUS status;
        HKEY hKey = NULL;
        status = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            subKey.c_str(),
            0,      // reserved
            NULL,   // user-defined class type of this key.
            0,      // default; non-volatile
            KEY_ALL_ACCESS,
            NULL,   // inherit security descriptor from parent.
            &hKey,
            NULL    // disposition [optional, out]
        );
        if (status != ERROR_SUCCESS) {
            throw DMExceptionWithErrorCode(status);
        }

        status = RegSetValueEx(hKey, propName.c_str(), 0, REG_DWORD, reinterpret_cast<BYTE*>(&propValue), sizeof(propValue));
        if (status != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            throw DMExceptionWithErrorCode(status);
        }

        RegCloseKey(hKey);
    }

    LSTATUS TryReadRegistryValue(const wstring& subKey, const wstring& propName, wstring& propValue)
    {
        DWORD dataSize = 0;
        LSTATUS status;
        status = RegGetValue(HKEY_LOCAL_MACHINE, subKey.c_str(), propName.c_str(), RRF_RT_REG_SZ, NULL, NULL, &dataSize);
        if (status != ERROR_SUCCESS)
        {
            return status;
        }

        vector<char> data(dataSize);
        status = RegGetValue(HKEY_LOCAL_MACHINE, subKey.c_str(), propName.c_str(), RRF_RT_REG_SZ, NULL, data.data(), &dataSize);
        if (status != ERROR_SUCCESS)
        {
            return status;
        }

        propValue = reinterpret_cast<const wchar_t*>(data.data());

        return ERROR_SUCCESS;
    }

    LSTATUS TryReadRegistryValue(const wstring& subKey, const wstring& propName, unsigned long& propValue)
    {
        DWORD dataSize = 0;
        LSTATUS status;
        status = RegGetValue(HKEY_LOCAL_MACHINE, subKey.c_str(), propName.c_str(), RRF_RT_REG_DWORD, NULL, NULL, &dataSize);
        if (status != ERROR_SUCCESS)
        {
            return status;
        }

        vector<char> data(dataSize);
        status = RegGetValue(HKEY_LOCAL_MACHINE, subKey.c_str(), propName.c_str(), RRF_RT_REG_DWORD, NULL, data.data(), &dataSize);
        if (status != ERROR_SUCCESS)
        {
            return status;
        }

        propValue = *(reinterpret_cast<const unsigned long*>(data.data()));

        return ERROR_SUCCESS;
    }

    wstring ReadRegistryValue(const wstring& subKey, const wstring& propName)
    {
        wstring propValue;
        LSTATUS status = TryReadRegistryValue(subKey, propName, propValue);
        if (status != ERROR_SUCCESS)
        {
            TRACEP(L"Error: Could not read registry value: ", (subKey + L"\\" + propName).c_str());
            throw DMExceptionWithErrorCode(status);
        }
        return propValue;
    }

    wstring ReadRegistryValue(const wstring& subKey, const wstring& propName, const wstring& propDefaultValue)
    {
        wstring propValue;
        if (ERROR_SUCCESS != Utils::TryReadRegistryValue(subKey, propName, propValue))
        {
            propValue = propDefaultValue;
        }
        return propValue;
    }

    wstring GetOSVersionString()
    {
        AnalyticsVersionInfo^ info = AnalyticsInfo::VersionInfo;
        Platform::String^ s = info->DeviceFamilyVersion;

        wchar_t* endPtr = nullptr;
        __int64 v = wcstoll(s->Data(), &endPtr, 10);

        unsigned long v1 = (v & 0xFFFF000000000000L) >> 48;
        unsigned long v2 = (v & 0x0000FFFF00000000L) >> 32;
        unsigned long v3 = (v & 0x00000000FFFF0000L) >> 16;
        unsigned long v4 = v & 0x000000000000FFFFL;

        basic_ostringstream<wchar_t> formattedVersion;
        formattedVersion << v1 << L"." << v2 << L"." << v3 << L"." << v4;

        return formattedVersion.str();
    }

    wstring GetEnvironmentVariable(const wstring& variableName)
    {
        DWORD charCount = ::GetEnvironmentVariable(variableName.c_str(), NULL, 0);
        if (charCount == 0)
        {
            throw DMExceptionWithErrorCode(GetLastError());
        }

        vector<wchar_t> buffer(charCount);
        charCount = ::GetEnvironmentVariable(variableName.c_str(), buffer.data(), static_cast<unsigned int>(buffer.size()));
        if (charCount == 0)
        {
            throw DMExceptionWithErrorCode(GetLastError());
        }

        return wstring(buffer.data());
    }

    wstring GetSystemRootFolder()
    {
        UINT size = GetSystemDirectory(0, 0);

        vector<wchar_t> buffer(size);
        if (size != GetSystemDirectory(buffer.data(), static_cast<unsigned int>(buffer.size())) + 1)
        {
            throw DMException("Error: failed to retrieve system folder.");
        }

        return wstring(buffer.data());
    }

    wstring GetProgramDataFolder()
    {
        return GetEnvironmentVariable(L"ProgramData");
    }

    bool FileExists(const wstring& fullFileName)
    {
        ifstream infile(fullFileName);
        return infile.good();
    }

    void EnsureFolderExists(const wstring& folder)
    {
        vector<wstring> tokens;
        SplitString(folder, L'\\', tokens);
        size_t index = 0;
        wstring path = L"";
        for (const wstring& s : tokens)
        {
            if (index == 0)
            {
                path += s;
            }
            else
            {
                path += L"\\";
                path += s;
                if (ERROR_SUCCESS != CreateDirectory(path.c_str(), NULL))
                {
                    if (ERROR_ALREADY_EXISTS != GetLastError())
                    {
                        throw DMExceptionWithErrorCode(GetLastError());
                    }
                }
            }
            ++index;
        }
    }

    wstring ToJsonPropertyName(const wstring& propertyName)
    {
        wstring jsonPropertyName = propertyName;
        replace(jsonPropertyName.begin(), jsonPropertyName.end(), L'.', L'_');
        replace(jsonPropertyName.begin(), jsonPropertyName.end(), L'-', L'_');
        return jsonPropertyName;
    }

    void LaunchProcess(const wstring& commandString, unsigned long& returnCode, string& output)
    {
        TRACEP(L"Launching: ", commandString.c_str());

        SECURITY_ATTRIBUTES securityAttributes;
        securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        securityAttributes.bInheritHandle = TRUE;
        securityAttributes.lpSecurityDescriptor = NULL;

        AutoCloseHandle stdOutReadHandle;
        AutoCloseHandle stdOutWriteHandle;
        DWORD pipeBufferSize = 4096;

        if (!CreatePipe(stdOutReadHandle.GetAddress(), stdOutWriteHandle.GetAddress(), &securityAttributes, pipeBufferSize))
        {
            throw DMExceptionWithErrorCode(GetLastError());
        }

        if (!SetHandleInformation(stdOutReadHandle.Get(), HANDLE_FLAG_INHERIT, 0 /*flags*/))
        {
            throw DMExceptionWithErrorCode(GetLastError());
        }

        PROCESS_INFORMATION piProcInfo;
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        STARTUPINFO siStartInfo;
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = stdOutWriteHandle.Get();
        siStartInfo.hStdOutput = stdOutWriteHandle.Get();
        siStartInfo.hStdInput = NULL;
siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

if (!CreateProcess(NULL,
    const_cast<wchar_t*>(commandString.c_str()), // command line 
    NULL,         // process security attributes 
    NULL,         // primary thread security attributes 
    TRUE,         // handles are inherited 
    0,            // creation flags 
    NULL,         // use parent's environment 
    NULL,         // use parent's current directory 
    &siStartInfo, // STARTUPINFO pointer 
    &piProcInfo)) // receives PROCESS_INFORMATION
{
    throw DMExceptionWithErrorCode(GetLastError());
}
TRACE("Child process has been launched.");

bool doneWriting = false;
while (!doneWriting)
{
    // Let the child process run for 1 second, and then check if there is anything to read...
    DWORD waitStatus = WaitForSingleObject(piProcInfo.hProcess, 1000);
    if (waitStatus == WAIT_OBJECT_0)
    {
        TRACE("Child process has exited.");
        if (!GetExitCodeProcess(piProcInfo.hProcess, &returnCode))
        {
            TRACEP("Warning: Failed to get process exist code. GetLastError() = ", GetLastError());
            // ToDo: do we ignore?
        }
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);

        // Child process has exited, no more writing will take place.
        // Without closing the write channel, the ReadFile will keep waiting.
        doneWriting = true;
        stdOutWriteHandle.Close();
    }
    else
    {
        TRACE("Child process is still running...");
    }

    DWORD bytesAvailable = 0;
    if (PeekNamedPipe(stdOutReadHandle.Get(), NULL, 0, NULL, &bytesAvailable, NULL))
    {
        if (bytesAvailable > 0)
        {
            DWORD readByteCount = 0;
            vector<char> readBuffer(bytesAvailable + 1);
            if (ReadFile(stdOutReadHandle.Get(), readBuffer.data(), static_cast<unsigned int>(readBuffer.size()) - 1, &readByteCount, NULL) || readByteCount == 0)
            {
                readBuffer[readByteCount] = '\0';
                output += readBuffer.data();
            }
        }
    }
    else
    {
        DWORD retCode = GetLastError();
        if (ERROR_PIPE_HAS_BEEN_ENDED != retCode)
        {
            printf("error code = %d\n", retCode);
        }
        break;
    }
}

TRACEP("Command return Code: ", returnCode);
TRACEP("Command output : ", output.c_str());
    }

    wstring GetProcessExePath(DWORD processID)
    {
        wchar_t exePath[MAX_PATH] = TEXT("<unknown>");

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
        if (NULL != hProcess)
        {
            HMODULE hModule;
            DWORD cbNeeded;

            if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded))
            {
                GetModuleFileNameEx(hProcess, hModule, exePath, sizeof(exePath) / sizeof(wchar_t));
            }

            CloseHandle(hProcess);
        }

        return wstring(exePath);
    }

    bool IsProcessRunning(const wstring& processName)
    {
        TRACE(__FUNCTION__);

        bool running = false;
        TRACEP(L"Checking: ", processName.c_str());

        DWORD processHandles[1024];
        DWORD bytesNeeded = 0;
        if (EnumProcesses(processHandles, sizeof(processHandles), &bytesNeeded))
        {
            DWORD processCount = bytesNeeded / sizeof(DWORD);
            for (DWORD i = 0; i < processCount; i++)
            {
                if (processHandles[i] == 0)
                {
                    continue;
                }
                wstring exePath = Utils::GetProcessExePath(processHandles[i]);
                TRACEP(L"Found Process: ", exePath.c_str());

                if (Utils::Contains(exePath, processName))
                {
                    TRACE(L"process is running!");
                    running = true;
                    break;
                }
            }
        }

        return running;
    }

    void LoadFile(const wstring& fileName, vector<char>& buffer)
    {
        TRACE(__FUNCTION__);
        TRACEP(L"fileName = ", fileName.c_str());

        ifstream file(fileName, ios::in | ios::binary | ios::ate);

        string line;
        if (!file.is_open())
        {
            throw DMException("Error: failed to open binary file!");
        }

        buffer.resize(static_cast<unsigned int>(file.tellg()));
        file.seekg(0, ios::beg);
        if (!file.read(buffer.data(), buffer.size()))
        {
            throw DMException("Error: failed to read file!");
        }
        file.close();
    }

    void Base64ToBinary(const wstring& encrypted, vector<char>& decrypted)
    {
        TRACE(__FUNCTION__);

        DWORD destinationSize = 0;
        if (!CryptStringToBinary(encrypted.c_str(), static_cast<unsigned int>(encrypted.size()), CRYPT_STRING_BASE64, nullptr, &destinationSize, nullptr, nullptr))
        {
            throw DMException("Error: cannot obtain the required size to decode buffer from base64.");
        }

        decrypted.resize(destinationSize);
        if (!CryptStringToBinary(encrypted.c_str(), static_cast<unsigned int>(encrypted.size()), CRYPT_STRING_BASE64, reinterpret_cast<unsigned char*>(decrypted.data()), &destinationSize, nullptr, nullptr))
        {
            throw DMException("Error: cannot obtain the required size to decode buffer from base64.");
        }
    }

    wstring ToBase64(vector<char>& buffer)
    {
        TRACE(__FUNCTION__);

        DWORD destinationSize = 0;
        if (!CryptBinaryToString(reinterpret_cast<unsigned char*>(buffer.data()), static_cast<unsigned int>(buffer.size()), CRYPT_STRING_BASE64, nullptr, &destinationSize))
        {
            throw DMException("Error: cannot obtain the required size to encode buffer into base64.");
        }

        vector<wchar_t> destinationBuffer(destinationSize);
        if (!CryptBinaryToString(reinterpret_cast<unsigned char*>(buffer.data()), static_cast<unsigned int>(buffer.size()), CRYPT_STRING_BASE64, destinationBuffer.data(), &destinationSize))
        {
            throw DMException("Error: cannot convert binary stream to base64.");
        }

        // Note that the size returned includes the null terminating character.
        return wstring(destinationBuffer.data(), destinationBuffer.size() - 1);
    }

    wstring FileToBase64(const wstring& fileName)
    {
        TRACE(__FUNCTION__);
        TRACEP(L"fileName = ", fileName.c_str());

        vector<char> buffer;
        LoadFile(fileName, buffer);
        return ToBase64(buffer);
    }

}
