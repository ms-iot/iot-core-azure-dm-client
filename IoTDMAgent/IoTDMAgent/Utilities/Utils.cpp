#include "stdafx.h"
#include <windows.h>
#include <wrl/client.h>
#include <xmllite.h>
#include "Utils.h"
#include "DMException.h"
#include "..\resource.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace Windows::System::Profile;

#define ERROR_PIPE_HAS_BEEN_ENDED 109

namespace Utils
{

class HandleRAII
{
public:
    HandleRAII() :
        _handle(NULL)
    {}

    HANDLE Get() { return _handle; }
    HANDLE* GetAddress() { return &_handle; }

    BOOL Close()
    {
        BOOL result = TRUE;
        if (_handle != NULL)
        {
            result = CloseHandle(_handle);
            _handle = NULL;
        }
        return result;
    }

    ~HandleRAII()
    {
        Close();
    }

private:
    HandleRAII(const HandleRAII&);            // prevent copy
    HandleRAII& operator=(const HandleRAII&); // prevent assignment

    HANDLE _handle;
};

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

wstring GetResourceString(int id)
{
    const int bufferSize = 1024;
    wchar_t buffer[bufferSize];
    if (!LoadString(GetModuleHandle(NULL), id, buffer, bufferSize))
    {
        if (!LoadString(GetModuleHandle(NULL), IDS_OMA_SYNCML_STATUS_UNKNOWN, buffer, bufferSize))
        {
            return wstring(L"Unknown OMA SyncML status code.");
        }
    }
    return wstring(buffer);
}

void ReadXmlValue(IStream* resultSyncML, const wstring& targetXmlPath, wstring& value)
{
    ComPtr<IXmlReader> xmlReader;

    HRESULT hr = CreateXmlReader(__uuidof(IXmlReader), (void**)xmlReader.GetAddressOf(), NULL);
    if (FAILED(hr))
    {
        TRACEP(L"Error: Failed to create xml reader. Code :", hr);
        throw exception();
    }

    hr = xmlReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
    if (FAILED(hr))
    {
        TRACEP(L"Error: XmlReaderProperty_DtdProcessing() failed. Code :\n", hr);
        throw exception();
    }

    hr = xmlReader->SetInput(resultSyncML);
    if (FAILED(hr))
    {
        TRACEP(L"Error: SetInput() failed. Code :\n", hr);
        throw exception();
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
                throw exception();
            }

            const wchar_t* localName;
            hr = xmlReader->GetLocalName(&localName, NULL);
            if (FAILED(hr))
            {
                TRACEP(L"Error: GetLocalName() failed. Code :\n", hr);
                throw exception();
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
                throw exception();
            }

            const wchar_t* localName = NULL;
            hr = xmlReader->GetLocalName(&localName, NULL);
            if (FAILED(hr))
            {
                TRACEP(L"Error: GetLocalName() failed. Code :", hr);
                throw exception();
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
                throw exception();
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
        throw exception();
    }
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
        throw exception();
    }
    ReadXmlValue(dataStream.Get(), targetXmlPath, value);

    // GlobalFree() is not needed since 'delete on release' is enabled.
    // GlobalFree(buffer);
}

void WriteRegistryValue(const wstring& subkey, const wstring& propName, const wstring& propValue)
{
    bool success = false;
    HKEY hKey = NULL;
    if (ERROR_SUCCESS == RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        subkey.c_str(),
        0,      // reserved
        NULL,   // user-defined class type of this key.
        0,      // default; non-volatile
        KEY_ALL_ACCESS,
        NULL,   // inherit security descriptor from parent.
        &hKey,
        NULL    // disposition [optional, out]
    ))
    {
        if (ERROR_SUCCESS == RegSetValueEx(hKey, propName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(propValue.c_str()), (propValue.size() + 1) * sizeof(propValue[0])))
        {
            success = true;
        }
        RegCloseKey(hKey);
    }

    if (!success)
    {
        throw exception();
    }
}

wstring ReadRegistryValue(const wstring& subkey, const wstring& propName)
{
    DWORD dataSize = 0;
    if (ERROR_SUCCESS != RegGetValue(HKEY_LOCAL_MACHINE, subkey.c_str(), propName.c_str(), RRF_RT_REG_SZ, NULL, NULL, &dataSize))
    {
        TRACEP(L"Error: Could not read registry value size: ", (subkey + L"\\" + propName).c_str());
        throw DMException("Failed to read registry value size.");
    }

    vector<char> data(dataSize);
    if (ERROR_SUCCESS != RegGetValue(HKEY_LOCAL_MACHINE, subkey.c_str(), propName.c_str(), RRF_RT_REG_SZ, NULL, data.data(), &dataSize))
    {
        TRACEP(L"Error: Could not read registry value: ", (subkey + L"\\" + propName).c_str());
        throw DMException("Failed to read registry value.");
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
                    throw DMException("Failed to create folder.", folder.c_str());
                }
            }
        }
        ++index;
    }
}

wstring ToJsonPropoertyName(const wstring& propertyName)
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

    HandleRAII stdOutReadHandle;
    HandleRAII stdOutWriteHandle;
    DWORD  pipeBufferSize = 4096;

    if (!CreatePipe(stdOutReadHandle.GetAddress(), stdOutWriteHandle.GetAddress(), &securityAttributes, pipeBufferSize))
    {
        throw DMException("Error: Failed to create pipe. GetLastError() = ", GetLastError());
    }

    if (!SetHandleInformation(stdOutReadHandle.Get(), HANDLE_FLAG_INHERIT, 0 /*flags*/))
    {
        throw DMException("Error: Failed to configure the stdout read handle. GetLastError() = ", GetLastError());
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
        throw DMException("Error: Failed to create process. GetLastError() = ", GetLastError());
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

}