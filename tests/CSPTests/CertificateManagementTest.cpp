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
#include <vector>
#include <iostream>
#include "..\..\src\SharedUtilities\DMException.h"
#include "..\..\src\SharedUtilities\Utils.h"
#include "..\..\src\SystemConfigurator\CSPs\MdmProvision.h"
#include "CertificateManagementTest.h"

using namespace std;

const wstring CertificateHash = L"6e7127c2d7c3d0aff188db3b386f63ecd98b8935";
const wstring CertificateEncoded = L"MIIDCzCCAfegAwIBAgIQc12Ffvo4SLhFl/f3Rcbk9zAJBgUrDgMCHQUAMBwxGjAY"
                                   L"BgNVBAMTEUdNaWxla2EgQ2VydCBOYW1lMB4XDTE2MTIxNDIxNTkyNloXDTM5MTIz"
                                   L"MTIzNTk1OVowHDEaMBgGA1UEAxMRR01pbGVrYSBDZXJ0IE5hbWUwggEiMA0GCSqG"
                                   L"SIb3DQEBAQUAA4IBDwAwggEKAoIBAQCuHbQjvmfUZcOIBNouwEDSFy9FDT58OAIN"
                                   L"OogYcT5fRAUtbNl4RNP3BFb3CAdzOdj/YUgVJN/iG5GtW66//NvJCHTXZYra0Lle"
                                   L"+6hrVv3qV1dra/1WShF72vS+iBntlnHwxBG4v+Q32OZHyS95edgCjBJUXwKQ67Cm"
                                   L"nPP66dMZdwRho/ZXXylUJFj3G7CO0eSJUDHnFvg4BJRkhino4/2uc25YJL0Mrkke"
                                   L"z87u+HNsType5UKLnCQ/V4hdmWuBb2TkeM5Kit4n0/munbzhiLfq9SB8WDrDz6un"
                                   L"aKVZXgVzGLNylIZTGdzszhOOSKfnxaKWwbkBK2rN9BfxSBwxUVipAgMBAAGjUTBP"
                                   L"ME0GA1UdAQRGMESAEBvxNS7lRcpo2wvpLLfgkKuhHjAcMRowGAYDVQQDExFHTWls"
                                   L"ZWthIENlcnQgTmFtZYIQc12Ffvo4SLhFl/f3Rcbk9zAJBgUrDgMCHQUAA4IBAQCL"
                                   L"PL1iM/0TX4fcfhPUhgmUU2qJOxICLM/8CgJhSiYYqIsiZdwpxu6SfMNCfw00r1vF"
                                   L"ukPlssdxktn3oyvPlcdQheCA00yv8Jwgy8vNMJViQ43p7LFuff0LbjyNS+sfTkJW"
                                   L"AU9nuuYN49E1NeNTkwoGkJj+Us/IZyIWi0EdgbKYmLhU/2X7RrULphlMdz/uTHh1"
                                   L"4FEIAVXUknnncdAphRrLLi5KGHJCi5FT/4Z1qsNCJPjGWHNymlboys6mV9YG65Fs"
                                   L"OWUb2zkdgT3i6ehgCfiQXlGPdQ06QjhCmLdKsg5dMJIyHnkYNPgaq+ZWtI3mQ+Xi"
                                   L"2XmN07XA9ygWkziNf2tG";

void CertificateManagementTest::AddCertificate(const wstring& cspPath)
{
    TRACE(__FUNCTION__);

    wstring path = cspPath + L"/" + CertificateHash + L"/EncodedCertificate";
    wcout << L"Adding certficate -> " << path << endl;

    MdmProvision::RunAddData(L"", path, CertificateEncoded);
}

void CertificateManagementTest::DeleteCertificate(const wstring& cspPath)
{
    TRACE(__FUNCTION__);

    wstring path = cspPath + L"/" + CertificateHash;
    wcout << L"Deleting certficate -> " << path << endl;

    MdmProvision::RunDelete(L"", path);
}

void CertificateManagementTest::ListCertificates(const wstring& cspPath, vector<wstring>& hashesVector)
{
    TRACE(__FUNCTION__);

    hashesVector.clear();
    wstring hashes = MdmProvision::RunGetString(cspPath);
    Utils::SplitString(hashes, L'/', hashesVector);
}

bool CertificateManagementTest::IsCertificatePresent(const wstring& cspPath)
{
    TRACE(__FUNCTION__);

    vector<wstring> hashesVector;
    ListCertificates(cspPath, hashesVector);

    bool found = false;
    for (const wstring& hash : hashesVector)
    {
        if (0 == _wcsicmp(hash.c_str(), CertificateHash.c_str()))
        {
            found = true;
            break;
        }
    }
    return found;
}

bool CertificateManagementTest::RunTest()
{
    bool result = true;
    try
    {
        vector<wstring> cspPaths;
        cspPaths.push_back(L"./Device/Vendor/MSFT/RootCATrustedCertificates/Root");
        cspPaths.push_back(L"./Device/Vendor/MSFT/RootCATrustedCertificates/CA");
        cspPaths.push_back(L"./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPublisher");
        cspPaths.push_back(L"./Device/Vendor/MSFT/RootCATrustedCertificates/TrustedPeople");
        cspPaths.push_back(L"./Vendor/MSFT/CertificateStore/CA/System");
        cspPaths.push_back(L"./Vendor/MSFT/CertificateStore/Root/System");
        cspPaths.push_back(L"./Vendor/MSFT/CertificateStore/My/User");
        cspPaths.push_back(L"./Vendor/MSFT/CertificateStore/My/System");

        for (const wstring& cspPath : cspPaths)
        {
            TRACEP(L"CSP Path = ", cspPath.c_str());

            // Add certificate...
            AddCertificate(cspPath);

            // Verify certificate has been added...
            if (!IsCertificatePresent(cspPath))
            {
                throw exception("cannot find the certificate that was just added.");
            }

            // Delete certificate...
            DeleteCertificate(cspPath);

            // Verify certificate has been deleted...
            if (IsCertificatePresent(cspPath))
            {
                throw exception("the certificate was not removed.");
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
