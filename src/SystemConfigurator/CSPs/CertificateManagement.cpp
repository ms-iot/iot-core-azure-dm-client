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
#include "CertificateManagement.h"
#include "MdmProvision.h"
#include "CertificateInfo.h"
#include "..\SharedUtilities\Logger.h"

using namespace std;

const wchar_t CspHashSeparator = L'/';
const wchar_t FileNameSeparator = L',';
const wchar_t CertificateSeparator = L',';
const wchar_t ConfigurationSeparator = L'|';
const wchar_t FilePartSeparator = L'\\';

const wchar_t* JsonStateUnknown = L"unknown";
const wchar_t* JsonStateInstalled = L"installed";
const wchar_t* JsonStateUninstalled = L"uninstalled";

bool icompare_pred(wchar_t a, wchar_t b)
{
    return ::towlower(a) == ::towlower(b);
}

bool icompare(std::wstring const& a, std::wstring const& b)
{
    if (a.length() == b.length())
    {
        return std::equal(b.begin(), b.end(), a.begin(), icompare_pred);
    }
    else
    {
        return false;
    }
}

void CertificateManagement::SyncCertificates(const std::wstring& path, const std::wstring& desiredStatesString)
{
    TRACE(__FUNCTION__);

    // Retrieve the current state
    wstring currentHashes = MdmProvision::RunGetString(path);

    vector<wstring> currentHashesVector;
    Utils::SplitString(currentHashes, CspHashSeparator, currentHashesVector);

    vector<wstring> certificateEntries;
    Utils::SplitString(desiredStatesString, CertificateSeparator, certificateEntries);

    // Loading desired certificates info...
    TRACE(L"Loading desired certificates info...");
    vector<CertificateFile> desiredInstalls;
    vector<wstring> desiredUninstalls;
    for (const wstring& certificateEntry : certificateEntries)
    {
        vector<wstring> certificateConfiguration;
        Utils::SplitString(certificateEntry, ConfigurationSeparator, certificateConfiguration);

        wstring desiredStateString = certificateConfiguration[0];
        TRACEP(L"Certificate Desired State: ", desiredStateString.c_str());

        if (desiredStateString == JsonStateInstalled)
        {
            TRACE(L"Certificate Desired State = Installed");

            wstring fileConfiguration = certificateConfiguration[1];
            TRACEP(L"Certificate File Configuration: ", fileConfiguration.c_str());

            vector<wstring> fileConfigurationParts;
            Utils::SplitString(fileConfiguration, FilePartSeparator, fileConfigurationParts);

            wstring fileName = fileConfigurationParts[1];
            TRACEP(L"Certificate File Name: ", fileName.c_str());

            wstring fullFileName = Utils::GetDmUserFolder() + L"\\" + fileName;
            TRACEP(L"Reading: ", fullFileName.c_str());
            desiredInstalls.push_back(CertificateFile(fullFileName));
        }
        else
        {
            TRACEP(L"Certificate Desired State = Uninstalled, ", certificateConfiguration[1].c_str());
            desiredUninstalls.push_back(certificateConfiguration[1]);
        }
    }

    // If in desired but not in current, add it.
    TRACE(L"Deciding what to add...");
    vector<CertificateFile> certificatesToAdd;
    for (const CertificateFile& certificateFileInfo : desiredInstalls)
    {
        wstring desiredHash = certificateFileInfo.ThumbPrint();
        bool found = false;
        for (const wstring& currentHash : currentHashesVector)
        {
            if (icompare(currentHash, desiredHash))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            TRACEP(L"-- Will be adding: ", certificateFileInfo.FullFileName().c_str());
            certificatesToAdd.push_back(certificateFileInfo);
        }
        else
        {
            TRACEP(L"-- Already installed: ", certificateFileInfo.FullFileName().c_str());
        }
    }

    // If in current but not in desired, remove it.
    TRACE(L"Deciding what to delete...");
    vector<wstring> hashesToDelete;
    for (const wstring& desiredUninstall : desiredUninstalls)
    {
        TRACEP(L"Looking for: ", desiredUninstall.c_str());
        bool found = false;
        for (const wstring& currentHash : currentHashesVector)
        {
            if (icompare(currentHash, desiredUninstall))
            {
                TRACEP(L"-- Found: ", currentHash.c_str());
                found = true;
                break;
            }
        }
        if (found)
        {
            TRACEP(L"-- Will be deleting: ", desiredUninstall.c_str());
            hashesToDelete.push_back(desiredUninstall);
        }
    }

    // Delete certificates
    for (const wstring& hashToDelete : hashesToDelete)
    {
        TRACEP(L"Deleting ", hashToDelete.c_str());
        CertificateInfo::DeleteCertificate(path, hashToDelete);
    }

    // Add certificates
    for (const CertificateFile& certificateToAdd : certificatesToAdd)
    {
        TRACEP(L"Adding: ", certificateToAdd.FullFileName().c_str());
        wstring certificateInBase64 = Utils::FileToBase64(certificateToAdd.FullFileName());
        CertificateInfo::AddCertificate(path, certificateToAdd.ThumbPrint(), certificateInBase64);
    }
}
