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
#pragma once

#include <string>

class WifiCSP
{
public:
    static void AddProfile(const std::wstring& profileName, const std::wstring& profileXml);
    static void DeleteProfile(const std::wstring& profileName);
    static std::wstring GetProfile(const std::wstring& profileName);
    static void SetProfile(const std::wstring& profileName, const std::wstring& profileXml);

    static void AddDisableInternetConnectivityChecks(const std::wstring& profileName, bool disable);
    static void DeleteDisableInternetConnectivityChecks(const std::wstring& profileName);
    static bool GetDisableInternetConnectivityChecks(const std::wstring& profileName);
    static void SetDisableInternetConnectivityChecks(const std::wstring& profileName, bool disable);

	// Proxy
	// ProxyPacUrl
	// ProxyWPAD
	// From MSDN: This configurations are only supported in Windows 10 Mobile.
    // https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/wifi-csp
};
