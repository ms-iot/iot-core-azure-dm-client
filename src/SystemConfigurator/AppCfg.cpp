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
#include <windows.h>
#include <collection.h>
#include "AppCfg.h"
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"
#include "..\SharedUtilities\Impersonator.h"
#include "CSPs\CertificateInfo.h"
#include "CSPs\EnterpriseModernAppManagementCSP.h"

#define IotStartup L"C:\\windows\\system32\\iotstartup.exe"
#define StartCmd L" run "
#define StopCmd L" stop "

using namespace std;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::ApplicationModel;
using namespace Windows::Data::Json;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Management::Deployment;

void AppCfg::StartStopApp(const wstring& appId, bool start)
{
    TRACE(__FUNCTION__);

    unsigned long returnCode;
    string output;
    wstring cmd = IotStartup;
    if (start) { cmd += StartCmd; }
    else       { cmd += StopCmd; }
    cmd += appId;
    Utils::LaunchProcess(cmd, returnCode, output);
    if (returnCode != 0)
    {
        throw DMExceptionWithErrorCode("Error: iotstartup.exe returned an error code.", returnCode);
    }
}

Package^ AppCfg::FindApp(const wstring& packageFamilyName)
{
    TRACE(__FUNCTION__);
    TRACEP(L"Finding package: ", packageFamilyName.c_str());

    PackageManager^ packageManager = ref new PackageManager;
    IIterable<Package^>^ packages = packageManager->FindPackages();
    IIterator<Package^>^ it = packages->First();
    while (it->HasCurrent)
    {
        Package^ package = it->Current;

        TRACE("---------------------------------------------------------------------------");
        TRACEP("Name        : ", Utils::WideToMultibyte(package->Id->Name->Data()).c_str());
        TRACEP("DisplayName : ", Utils::WideToMultibyte(package->DisplayName->Data()).c_str());
        TRACEP("FullName    : ", Utils::WideToMultibyte(package->Id->FullName->Data()).c_str());
        TRACEP("FamilyName  : ", Utils::WideToMultibyte(package->Id->FamilyName->Data()).c_str());
        TRACEP("Name        : ", Utils::WideToMultibyte(package->Id->Name->Data()).c_str());
        TRACE("---------------------------------------------------------------------------");

        if (0 == _wcsicmp(packageFamilyName.c_str(), package->Id->FamilyName->Data()))
        {
            TRACE("Found.");
            return package;
        }

        it->MoveNext();
    }

    TRACE("Not found.");
    return nullptr;
}

ApplicationInfo AppCfg::GetAppInfo(Package^ package)
{
    TRACE(__FUNCTION__);
    TRACE("1x");
    assert(package);
    TRACE("2x");
    ApplicationInfo applicationInfo;
    wstring version = to_wstring(package->Id->Version.Major) + L"." +
                      to_wstring(package->Id->Version.Minor) + L"." +
                      to_wstring(package->Id->Version.Build) + L"." +
                      to_wstring(package->Id->Version.Revision);
    applicationInfo.packageFamilyName = package->Id->FamilyName->Data();
    applicationInfo.name = package->Id->Name->Data();
    applicationInfo.version = version;
    // ToDo: When accessing package->InstalledDate, "Element not found." exception is thrown.
    // applicationInfo.installDate = package->InstalledDate.ToString()->Data();
    return applicationInfo;
}

ApplicationInfo AppCfg::BuildOperationResult(const wstring& packageFamilyName, int errorCode, const wstring& errorMessage)
{
    TRACE(__FUNCTION__);

    ApplicationInfo applicationInfo(packageFamilyName);
    Package^ package = FindApp(packageFamilyName);

    if (package)
    {
        TRACE(L"Found the package!");

        applicationInfo = GetAppInfo(package);
    }
    else
    {
        TRACE(L"Could not find the package!");

        applicationInfo.packageFamilyName = packageFamilyName;
        applicationInfo.version = L"not installed";
    }

    if (errorCode != 0)
    {
        TRACE(L"An error occured!");
        TRACEP(L"Reported Error Message: ", errorMessage.c_str());
        TRACEP(L"Reported Error Code   : ", errorCode);

        applicationInfo.errorCode = errorCode;
        applicationInfo.errorMessage = errorMessage;
    }
    return applicationInfo;
}

ApplicationInfo AppCfg::InstallApp(const wstring& packageFamilyName, const wstring& appxLocalPath, const vector<wstring>& dependentPackages, const wstring& certFileName, const wstring& certStore)
{
    TRACE(__FUNCTION__);
    TRACEP(L"Installing app: ", packageFamilyName.c_str());

    HANDLE completedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!completedEvent)
    {
        TRACE("Error: failed to create synchronization event.");
        return ApplicationInfo(packageFamilyName, -1, L"Error: DM Internal error. Failed to create event.");
    }

    TRACEP(L"Installing certificate to ", certStore.c_str());
    CertificateFile certificateFile(certFileName);
    certificateFile.Install(certStore);

    int errorCode = 0;
    wstring errorMessage;

    try
    {
        TRACE("Installing appx and its dependencies...");

        Impersonator impersonator;
        impersonator.ImpersonateShellHost();

        Vector<Uri^>^ appxDepPkgs = ref new Vector<Uri^>();
        for (const wstring& depSource : dependentPackages)
        {
            appxDepPkgs->Append(ref new Uri(ref new String(depSource.c_str())));
        }

        Uri^ packageUri = ref new Uri(ref new Platform::String(appxLocalPath.c_str()));
        PackageManager^ packageManager = ref new PackageManager;
        auto installTask = packageManager->AddPackageAsync(packageUri, appxDepPkgs, DeploymentOptions::None);
        installTask->Completed = ref new AsyncOperationWithProgressCompletedHandler<DeploymentResult^, DeploymentProgress>(
            [&](IAsyncOperationWithProgress<DeploymentResult^, DeploymentProgress>^ operation, AsyncStatus)
            {
                TRACE("Firing 'install completed' event.");
                SetEvent(completedEvent);
            });

        TRACE("Waiting for installing to complete...");
        WaitForSingleObject(completedEvent, INFINITE);

        TRACE("Install task completed.");
        if (installTask->Status == AsyncStatus::Completed)
        {
            TRACE("Succeeded.");
        }
        else if (installTask->Status == AsyncStatus::Started)
        {
            // This should never happen...
            errorCode = -1;
            errorMessage = L"Error: failed to wait for installation to complete.";
        }
        else if (installTask->Status == AsyncStatus::Error)
        {
            auto deploymentResult = installTask->GetResults();
            errorCode = installTask->ErrorCode.Value;
            errorMessage = deploymentResult->ErrorText->Data();
        }
        else if (installTask->Status == AsyncStatus::Canceled)
        {
            errorCode = -1;
            errorMessage = L"Error: application installation task cancelled.";
        }
    }
    catch (...)
    {
        TRACE(L"Unknown exception while installing app!");

        errorCode = -1;
        errorMessage = L"Error: unknown failure while installing application.";
    }

    return BuildOperationResult(packageFamilyName, errorCode, errorMessage);
}

ApplicationInfo AppCfg::UninstallApp(const wstring& packageFamilyName)
{
    TRACE(__FUNCTION__);
    TRACEP(L"Uninstalling app: ", packageFamilyName.c_str());

    HANDLE completedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!completedEvent)
    {
        TRACE("Error: failed to create synchronization event.");
        return ApplicationInfo(packageFamilyName, -1, L"Error: DM Internal error. Failed to create event.");
    }

    int errorCode = 0;
    wstring errorMessage;

    try
    {
        TRACE("Uninstalling appx...");

        Impersonator impersonator;
        impersonator.ImpersonateShellHost();

        Package^ package = FindApp(packageFamilyName);
        if (!package)
        {
            TRACE("Warning: failed to find the specified package.");
            return ApplicationInfo(packageFamilyName, L"", L"not installed", L"", 0, L"");
        }

        PackageManager^ packageManager = ref new PackageManager;
        auto uninstallTask = packageManager->RemovePackageAsync(package->Id->FullName);
        uninstallTask->Completed = ref new AsyncOperationWithProgressCompletedHandler<DeploymentResult^, DeploymentProgress>(
            [&](IAsyncOperationWithProgress<DeploymentResult^, DeploymentProgress>^ operation, AsyncStatus)
            {
                TRACE("Firing 'uninstall completed' event.");
                SetEvent(completedEvent);
            });

        TRACE("Waiting for uninstalling to complete...");
        WaitForSingleObject(completedEvent, INFINITE);

        TRACE("Uninstall task completed.");
        if (uninstallTask->Status == AsyncStatus::Completed)
        {
            TRACE("Succeeded.");
        }
        else if (uninstallTask->Status == AsyncStatus::Started)
        {
            // This should never happen...
            errorCode = -1;
            errorMessage = L"Error: failed to wait for uninstallation to complete.";
        }
        else if (uninstallTask->Status == AsyncStatus::Error)
        {
            auto deploymentResult = uninstallTask->GetResults();
            errorCode = uninstallTask->ErrorCode.Value;
            errorMessage = deploymentResult->ErrorText->Data();
        }
        else if (uninstallTask->Status == AsyncStatus::Canceled)
        {
            errorCode = -1;
            errorMessage = L"Error: application uninstallation task cancelled.";
        }
    }
    catch (...)
    {
        errorCode = -1;
        errorMessage = L"Error: unknown failure while uninstalling application.";
    }

    return BuildOperationResult(packageFamilyName, errorCode, errorMessage);
}