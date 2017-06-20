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
#include <string>
#include <algorithm>
#include <regex>
#include <vector>
#include <iostream>
#include "..\..\src\SharedUtilities\DMException.h"
#include "..\..\src\SharedUtilities\Utils.h"
#include "..\..\src\SystemConfigurator\CSPs\MdmProvision.h"
#include "WifiManagementTest.h"

using namespace std;

#define WIFI_PROFILE_PATH L"./Vendor/MSFT/WiFi/Profile"
const wstring WifiProfileName = L"connector623";
const wstring WifiProfile = L"<?xml version=\"1.0\"?>\n\
<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n\
        <name>connector623</name>\n\
        <SSIDConfig>\n\
                <SSID>\n\
                        <hex>636F6E6E6563746F72363233</hex>\n\
                        <name>connector623</name>\n\
                </SSID>\n\
        </SSIDConfig>\n\
        <connectionType>ESS</connectionType>\n\
        <connectionMode>auto</connectionMode>\n\
        <MSM>\n\
                <security>\n\
                        <authEncryption>\n\
                                <authentication>WPAPSK</authentication>\n\
                                <encryption>TKIP</encryption>\n\
                                <useOneX>false</useOneX>\n\
                        </authEncryption>\n\
                        <sharedKey>\n\
                                <keyType>passPhrase</keyType>\n\
                                <protected>true</protected>\n\
                                <keyMaterial>01000000D08C9DDF0115D1118C7A00C04FC297EB01000000385CDD5FBB257743ABC5ADA7051D864800000000020000000000106600000001000020000000C116D829FBFACB1E8567BB35B02BAABD332AFC194D7D807119A31093C02C2820000000000E8000000002000020000000FEF7DB03A88EA9870B0613BCC2A225D9F993DECEA23E30AB9A0809BFE4C2556310000000EDD7777757C2C2639E14A5329FAAEA194000000023C5A9D20E7917E98BF82D4EB1C16D7BFA3198254476A02E4669DD90599936C1655EBB6A2A37EEA224E1DCC10EB1355C5D643F473CEE3C39505A91C53534487A</keyMaterial>\n\
                        </sharedKey>\n\
                </security>\n\
        </MSM>\n\
        <MacRandomization xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v3\">\n\
                <enableRandomization>false</enableRandomization>\n\
        </MacRandomization>\n\
</WLANProfile>\n";

const wstring EscapedWifiProfileXml = L"&lt;?xml version=\"1.0\"?&gt;\
&lt;WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\"&gt;\
 &lt;name&gt;connector623&lt;/name&gt;\
 &lt;SSIDConfig&gt;\
  &lt;SSID&gt;\
   &lt;hex&gt;636F6E6E6563746F72363233&lt;/hex&gt;\
   &lt;name&gt;connector623&lt;/name&gt;\
  &lt;/SSID&gt;\
 &lt;/SSIDConfig&gt;\
 &lt;connectionType&gt;ESS&lt;/connectionType&gt;\
 &lt;connectionMode&gt;auto&lt;/connectionMode&gt;\
 &lt;MSM&gt;\
  &lt;security&gt;\
   &lt;authEncryption&gt;\
    &lt;authentication&gt;WPAPSK&lt;/authentication&gt;\
    &lt;encryption&gt;TKIP&lt;/encryption&gt;\
    &lt;useOneX&gt;false&lt;/useOneX&gt;\
   &lt;/authEncryption&gt;\
   &lt;sharedKey&gt;\
    &lt;keyType&gt;passPhrase&lt;/keyType&gt;\
    &lt;protected&gt;false&lt;/protected&gt;\
    &lt;keyMaterial&gt;connector&lt;/keyMaterial&gt;\
   &lt;/sharedKey&gt;\
  &lt;/security&gt;\
 &lt;/MSM&gt;\
 &lt;MacRandomization xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v3\"&gt;\
  &lt;enableRandomization&gt;false&lt;/enableRandomization&gt;\
 &lt;/MacRandomization&gt;\
&lt;/WLANProfile&gt;";

vector<wstring> WifiManagementTest::GetProfiles()
{
    TRACE(__FUNCTION__);

    vector<wstring> data;
    // use std::function to pass lambda that captures something
    std::function<void(std::vector<std::wstring>&, std::wstring&)> valueHandler =
        [&data](vector<wstring>& uriTokens, wstring& value) {
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

void WifiManagementTest::AddWifiProfile()
{
    TRACE(__FUNCTION__);

    wstring path = WIFI_PROFILE_PATH L"/";
    path += WifiProfileName;
    path += L"/WlanXml";

    MdmProvision::RunAddData(path, EscapedWifiProfileXml);
}

void WifiManagementTest::DeleteWifiProfile()
{
    TRACE(__FUNCTION__);

    wstring path = WIFI_PROFILE_PATH L"/";
    path += WifiProfileName;

    MdmProvision::RunDelete(path);
}

wstring WifiManagementTest::GetProfileDetails()
{
    TRACE(__FUNCTION__);

    wstring path = WIFI_PROFILE_PATH L"/";
    path += WifiProfileName;
    path += L"/WlanXml";

    wstring profileXml = MdmProvision::RunGetString(path);
    TRACEP(L" profile xml = ", profileXml.c_str());
    return profileXml;
}

wstring StripProfileXml(wstring xml)
{
    auto strippedXml = xml;
    // clear spaces
    strippedXml.erase(remove_if(strippedXml.begin(), strippedXml.end(), ::isspace), strippedXml.end());
    // clear keyMaterial (it changes)
    auto startKeyMaterial = strippedXml.find(L"<keyMaterial>");
    auto endKeyMaterial = strippedXml.find(L"</keyMaterial>");

    strippedXml.erase(startKeyMaterial, endKeyMaterial);
    return strippedXml;
}

bool WifiManagementTest::RunTest()
{
    bool result = true;
    try
    {
        {
            // Verify profile is not present...
            vector<wstring> profiles = GetProfiles();
            if (std::find(profiles.begin(), profiles.end(), WifiProfileName) != profiles.end())
            {
                throw exception("test profile is already present.");
            }
        }

        {
            // Add certificate...
            AddWifiProfile();
            // Verify profile has been added...
            vector<wstring> profiles = GetProfiles();
            if (std::find(profiles.begin(), profiles.end(), WifiProfileName) == profiles.end())
            {
                throw exception("cannot find the profile that was just added.");
            }
        }

        {
            // Verify profile details...
            wstring expectedDetails = StripProfileXml(WifiProfile);
            wstring details = StripProfileXml(GetProfileDetails());
            if (details != expectedDetails)
            {
                TRACEP(L"expected [", expectedDetails.c_str());
                TRACE(L"]");
                TRACEP(L"actual [", details.c_str());
                TRACE(L"]");
                throw exception("profile details do not match.");
            }
        }

        {
            // Delete certificate...
            DeleteWifiProfile();
            // Verify profile is not present...
            vector<wstring> profiles = GetProfiles();
            if (std::find(profiles.begin(), profiles.end(), WifiProfileName) != profiles.end())
            {
                throw exception("profile was not removed.");
            }
        }
    }
    catch (DMException& e)
    {
        TRACEP("Error: ", e.what());

        cout << "Error: " << e.what() << endl;
        result = false;
    }
    catch (exception e)
    {
        TRACEP("Error: ", e.what());

        cout << "Error: " << e.what() << endl;
        result = false;
    }

    return result;
}
