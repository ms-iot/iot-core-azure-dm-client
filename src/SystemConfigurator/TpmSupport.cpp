#include "stdafx.h"
#include "..\SharedUtilities\Logger.h"
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


static std::string RunLimpet(int logicalId, const std::wstring& param)
{
    TRACE(__FUNCTION__);

    std::string output;

    // build limpet command and invoke it  
    wchar_t sys32dir[MAX_PATH];
    GetSystemDirectoryW(sys32dir, _countof(sys32dir));

    wchar_t fullCommand[MAX_PATH];
    swprintf_s(fullCommand, _countof(fullCommand), L"%s\\%s %u %s", sys32dir, L"limpet.exe", logicalId, param.c_str());

    unsigned long returnCode;

    Utils::LaunchProcess(fullCommand, returnCode, output);

    return output;
}

std::string GetServiceUrl(int logicalId)
{
    const std::string response = RunLimpet(logicalId, L"-rur");

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
    const std::string response = RunLimpet(logicalId, L"-ast");
    return response;
}
