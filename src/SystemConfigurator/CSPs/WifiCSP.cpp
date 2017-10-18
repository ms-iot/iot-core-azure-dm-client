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
#include "WifiCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"

using namespace Windows::Data::Json;
using namespace std;

#define WIFI_PROFILE_PATH L"./Vendor/MSFT/WiFi/Profile"

// Wifi CSP docs
// https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/wifi-csp
//

vector<wstring> WifiCSP::GetProfiles()
{
    TRACE(__FUNCTION__);

    vector<wstring> data;
    // use std::function to pass lambda that captures something
    std::function<void(std::vector<std::wstring>&, std::wstring&)> valueHandler =
        [&data](vector<wstring>& uriTokens, wstring&) {
            if (uriTokens.size() == 6)
            {
                // 0/__1____/_2_/__3__/__4___/______5______
                // ./Vendor/MSFT/WiFi/Profile/connector701
                data.push_back(uriTokens[5]);
            }
        };

    wstring path = WIFI_PROFILE_PATH L"?list=StructData";
    MdmProvision::RunGetStructData(path, valueHandler);

    return data;
}

void WifiCSP::AddProfile(const wstring& profileName, const wstring& profileXml)
{
    TRACE(__FUNCTION__);

    wstring path = WIFI_PROFILE_PATH L"/";
    path += profileName;
    path += L"/WlanXml";

    MdmProvision::RunAddData(path, profileXml);
}

void WifiCSP::DeleteProfile(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = WIFI_PROFILE_PATH L"/";
    path += profileName;

    MdmProvision::RunDelete(path);
}

wstring WifiCSP::GetProfile(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = WIFI_PROFILE_PATH L"/";
    path += profileName;
    path += L"/WlanXml";

    wstring profileXml = MdmProvision::RunGetString(path);
    TRACEP(L" profile xml = ", profileXml.c_str());
    return profileXml;
}

void WifiCSP::SetProfile(const wstring& profileName, const wstring& profileXml)
{
    TRACE(__FUNCTION__);

    wstring path = WIFI_PROFILE_PATH L"/";
    path += profileName;
    path += L"/WlanXml";

    MdmProvision::RunSet(path, profileXml);
}

