#pragma once

#include <string>

class WifiCSP
{
public:
    static void AddProfile(const std::wstring& profileName, const std::wstring& profileXml);
    static void DeleteProfile(const std::wstring& profileName);
    static std::wstring GetProfile(const std::wstring& profileName);
    static void SetProfile(const std::wstring& profileName, const std::wstring& profileXml);

    static void AddProxy(const std::wstring& profileName, const std::wstring& proxy);
    static void DeleteProxy(const std::wstring& profileName);
    static std::wstring GetProxy(const std::wstring& profileName);
    static void SetProxy(const std::wstring& profileName, const std::wstring& proxy);

    static void AddDisableInternetConnectivityChecks(const std::wstring& profileName, bool disable);
    static void DeleteDisableInternetConnectivityChecks(const std::wstring& profileName);
    static bool GetDisableInternetConnectivityChecks(const std::wstring& profileName);
    static void SetDisableInternetConnectivityChecks(const std::wstring& profileName, bool disable);

    static void AddProxyPacUrl(const std::wstring& profileName, const std::wstring& proxyPacUrl);
    static void DeleteProxyPacUrl(const std::wstring& profileName);
    static std::wstring GetProxyPacUrl(const std::wstring& profileName);
    static void SetProxyPacUrl(const std::wstring& profileName, const std::wstring& proxyPacUrl);

    // ProxyWPAD
    // From MSDN: This proxy configuration is only supported in Windows 10 Mobile.
    // https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/wifi-csp
};