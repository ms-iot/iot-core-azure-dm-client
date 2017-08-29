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
#include "CustomDeviceUiCSP.h"
#include "RebootCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"

using namespace std;
using namespace Windows::Data::Json;

// EnterpriseModernAppManagement CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904956(v=vs.85).aspx
//

wstring CustomDeviceUiCSP::GetStartupAppId()
{
    TRACE(__FUNCTION__);
#ifdef IOT_ENTERPRISE
    wstring appId = L"";
#else
    // REQUEST
    //    ./Vendor/MSFT/CustomDeviceUI/StartupAppID?list=StructData
    //
    // RESPONSE
    //    <Status><CmdID>2</CmdID><MsgRef>1</MsgRef><CmdRef>1</CmdRef><Cmd>Get</Cmd><Data>200</Data></Status>
    //    <Results>
    //        <CmdID>3</CmdID><MsgRef>1</MsgRef><CmdRef>1</CmdRef>
    //        <Item>
    //            <Source>
    //                <LocURI>./Vendor/MSFT/CustomDeviceUI/StartupAppID</LocURI>
    //            </Source>
    //            <Data>23983CETAthensQuality.IoTCoreSmartDisplay_7grdn1j1n8awe!App</Data>
    //        </Item>
    //    </Results>
        
    wstring sid = Utils::GetDmUserSid();
    auto appId = MdmProvision::RunGetString(
        sid.c_str(),
        L"./Vendor/MSFT/CustomDeviceUI/StartupAppID?list=StructData");

    appId = Utils::TrimString(appId, L"!App");
#endif // IOT_ENTERPRISE
    return appId;
}

wstring CustomDeviceUiCSP::GetBackgroundTasksToLaunch()
{
    TRACE(__FUNCTION__);
#ifdef IOT_ENTERPRISE
	auto data = ref new Windows::Data::Json::JsonArray(); 
#else
    // REQUEST
    //    ./Vendor/MSFT/CustomDeviceUI/BackgroundTaskstoLaunch?list=Struct
    // RESPONSE
    //    <Status><CmdID>2</CmdID><MsgRef>1</MsgRef><CmdRef>1</CmdRef><Cmd>Get</Cmd><Data>200</Data></Status>
    //    <Results><CmdID>3</CmdID><MsgRef>1</MsgRef><CmdRef>1</CmdRef>
    //        <Item>
    //            <Source>
    //                <LocURI>./Vendor/MSFT/CustomDeviceUI/BackgroundTaskstoLaunch</LocURI>
    //            </Source>
    //            <Meta><Format xmlns="syncml:metinf">node</Format></Meta>
    //        </Item>
    //        <Item>
    //            <Source>
    //                <LocURI>./Vendor/MSFT/CustomDeviceUI/BackgroundTaskstoLaunch/IoTOnboardingTask-uwp_1w720vyc4ccym!App</LocURI>
    //            </Source>
    //        </Item>
    //        <Item>
    //            <Source>
    //                <LocURI>./Vendor/MSFT/CustomDeviceUI/BackgroundTaskstoLaunch/IoTOnboardingTask-uwp_1w720vyc4ccym!App</LocURI>
    //            </Source>
    //        </Item>
    //    </Results>

    
    auto data = ref new Windows::Data::Json::JsonArray();
    // use std::function to pass lambda that captures something
    std::function<void(std::vector<std::wstring>&, std::wstring&)> valueHandler =
        [data](vector<wstring>& uriTokens, wstring& /*value*/) {
        if (uriTokens.size() == 6)
        {
            // 0/__1___/__2__/____3________/___________4___________/___5_
            // ./Vendor/MSFT/CustomDeviceUI/BackgroundTaskstoLaunch/Aumid
            wstring pkgFamilyName = Utils::TrimString(uriTokens[5], L"!App");
            auto aumid = ref new Platform::String(pkgFamilyName.c_str());
            data->Append(JsonValue::CreateStringValue(aumid));
        }
    };
    MdmProvision::RunGetStructData(
        L"./Vendor/MSFT/CustomDeviceUI/BackgroundTaskstoLaunch?list=Struct",
        valueHandler);
#endif // IOT_ENTERPRISE
    return data->Stringify()->Data();
}

void HandleStartupApp(const wstring& appId, bool backgroundApplication, bool add)
{
    TRACE(__FUNCTION__);
#ifdef IOT_ENTERPRISE
#else
    const wchar_t* syncML_forApp = LR"(
  <SyncBody>      
        <%s>
          <CmdID>1</CmdID>
          <Item>
            <Target>
              <LocURI>./Vendor/MSFT/CustomDeviceUI/StartupAppID</LocURI>
            </Target>       
             <Meta>
                <Format xmlns="syncml:metinf">chr</Format>
            </Meta>
            <Data>%s</Data>
        </Item>
        </%s>
     <Final/>
  </SyncBody>
)";
    const wchar_t* syncML_forBackgroundApp = LR"(
  <SyncBody>      
        <%s>
          <CmdID>1</CmdID>
          <Item>
            <Target>
              <LocURI>./Vendor/MSFT/CustomDeviceUI/BackgroundTaskstoLaunch/%s</LocURI>
            </Target>
            <Meta>
                <Format xmlns="syncml:metinf">chr</Format>
            </Meta>
            <Data>0</Data>
          </Item>
        </%s>        
     <Final/>
  </SyncBody>
)";
    if (!backgroundApplication && !add)
    {
        // there is no remove for foreground app ... only "Replace"
        throw DMException("./Vendor/MSFT/CustomDeviceUI/StartupAppID only accepts Get/Replace");
    }

    const wchar_t *action = (backgroundApplication) ? ((add) ? L"Add" : L"Delete") : L"Replace";
    const wchar_t *syncML = (backgroundApplication) ? syncML_forBackgroundApp : syncML_forApp;

    wstring cspAppId = appId + L"!App";
    size_t bufsize = _scwprintf(syncML, action, cspAppId.c_str(), action);

    bufsize += 1; // need null-termintator
    vector<wchar_t> buff(bufsize);

    _snwprintf_s(buff.data(), bufsize, bufsize, syncML, action, cspAppId.c_str(), action);

    wstring output;
    wstring sid = Utils::GetDmUserSid();
    MdmProvision::RunSyncML(sid, buff.data(), output);
#endif // IOT_ENTERPRISE
}

void CustomDeviceUiCSP::AddAsStartupApp(const wstring& appId, bool backgroundApplication)
{
    TRACE(__FUNCTION__);
#ifdef IOT_ENTERPRISE
#else
    HandleStartupApp(appId, backgroundApplication, true /*add*/);
#endif // IOT_ENTERPRISE
}

void CustomDeviceUiCSP::RemoveBackgroundApplicationAsStartupApp(const wstring& appId)
{
    TRACE(__FUNCTION__);
#ifdef IOT_ENTERPRISE
#else
    HandleStartupApp(appId, true, false /*replace/delete*/);
#endif // IOT_ENTERPRISE
}

bool CustomDeviceUiCSP::IsForeground(const std::wstring& pkgFamilyName)
{
    TRACE(__FUNCTION__);
#ifdef IOT_ENTERPRISE
    return false;
#else
    wstring forgroundAppId = CustomDeviceUiCSP::GetStartupAppId();
    return forgroundAppId == pkgFamilyName;
#endif // IOT_ENTERPRISE
}

bool CustomDeviceUiCSP::IsBackground(const std::wstring& pkgFamilyName)
{
    TRACE(__FUNCTION__);
#ifdef IOT_ENTERPRISE
    return false;
#else
    wstring backgroundTasks = CustomDeviceUiCSP::GetBackgroundTasksToLaunch();
    return wstring::npos != backgroundTasks.find(pkgFamilyName);
#endif // IOT_ENTERPRISE
}
