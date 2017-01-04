#include "stdafx.h"
#include "CustomDeviceUiCSP.h"
#include "RebootCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"

using namespace std;

// EnterpriseModernAppManagement CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904956(v=vs.85).aspx
//

wstring CustomDeviceUiCSP::GetStartupAppId()
{
    TRACE(__FUNCTION__);

    auto data = ref new Windows::Data::Json::JsonObject();
    MdmProvision::RunGetStructData(
        L"./Vendor/MSFT/CustomDeviceUI/StartupAppID?list=StructData",
        data);
    return data->Stringify()->Data();
}
wstring CustomDeviceUiCSP::GetBackgroundTasksToLaunch()
{
    TRACE(__FUNCTION__);

    auto data = ref new Windows::Data::Json::JsonObject();
    MdmProvision::RunGetStructData(
        L"./Vendor/MSFT/CustomDeviceUI/BackgroundTaskstoLaunch?list=Struct",
        data);
    return data->Stringify()->Data();
}
void HandleStartupApp(const wstring& appId, bool backgroundApplication, bool add)
{
    TRACE(__FUNCTION__);

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
        </Replace>        
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

    size_t bufsize = _scwprintf(syncML, action, appId.c_str(), action);

    bufsize += 1; // need null-termintator
    vector<wchar_t> buff(bufsize);

    _snwprintf_s(buff.data(), bufsize, bufsize, syncML, action, appId.c_str(), action);

    wstring output;
    wstring sid = Utils::GetSidForAccount(L"DefaultAccount");
    MdmProvision::RunSyncML(sid, buff.data(), output);

}
void CustomDeviceUiCSP::AddAsStartupApp(const wstring& appId, bool backgroundApplication)
{
    TRACE(__FUNCTION__);

    HandleStartupApp(appId, backgroundApplication, true);
}
void CustomDeviceUiCSP::RemoveBackgroundApplicationAsStartupApp(const wstring& appId)
{
    TRACE(__FUNCTION__);

    HandleStartupApp(appId, true, false);
}

