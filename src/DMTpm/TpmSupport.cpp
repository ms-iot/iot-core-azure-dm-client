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
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\Utils.h"
#include "DMException.h"

using namespace std;

// Example -ast output:
//
// HostName=iot-open-house-demo.azure-devices.net;DeviceId=device0;SharedAccessSignature=SharedAccessSignature sr=iot-open-house-demo.azure-devices.net/devices/device0&sig=dPmolG%2foBeyrrO1pt6dCQr853a6%2fc6xxWYth%2fUptFyg%3d&se=1486606191

// Example -rur output:
//
//<?xml version="1.0" encoding="utf-8"?>
//<ServiceURI>
//  iot-open-house-demo.azure-devices.net/device0
//</ServiceURI>


static std::string RunLimpet(const std::wstring& params)
{
    TRACE(__FUNCTION__);

    std::string output;

    // build limpet command and invoke it  
    wchar_t sys32dir[MAX_PATH];
    GetSystemDirectoryW(sys32dir, _countof(sys32dir));

    wchar_t fullCommand[MAX_PATH];
    swprintf_s(fullCommand, _countof(fullCommand), L"%s\\%s %s", sys32dir, L"limpet.exe", params.c_str());

    unsigned long returnCode;

    Utils::LaunchProcess(fullCommand, returnCode, output);

    return output;
}

std::string GetServiceUrl(int logicalId)
{
    TRACE(__FUNCTION__);

    const std::string response = RunLimpet(to_wstring(logicalId) + L" -rur");

    std::regex rgx(".*<ServiceURI>\\s*(\\S+)\\s*</ServiceURI>.*");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx))
    {
        auto m = match[1];
        return m.str();
    }
    auto responseW = Utils::MultibyteToWide(response.c_str());
    TRACEP(L"Unexpected response from Limpet:", responseW.c_str());
    throw DMException("cannot parse Limpet response. Is TPM supported?");
}

std::string GetSASToken(int logicalId)
{
    TRACE(__FUNCTION__);

    const std::string response = RunLimpet(to_wstring(logicalId) + L" -ast");

    // There is a bug in Limpet that produces the entire connection string and not only the SAS token
    // Work around by extracting the actual connection string
    // The workaround will continue to work (but will be unnecessary) once the bug in Limpet is fixed

    std::regex rgx(".*(SharedAccessSignature sr.*)");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx))
    {
        auto m = match[1];
        return m.str();
    }
    auto responseW = Utils::MultibyteToWide(response.c_str());
    TRACEP(L"Unexpected response from Limpet:", responseW.c_str());
    throw DMException("cannot parse Limpet response. Is TPM supported?");
}

void ClearTPM()
{
    TRACE(__FUNCTION__);

    const std::string response = RunLimpet(L"-fct");

    std::regex rgx(".*<TpmClear>\\s*(\\S+)\\s*</TpmClear>.*");
    std::smatch match;

    if (std::regex_search(response.begin(), response.end(), match, rgx))
    {
        auto m = match[1].str();
        string result = Utils::TrimString(m, string(" \t\n\r"));
        if (0 == _stricmp(result.c_str(), "ok"))
        {
            TRACE(L"ClearTPM completed successfully.");
            return;
        }
    }
    auto responseW = Utils::MultibyteToWide(response.c_str());
    TRACEP(L"Unexpected response from Limpet:", responseW.c_str());
    throw DMException("cannot parse Limpet response. Is TPM supported?");
}

std::string GetEndorsementKey()
{
	TRACE(__FUNCTION__);

	const std::string response = RunLimpet(L"-erk");
	std::regex rgx(".*<ERKPub>\\s*(\\S+)\\s*</ERKPub>.*");
	std::smatch match;

	if (std::regex_search(response.begin(), response.end(), match, rgx))
	{
		auto m = match[1].str();
		string result = Utils::TrimString(m, string(" \t\n\r"));
		TRACE(L"GetEndorsementKey completed successfully.");
		return result;
	}
	throw DMException("cannot parse Limpet response. Is TPM supported?");
}

std::string GetSRootKey()
{
	TRACE(__FUNCTION__);

	const std::string response = RunLimpet(L"-srk");
	std::regex rgx(".*<SRKPub>\\s*(\\S+)\\s*</SRKPub>.*");
	std::smatch match;

	if (std::regex_search(response.begin(), response.end(), match, rgx))
	{
		auto m = match[1].str();
		string result = Utils::TrimString(m, string(" \t\n\r"));
		TRACE(L"GetSRootKey completed successfully.");
		return result;
	}
	throw DMException("cannot parse Limpet response. Is TPM supported?");
}

void DestroyServiceUrl(int logicalId)
{
	TRACE(__FUNCTION__);

	RunLimpet(to_wstring(logicalId) + L" -dur");
}

void StoreServiceUrl(int logicalId, const std::string& url)
{
	TRACE(__FUNCTION__);

	RunLimpet(to_wstring(logicalId) + (std::wstring)L" -sur " + Utils::MultibyteToWide(url.c_str()));
}

void ImportSymetricIdentity(int logicalId, const std::string& hostageFile)
{
	TRACE(__FUNCTION__);

	RunLimpet(to_wstring(logicalId) + L" -isi " + Utils::MultibyteToWide(hostageFile.c_str()));
}

void EvictHmacKey(int logicalId)
{
	TRACE(__FUNCTION__);

	RunLimpet(to_wstring(logicalId) + L" -ehk");
}

