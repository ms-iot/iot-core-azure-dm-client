#include "stdafx.h"
#include <wrl/client.h>
#include <xmllite.h>
#include <deque>
#include "XmlParser.h"
#include "Logger.h"

using namespace std;
using namespace Microsoft::WRL;

bool XmlParser::ReadXmlValue(const std::wstring& resultSyncML, const std::wstring& targetXmlPath, std::wstring& value)
{
    DWORD bufferSize = static_cast<DWORD>(resultSyncML.size() * sizeof(resultSyncML[0]));
    char* buffer = (char*)GlobalAlloc(GMEM_FIXED, bufferSize);
    memcpy(buffer, resultSyncML.c_str(), bufferSize);

    bool result = false;
    {
        ComPtr<IStream> dataStream;
        HRESULT hr = ::CreateStreamOnHGlobal(buffer, TRUE /*delete on release*/, dataStream.GetAddressOf());
        if (SUCCEEDED(hr))
        {
            result = ReadXmlValue(dataStream.Get(), targetXmlPath, value);
        }
    }

    // GlobalFree() is not needed since 'delete on release' is enabled.
    // GlobalFree(buffer);
    return result;
}

bool XmlParser::ReadXmlValue(IStream* resultSyncML, const std::wstring& targetXmlPath, std::wstring& value)
{
    ComPtr<IXmlReader> xmlReader;

    HRESULT hr = CreateXmlReader(__uuidof(IXmlReader), (void**)xmlReader.GetAddressOf(), NULL);
    if (FAILED(hr))
    {
        TRACEP(L"Error: Failed to create xml reader. Code :",  hr);
        return false;
    }

    hr = xmlReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
    if (FAILED(hr))
    {
        TRACEP(L"Error: XmlReaderProperty_DtdProcessing() failed. Code :\n",  hr);
        return false;
    }

    hr = xmlReader->SetInput(resultSyncML);
    if (FAILED(hr))
    {
        TRACEP(L"Error: SetInput() failed. Code :\n",  hr);
        return false;
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
                TRACEP(L"Error: GetPrefix() failed. Code :\n",  hr);
                return false;
            }

            const wchar_t* localName;
            hr = xmlReader->GetLocalName(&localName, NULL);
            if (FAILED(hr))
            {
                TRACEP(L"Error: GetLocalName() failed. Code :\n",  hr);
                return false;
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
                TRACEP(L"Error: GetPrefix() failed. Code :",  hr);
                return false;
            }

            const wchar_t* localName = NULL;
            hr = xmlReader->GetLocalName(&localName, NULL);
            if (FAILED(hr))
            {
                TRACEP(L"Error: GetLocalName() failed. Code :",  hr);
                return false;
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
                TRACEP(L"Error: GetValue() failed. Code :",  hr);
                return false;
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

    return pathFound;
}