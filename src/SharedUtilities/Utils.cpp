#include "stdafx.h"
// ToDo: Need to move this to the precompiled header.
#include <windows.h>
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

using namespace std;
using namespace Microsoft::WRL;
using namespace Windows::Data::Json;
using namespace Windows::System::Profile;

#define ERROR_PIPE_HAS_BEEN_ENDED 109

namespace Utils
{
    wstring GetSidForAccount(const wchar_t* userAccount)
    {
        wstring sidString = L"";

        BYTE userSidBytes[SECURITY_MAX_SID_SIZE] = {};
        PSID userSid = reinterpret_cast<PSID>(userSidBytes);
        DWORD sidSize = ARRAYSIZE(userSidBytes);
        wchar_t domainNameUnused[MAX_PATH] = {};
        DWORD domainSizeUnused = ARRAYSIZE(domainNameUnused);
        SID_NAME_USE sidTypeUnused = SidTypeInvalid;

        if (!LookupAccountName(
                nullptr,
                userAccount,
                &userSid,
                &sidSize,
                domainNameUnused,
                &domainSizeUnused,
                &sidTypeUnused
            ))
        {
            throw DMExceptionWithErrorCode("Error: Utils::GetSidForAccount LookupAccountName failed.", GetLastError());
        }
        LPWSTR pString = nullptr;
        if (!ConvertSidToStringSid(
                &userSid,
                &pString
            ))
        { 
            throw DMExceptionWithErrorCode("Error: Utils::GetSidForAccount ConvertSidToStringSid failed.", GetLastError());
        }
        sidString = pString;
        LocalFree(pString);

        return sidString;
    }

    string WideToMultibyte(const wchar_t* s)
    {
        size_t length = s ? wcslen(s) : 0;
        size_t requiredCharCount = WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0, nullptr, nullptr);

        // add room for \0
        ++requiredCharCount;

        vector<char> multibyteString(requiredCharCount);
        WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), multibyteString.data(), static_cast<int>(multibyteString.size()), nullptr, nullptr);

        return string(multibyteString.data());
    }

    wstring MultibyteToWide(const char* s)
    {
        size_t length = s ? strlen(s) : 0;
        size_t requiredCharCount = MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0);

        // add room for \0
        ++requiredCharCount;

        vector<wchar_t> wideString(requiredCharCount);
        MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), wideString.data(), static_cast<int>(wideString.size()));

        return wstring(wideString.data());
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
        wstring uriPath = L"Root\\Results\\Item\\Source\\LocURI\\";
        wstring dataPath = L"Root\\Results\\Item\\Data\\";
        wstring itemPath = L"Root\\Results\\Item\\";

        wstring emptyString = L"";
        auto value = emptyString;
        auto uri = emptyString;

        ComPtr<IXmlReader> xmlReader;

        HRESULT hr = CreateXmlReader(__uuidof(IXmlReader), (void**)xmlReader.GetAddressOf(), NULL);
        if (FAILED(hr))
        {
            throw DMExceptionWithHRESULT("Error: Failed to create xml reader.", hr);
        }

        hr = xmlReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
        if (FAILED(hr))
        {
            throw DMExceptionWithHRESULT("Error: XmlReaderProperty_DtdProcessing() failed.", hr);
        }

        hr = xmlReader->SetInput(resultSyncML);
        if (FAILED(hr))
        {
            throw DMExceptionWithHRESULT("Error: SetInput() failed.", hr);
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
                    throw DMExceptionWithHRESULT("Error: GetPrefix() failed.", hr);
                }

                const wchar_t* localName;
                hr = xmlReader->GetLocalName(&localName, NULL);
                if (FAILED(hr))
                {
                    throw DMExceptionWithHRESULT("Error: GetLocalName() failed.", hr);
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
                    throw DMExceptionWithHRESULT("Error: GetPrefix() failed.", hr);
                }

                const wchar_t* localName = NULL;
                hr = xmlReader->GetLocalName(&localName, NULL);
                if (FAILED(hr))
                {
                    throw DMExceptionWithHRESULT("Error: GetLocalName() failed.", hr);
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
                    throw DMExceptionWithHRESULT("Error: GetValue() failed.", hr);
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
            throw DMExceptionWithHRESULT(hr);
        }

        hr = xmlReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
        if (FAILED(hr))
        {
            TRACEP(L"Error: XmlReaderProperty_DtdProcessing() failed. Code :\n", hr);
            throw DMExceptionWithHRESULT(hr);
        }

        hr = xmlReader->SetInput(resultSyncML);
        if (FAILED(hr))
        {
            TRACEP(L"Error: SetInput() failed. Code :\n", hr);
            throw DMExceptionWithHRESULT(hr);
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
                    throw DMExceptionWithHRESULT(hr);
                }

                const wchar_t* localName;
                hr = xmlReader->GetLocalName(&localName, NULL);
                if (FAILED(hr))
                {
                    TRACEP(L"Error: GetLocalName() failed. Code :\n", hr);
                    throw DMExceptionWithHRESULT(hr);
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
                    throw DMExceptionWithHRESULT(hr);
                }

                const wchar_t* localName = NULL;
                hr = xmlReader->GetLocalName(&localName, NULL);
                if (FAILED(hr))
                {
                    TRACEP(L"Error: GetLocalName() failed. Code :", hr);
                    throw DMExceptionWithHRESULT(hr);
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
                    throw DMExceptionWithHRESULT(hr);
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
            throw DMExceptionWithHRESULT(hr);
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
            throw DMExceptionWithHRESULT(hr);
        }
        ReadXmlValue(dataStream.Get(), targetXmlPath, value);

        // GlobalFree() is not needed since 'delete on release' is enabled.
        // GlobalFree(buffer);
    }

    void WriteRegistryValue(const wstring& subkey, const wstring& propName, const wstring& propValue)
    {
        LSTATUS status;
        HKEY hKey = NULL;
        status = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            subkey.c_str(),
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

        status = RegSetValueEx(hKey, propName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(propValue.c_str()), (propValue.size() + 1) * sizeof(propValue[0]));
        if (status != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            throw DMExceptionWithErrorCode(status);
        }

        RegCloseKey(hKey);
    }

    wstring ReadRegistryValue(const wstring& subkey, const wstring& propName)
    {
        DWORD dataSize = 0;
        LSTATUS status;
        status = RegGetValue(HKEY_LOCAL_MACHINE, subkey.c_str(), propName.c_str(), RRF_RT_REG_SZ, NULL, NULL, &dataSize);
        if (status != ERROR_SUCCESS)
        {
            TRACEP(L"Error: Could not read registry value size: ", (subkey + L"\\" + propName).c_str());
            throw DMExceptionWithErrorCode(status);
        }

        vector<char> data(dataSize);
        status = RegGetValue(HKEY_LOCAL_MACHINE, subkey.c_str(), propName.c_str(), RRF_RT_REG_SZ, NULL, data.data(), &dataSize);
        if (status != ERROR_SUCCESS)
        {
            TRACEP(L"Error: Could not read registry value: ", (subkey + L"\\" + propName).c_str());
            throw DMExceptionWithErrorCode(status);
        }

        // return wstring(reinterpret_cast<const wchar_t*>(data.data()));
        return wstring(reinterpret_cast<const wchar_t*>(data.data()));
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
        charCount = ::GetEnvironmentVariable(variableName.c_str(), buffer.data(), buffer.size());
        if (charCount == 0)
        {
            throw DMExceptionWithErrorCode(GetLastError());
        }

        return wstring(buffer.data());
    }

    wstring GetSystemRootFolder()
    {
        return GetEnvironmentVariable(L"SystemRoot");
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
                    if (ReadFile(stdOutReadHandle.Get(), readBuffer.data(), readBuffer.size() - 1, &readByteCount, NULL) || readByteCount == 0)
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

    void LoadFile(const wstring& fileName, vector<char>& buffer)
    {
        TRACE(__FUNCTION__);
        TRACEP(L"fileName = ", fileName.c_str());

        ifstream file(fileName, ios::in | ios::binary | ios::ate);

        string line;
        if (!file.is_open())
        {
            throw new DMException("Error: failed to open binary file!");
        }

        buffer.resize(static_cast<unsigned int>(file.tellg()));
        file.seekg(0, ios::beg);
        if (!file.read(buffer.data(), buffer.size()))
        {
            throw new DMException("Error: failed to read file!");
        }
        file.close();
    }

    wstring ToBase64(std::vector<char>& buffer)
    {
        TRACE(__FUNCTION__);

        DWORD destinationSize = 0;
        if (!CryptBinaryToString(reinterpret_cast<unsigned char*>(buffer.data()), buffer.size(), CRYPT_STRING_BASE64, nullptr, &destinationSize))
        {
            throw new DMException("Error: cannot obtain the required size to encode buffer into base64.");
        }

        vector<wchar_t> destinationBuffer(destinationSize);
        if (!CryptBinaryToString(reinterpret_cast<unsigned char*>(buffer.data()), buffer.size(), CRYPT_STRING_BASE64, destinationBuffer.data(), &destinationSize))
        {
            throw new DMException("Error: cannot convert binary stream to base64.");
        }

        return wstring(destinationBuffer.size(), destinationSize);
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