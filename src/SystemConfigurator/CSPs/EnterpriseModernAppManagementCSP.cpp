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
#include "EnterpriseModernAppManagementCSP.h"
#include "RebootCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"
#include "..\CSPs\CertificateInfo.h"

using namespace std;
using namespace Windows::Data::Json;

// EnterpriseModernAppManagement CSP docs
// https://msdn.microsoft.com/en-us/library/windows/hardware/dn904956(v=vs.85).aspx
//

std::wstring EnterpriseModernAppManagementCSP::AppSourceTypeToString(AppSourceType appSourceType)
{
    wstring s = L"";
    switch (appSourceType)
    {
    case System:
        s = L"System";
        break;
    case NonStore:
        s = L"NonStore";
        break;
    case Store:
        s = L"Store";
        break;
    default:
        assert(false);
    }
    return s;
}

ApplicationInfo EnterpriseModernAppManagementCSP::GetInstalledAppInfo(const std::wstring& packageFamilyName, AppSourceType appSourceType)
{
    TRACE(__FUNCTION__);

    ApplicationInfo applicationInfo(packageFamilyName);

    wstring path;
    path += L"./Device/Vendor/MSFT/EnterpriseModernAppManagement/AppManagement/";
    path += AppSourceTypeToString(appSourceType);
    path += L"/";
    path += packageFamilyName;
    path += L"?list=StructData";

    std::function<void(std::vector<std::wstring>&, std::wstring&)> valueHandler =
        [&applicationInfo](vector<wstring>& uriTokens, wstring& value) {
        if (uriTokens.size() == 10)
        {
            // 0/__1___/__2___/__3_/______________4______________/______5______/___6____/_________7_______/_______8_______/______9_____
            // ./Device/Vendor/MSFT/EnterpriseModernAppManagement/AppManagement/AppStore/PackageFamilyName/PackageFullName/PropertyName
            if (uriTokens[9] == L"Name")
            {
                applicationInfo.name = value;
            }
            else if (uriTokens[9] == L"InstallDate")
            {
                applicationInfo.installDate = value;
            }
            else if (uriTokens[9] == L"Version")
            {
                applicationInfo.version = value;
            }
        }
    };

    MdmProvision::RunGetStructData(path, valueHandler);

    return applicationInfo;
}

wstring EnterpriseModernAppManagementCSP::GetInstalledApps()
{
    TRACE(__FUNCTION__);

    auto data = ref new JsonObject();
    // use std::function to pass lambda that captures something
    std::function<void(std::vector<std::wstring>&, std::wstring&)> valueHandler =
        [data](vector<wstring>& uriTokens, wstring& value) {
        if (uriTokens.size() == 10)
        {
            // 0/__1___/__2___/__3_/______________4______________/______5______/___6____/_________7_______/_______8_______/______9_____
            // ./Device/Vendor/MSFT/EnterpriseModernAppManagement/AppManagement/AppStore/PackageFamilyName/PackageFullName/PropertyName
            auto pfn = ref new Platform::String(uriTokens[8].c_str());
            if (!data->HasKey(pfn))
            {
                auto propMap = ref new JsonObject();
                propMap->Insert(ref new Platform::String(uriTokens[9].c_str()), JsonValue::CreateStringValue(ref new Platform::String(value.c_str())));
                propMap->Insert(ref new Platform::String(L"AppSource"), JsonValue::CreateStringValue(ref new Platform::String(uriTokens[6].c_str())));
                propMap->Insert(ref new Platform::String(L"PackageFamilyName"), JsonValue::CreateStringValue(ref new Platform::String(uriTokens[7].c_str())));

                data->Insert(pfn, propMap);
            }
            else
            {
                data->GetNamedObject(pfn)->Insert(ref new Platform::String(uriTokens[9].c_str()), JsonValue::CreateStringValue(ref new Platform::String(value.c_str())));
            }
        }
    };

    MdmProvision::RunGetStructData(
        L"./Device/Vendor/MSFT/EnterpriseModernAppManagement/AppManagement?list=StructData",
        valueHandler);
    return data->Stringify()->Data();
}

ApplicationInfo EnterpriseModernAppManagementCSP::InstallApp(const std::wstring& packageFamilyName, const std::wstring& appxLocalPath, const std::vector<std::wstring>& dependentPackages)
{
    TRACE(__FUNCTION__);

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

    size_t bufsize = _scwprintf(syncML, packageFamilyName.c_str(), packageFamilyName.c_str(), appxLocalPath.c_str(), applicationContent.c_str());

    bufsize += 1; // need null-termintator
    std::vector<wchar_t> buff(bufsize);

    _snwprintf_s(buff.data(), bufsize, bufsize, syncML, packageFamilyName.c_str(), packageFamilyName.c_str(), appxLocalPath.c_str(), applicationContent.c_str());

    std::wstring output;
    std::wstring sid = Utils::GetSidForAccount(L"DefaultAccount");
    MdmProvision::RunSyncML(sid, buff.data(), output);

    ApplicationInfo applicationInfo = GetInstalledAppInfo(packageFamilyName, AppSourceType::NonStore);
    TRACE(L"-------------------------------------------------------------------");
    TRACEP(L"Name             : ", applicationInfo.name.c_str());
    TRACEP(L"PackageFamilyName: ", applicationInfo.packageFamilyName.c_str());
    TRACEP(L"Version          : ", applicationInfo.version.c_str());
    TRACEP(L"InstallDate      : ", applicationInfo.installDate.c_str());
    TRACE(L"-------------------------------------------------------------------");

    return applicationInfo;
}

void EnterpriseModernAppManagementCSP::UninstallApp(const std::wstring& packageFamilyName, bool storeApp)
{
    TRACE(__FUNCTION__);

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

    std::wstring appLocation = (storeApp) ? L"AppStore" : L"NonStore";
    size_t bufsize = _scwprintf(syncML, appLocation.c_str(), packageFamilyName.c_str());

    bufsize += 1; // need null-termintator
    std::vector<wchar_t> buff(bufsize);

    _snwprintf_s(buff.data(), bufsize, bufsize, syncML, appLocation.c_str(), packageFamilyName.c_str());

    std::wstring output;
    std::wstring sid = Utils::GetSidForAccount(L"DefaultAccount");
    MdmProvision::RunSyncML(sid, buff.data(), output);
}
