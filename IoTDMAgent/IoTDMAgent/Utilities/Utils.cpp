#include "stdafx.h"
#include <windows.h>
#include <wrl/client.h>
#include <xmllite.h>
#include <time.h>
#include <string>
#include <vector>
#include <deque>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "Utils.h"
#include "Logger.h"
#include "..\resource.h"

using namespace std;
using namespace Microsoft::WRL;

namespace Utils
{

std::string WideToMultibyte(const wchar_t* s)
{
    size_t length = s ? wcslen(s) : 0;
    size_t requiredCharCount = WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0, nullptr, nullptr);

    // add room for \0
    ++requiredCharCount;

    vector<char> multibyteString(requiredCharCount);
    WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), multibyteString.data(), static_cast<int>(multibyteString.size()), nullptr, nullptr);

    return string(multibyteString.data());
}

std::wstring MultibyteToWide(const char* s)
{
    size_t length = s ? strlen(s) : 0;
    size_t requiredCharCount = MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0);

    // add room for \0
    ++requiredCharCount;

    vector<wchar_t> wideString(requiredCharCount);
    MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), wideString.data(), static_cast<int>(wideString.size()));

    return wstring(wideString.data());
}

void SplitString(const wstring &s, wchar_t delim, vector<wstring> &tokens) {
    basic_stringstream<wchar_t> ss;
    ss.str(s);
    wstring item;
    while (getline<wchar_t>(ss, item, delim))
    {
        tokens.push_back(item);
    }
}

std::wstring GetCurrentDateTimeString()
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

void ReadXmlValue(IStream* resultSyncML, const std::wstring& targetXmlPath, std::wstring& value)
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

void ReadXmlValue(const std::wstring& resultSyncML, const std::wstring& targetXmlPath, std::wstring& value)
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
        throw exception();
    }

    vector<char> data(dataSize);
    if (ERROR_SUCCESS != RegGetValue(HKEY_LOCAL_MACHINE, subkey.c_str(), propName.c_str(), RRF_RT_REG_SZ, NULL, data.data(), &dataSize))
    {
        throw exception();
    }

    return wstring(reinterpret_cast<const wchar_t*>(data.data()));
}

}