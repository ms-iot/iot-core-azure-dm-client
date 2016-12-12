#include "stdafx.h"
#include "EnterpriseModernAppManagementCSP.h"
#include "RebootCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"

using namespace std;

// EnterpriseModernAppManagement CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904956(v=vs.85).aspx
//

wstring ReplaceAll(const wstring& str, const wstring& from, const wstring& to) 
{
    wstring wsRet(L"");
    if (!from.empty())
    {
        wsRet.reserve(str.length());
        size_t start_pos = 0, pos;
        while ((pos = str.find(from, start_pos)) != string::npos) {
            wsRet += str.substr(start_pos, pos - start_pos);
            wsRet += to;
            pos += from.length();
            start_pos = pos;
        }
        wsRet += str.substr(start_pos);
    }
    return wsRet;
}

wstring EnterpriseModernAppManagementCSP::GetInstalledApps()
{
    TRACE(L"\n---- Get Installed Apps\n");
    auto appsInfo = MdmProvision::RunGetStructData(L"./Device/Vendor/MSFT/EnterpriseModernAppManagement/AppManagement?list=StructData");
    wstring json(L"{");
    bool firstApp = true;
    for each(auto appInfo in appsInfo)
    {
        if (!firstApp) json.append(L", ");
        firstApp = false;

        json.append(L"\"");
        json.append(appInfo.first);
        json.append(L"\":{");
        bool firstProp = true;
        for each (auto propinfo in appInfo.second)
        {
            if (!firstProp) json.append(L", ");
            firstProp = false;

            json.append(L"\"");
            json.append(propinfo.first);
            json.append(L"\":");
            json.append(L"\"");
            json.append(ReplaceAll(propinfo.second, L"\\", L"\\\\"));
            json.append(L"\"");
        }
        json.append(L"}");
    }
    json.append(L"}");

    TRACEP(L"\n                app json:", json.c_str());
    return json;
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