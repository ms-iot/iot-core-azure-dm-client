#include "stdafx.h"
#include "EnterpriseModernAppManagementCSP.h"
#include "RebootCSP.h"
#include "MdmProvision.h"

using namespace std;

// EnterpriseModernAppManagement CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904956(v=vs.85).aspx
//

std::vector<std::wstring> EnterpriseModernAppManagementCSP::GetInstalledApps()
{
    std::wstring apps = MdmProvision::RunGetString(L"./Device/Vendor/MSFT/EnterpriseModernAppManagement/AppManagement?list=StructData");

    // This is a hack: we need to figure out how to parse into multiple objects
    std::vector<std::wstring> result = { apps };
    return result;
}

// TODO: work in progress!
void EnterpriseModernAppManagementCSP::InstallApp(const std::wstring& packageFamilyName, const std::wstring& packageUri, const std::vector<std::wstring>& dependentPackages)
{
    const wchar_t* syncML = LR"(
<!-- Add PackageFamilyName -->
<Add>
   <CmdID>0</CmdID>
   <Item>
      <Target>
         <LocURI>./User/Vendor/MSFT/EnterpriseModernAppManagement/AppInstallation/{%s}</LocURI>
      </Target>
   </Item>
</Add> 
<!-- Install appx with deployment options and framework dependencies-->
<Exec>
   <CmdID>1</CmdID>
   <Item>
      <Target>
         <LocURI>./User/Vendor/MSFT/EnterpriseModernAppManagement/AppInstallation/{%s}/HostedInstall</LocURI>
      </Target>
      <Meta>
         <Format xmlns="syncml:metinf">xml</Format>
      </Meta>
      <Data>
         <Application PackageUri="%s" DeploymentOptions="0" >
            <Dependencies>
                %s
            </Dependencies>
        </Application>
      </Data>
   </Item>
</Exec>
)";

    std::wstring dependencies = L"";
    for (auto& s : dependentPackages)
    {
        dependencies += L"<Dependency PackageUri=\"" + s;
        dependencies += L"\"/>";
        dependencies += L"\n";
    }

    size_t bufsize = _scwprintf(syncML, packageFamilyName.c_str(), packageFamilyName.c_str(), packageUri.c_str(), dependencies.c_str());

    bufsize += 1; // need null-termintator
    std::vector<wchar_t> buff(bufsize);

    _snwprintf_s(buff.data(), bufsize, bufsize, syncML, packageFamilyName.c_str(), packageFamilyName.c_str(), packageUri.c_str(), dependencies.c_str());

    std::wstring output;

    std::wstring sid = L"";
    MdmProvision::RunSyncML(sid, buff.data(), output);
}

// TODO: work in progress!
void EnterpriseModernAppManagementCSP::UninstallApp(const std::wstring& packageFamilyName)
{
    const wchar_t* syncML = LR"(
<!-- Uninstall App for a Package Family-->
<Delete>
   <CmdID>1</CmdID>
   <Item>
      <Target>
         <LocURI>./User/Vendor/MSFT/EnterpriseModernAppManagement/AppManagement/AppStore/{%s}</LocURI>
      </Target>
   </Item>
</Delete>
)";

    size_t bufsize = _scwprintf(syncML, packageFamilyName.c_str());

    bufsize += 1; // need null-termintator
    std::vector<wchar_t> buff(bufsize);

    _snwprintf_s(buff.data(), bufsize, bufsize, syncML, packageFamilyName.c_str());

    std::wstring output;

    std::wstring sid = L"";
    MdmProvision::RunSyncML(sid, buff.data(), output);
}