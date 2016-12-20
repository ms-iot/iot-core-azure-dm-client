#include "stdafx.h"
#include "EnterpriseModernAppManagementCSP.h"
#include "RebootCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"

using namespace std;

// EnterpriseModernAppManagement CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904956(v=vs.85).aspx
//

wstring EnterpriseModernAppManagementCSP::GetInstalledApps()
{
    TRACE(L"\n---- Get Installed Apps\n");
    auto data = ref new Windows::Data::Json::JsonObject();
    MdmProvision::RunGetStructData(
        L"./Device/Vendor/MSFT/EnterpriseModernAppManagement/AppManagement?list=StructData",
        data);
    return data->Stringify()->Data();
}

void EnterpriseModernAppManagementCSP::InstallApp(const std::wstring& packageFamilyName, const std::wstring& packageUri, const std::vector<std::wstring>& dependentPackages)
{
    const wchar_t* syncML = LR"(
<SyncBody>
    <!-- Add PackageFamilyName -->
    <Add>
       <CmdID>0</CmdID>
       <Item>
          <Target>
             <LocURI>./User/Vendor/MSFT/EnterpriseModernAppManagement/AppInstallation/%s</LocURI>
          </Target>
       </Item>
    </Add> 
    <!-- Install appx with deployment options and framework dependencies-->
    <Exec>
       <CmdID>1</CmdID>
       <Item>
          <Target>
             <LocURI>./User/Vendor/MSFT/EnterpriseModernAppManagement/AppInstallation/%s/HostedInstall</LocURI>
          </Target>
          <Meta>
             <Format xmlns="syncml:metinf">xml</Format>
          </Meta>
          <Data>
             <Application PackageUri="%s" DeploymentOptions="0" %s
          </Data>
       </Item>
    </Exec>
</SyncBody>
)";

    std::wstring applicationContent = L"";
    if (dependentPackages.size() != 0)
    {
        applicationContent += L"><Dependencies>";
        bool first = true;
        for (auto& s : dependentPackages)
        {
            if (!first) 
            { 
                applicationContent += L"\n"; 
            }
            first = false;
            applicationContent += L"<Dependency PackageUri=\"" + s;
            applicationContent += L"\"/>";
        }
        applicationContent += L"</Dependencies></Application>";
    }
    else
    {
        applicationContent += L"/>";
    }

    size_t bufsize = _scwprintf(syncML, packageFamilyName.c_str(), packageFamilyName.c_str(), packageUri.c_str(), applicationContent.c_str());

    bufsize += 1; // need null-termintator
    std::vector<wchar_t> buff(bufsize);

    _snwprintf_s(buff.data(), bufsize, bufsize, syncML, packageFamilyName.c_str(), packageFamilyName.c_str(), packageUri.c_str(), applicationContent.c_str());

    std::wstring output;
    std::wstring sid = Utils::GetSidForAccount(L"DefaultAccount");
    MdmProvision::RunSyncML(sid, buff.data(), output);
}

void EnterpriseModernAppManagementCSP::UninstallApp(const std::wstring& packageFamilyName, bool storeApp)
{
    const wchar_t* syncML = LR"(
<SyncBody>
    <!-- Uninstall App for a Package Family-->
    <Delete>
       <CmdID>1</CmdID>
       <Item>
          <Target>
             <LocURI>./User/Vendor/MSFT/EnterpriseModernAppManagement/AppManagement/%s/%s</LocURI>
          </Target>
       </Item>
    </Delete>
</SyncBody>
)";

    std::wstring appLocation = (storeApp) ? L"AppStore" : L"nonStore";
    size_t bufsize = _scwprintf(syncML, appLocation.c_str(), packageFamilyName.c_str());

    bufsize += 1; // need null-termintator
    std::vector<wchar_t> buff(bufsize);

    _snwprintf_s(buff.data(), bufsize, bufsize, syncML, appLocation.c_str(), packageFamilyName.c_str());

    std::wstring output;
    std::wstring sid = Utils::GetSidForAccount(L"DefaultAccount");
    MdmProvision::RunSyncML(sid, buff.data(), output);
}