#include "stdafx.h"
#include "WifiCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"

using namespace std;

// Wifi CSP docs
// https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/wifi-csp
//

void WifiCSP::AddProfile(const wstring& profileName, const wstring& profileXml)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/WlanXml";

    MdmProvision::RunAddData(path, profileXml);
}

void WifiCSP::DeleteProfile(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;

    MdmProvision::RunDelete(path);
}

wstring WifiCSP::GetProfile(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;

    wstring profileXml = MdmProvision::RunGetString(path);
    TRACEP(L" profile xml = ", profileXml.c_str());
    return profileXml;
}

void WifiCSP::SetProfile(const wstring& profileName, const wstring& profileXml)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/WlanXml";

    MdmProvision::RunSet(path, profileXml);
}

void WifiCSP::AddProxy(const wstring& profileName, const wstring& proxy)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/Proxy";

    MdmProvision::RunAddData(path, proxy);
}

void WifiCSP::DeleteProxy(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/Proxy";

    MdmProvision::RunDelete(path);
}

wstring WifiCSP::GetProxy(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/Proxy";

    wstring proxy = MdmProvision::RunGetString(path);
    TRACEP(L" proxy = ", proxy.c_str());
    return proxy;
}

void WifiCSP::SetProxy(const wstring& profileName, const wstring& proxy)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/Proxy";

    MdmProvision::RunSet(path, proxy);
}

void WifiCSP::AddDisableInternetConnectivityChecks(const wstring& profileName, bool disable)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/DisableInternetConnectivityChecks";

    MdmProvision::RunAddData(path, disable ? L"True" : L"False");
}

void WifiCSP::DeleteDisableInternetConnectivityChecks(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/DisableInternetConnectivityChecks";

    MdmProvision::RunDelete(path);
}

bool WifiCSP::GetDisableInternetConnectivityChecks(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/DisableInternetConnectivityChecks";

    wstring proxy = MdmProvision::RunGetString(path);
    TRACEP(L" proxy = ", proxy.c_str());
    return 0 == _wcsicmp(proxy.c_str(), L"True");
}

void WifiCSP::SetDisableInternetConnectivityChecks(const wstring& profileName, bool disable)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/DisableInternetConnectivityChecks";

    MdmProvision::RunSet(path, disable ? L"True" : L"False");
}

void AddProxyPacUrl(const wstring& profileName, const wstring& proxyPacUrl)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/ProxyPacUrl";

    MdmProvision::RunAddData(path, proxyPacUrl);
}

void WifiCSP::DeleteProxyPacUrl(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/ProxyPacUrl";

    MdmProvision::RunDelete(path);
}

wstring WifiCSP::GetProxyPacUrl(const wstring& profileName)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/ProxyPacUrl";

    wstring proxyPacUrl = MdmProvision::RunGetString(path);
    TRACEP(L" proxyPacUrl = ", proxyPacUrl.c_str());
    return proxyPacUrl;
}

void WifiCSP::SetProxyPacUrl(const wstring& profileName, const wstring& proxyPacUrl)
{
    TRACE(__FUNCTION__);

    wstring path = L"./Vendor/MSFT/WiFi/Profile/";
    path += profileName;
    path += L"/ProxyPacUrl";

    MdmProvision::RunSet(path, proxyPacUrl);
}
