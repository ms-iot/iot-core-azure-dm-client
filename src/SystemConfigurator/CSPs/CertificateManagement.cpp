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
#include "..\DMMessage\DMGarbageCollectorTempFolder.h"

using namespace std;

void CertificateManagement::SyncCertificates(const std::wstring& path, const std::wstring& desiredCertificateFiles)
{
    TRACE(__FUNCTION__);

    // Retrieve the current state
    wstring currentHashes = MdmProvision::RunGetString(path);

    vector<wstring> currentHashesVector;
    Utils::SplitString(currentHashes, L'/', currentHashesVector);

    vector<wstring> desiredCertificateFilesVector;
    Utils::SplitString(desiredCertificateFiles, L'/', desiredCertificateFilesVector);

    // Loading desired certificates info...
    TRACE(L"Loading desired certificates info...");
    vector<CertificateFileInfo> desiredCertificates;
    for (const wstring& desiredCertificateFile : desiredCertificateFilesVector)
    {
        wstring fullFileName = wstring(SC_CLEANUP_FOLDER) + L"\\" + desiredCertificateFile;
        TRACEP(L"Reading: ", fullFileName.c_str());
        desiredCertificates.push_back(CertificateFileInfo(fullFileName));
    }

    // If in desired but not in current, add it.
    TRACE(L"Deciding what to add...");
    vector<CertificateFileInfo> certificatesToAdd;
    for (const CertificateFileInfo& certificateFileInfo : desiredCertificates)
    {
        wstring desiredHash = certificateFileInfo.ThumbPrint();
        if (currentHashesVector.end() == std::find(currentHashesVector.begin(), currentHashesVector.end(), desiredHash))
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
    for (const wstring& currentHash : currentHashesVector)
    {
        TRACEP(L"Looking for: ", currentHash.c_str());
        bool found = false;
        for (const CertificateFileInfo& certificateFileInfo : desiredCertificates)
        {
            if (currentHash == certificateFileInfo.ThumbPrint())
            {
                TRACEP(L"-- Found: ", currentHash.c_str());
                found = true;
                break;
            }
        }
        if (!found)
        {
            TRACEP(L"-- Will be deleting: ", currentHash.c_str());
            hashesToDelete.push_back(currentHash);
        }
    }

    // Delete certificates
    for (const wstring& hashToDelete : hashesToDelete)
    {
        TRACEP(L"Deleting ", hashToDelete.c_str());
        // ToDo: Too dangerous to enable right now.
        // CertificateInfo::DeleteCertificate(path, hashToDelete);
    }

    // Add certificates
    for (const CertificateFileInfo& certificateToAdd : certificatesToAdd)
    {
        TRACEP(L"Adding: ", certificateToAdd.FullFileName().c_str());
        wstring certificateInBase64 = Utils::FileToBase64(certificateToAdd.FullFileName());
        CertificateInfo::AddCertificate(path, certificateToAdd.ThumbPrint(), certificateInBase64);
    }
}
