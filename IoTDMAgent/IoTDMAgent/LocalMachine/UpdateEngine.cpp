#include "stdafx.h"
#include <windows.h>
#include <filesystem>
#include "UpdateEngine.h"

using namespace std;
using namespace tr2::sys;

#define StagingFolder L"C:\\Data\\ProgramData\\USOShared\\Data"

void UpdateEngine::Scan()
{
    TRACE("UpdateEngine::Scan()");

    lock_guard<mutex> lock(_mutex);

    _packageList.clear();

    // build the command line
    wchar_t systemRoot[MAX_PATH] = { 0 };
    wchar_t cmdLine[MAX_PATH] = { 0 };
    if (0 != GetEnvironmentVariable(L"SystemRoot", systemRoot, ARRAYSIZE(systemRoot)))
    {
        swprintf_s(cmdLine, MAX_PATH, L"%s\\servicing\\UpdateApp.exe getinstalledpackages", systemRoot);
    }

    // ToDo: Replace UpdateApp.exe with ApplyUpdate.exe when it starts supporting getinstalledpackages.
    //       UpdateApp.exe is a test app and its output is not guaranteed to stay the same.
    unsigned long returnCode = 0;
    string output;
    Utils::LaunchProcess(cmdLine, returnCode, output);

    vector<string> lines;
    Utils::SplitString(output, '\n', lines);

    for (const string& line : lines)
    {
        vector<string> tokens;
        Utils::SplitString(line, ',', tokens);

        if (tokens.size() == 3)
        {
            PackageData packageData;

            unsigned int tokenIndex = 0;
            for (const string& token : tokens)
            {
                wstring trimmedToken = Utils::MultibyteToWide(Utils::TrimString<string>(token, " \t\n\r").c_str());

                switch (tokenIndex)
                {
                case 0:
                    packageData.id = trimmedToken;
                    break;
                case 1:
                    packageData.type = trimmedToken;
                    break;
                case 2:
                    packageData.version = trimmedToken;
                    break;
                }
                ++tokenIndex;
            }
            _packageList.push_back(packageData);
        }
    }
}

bool UpdateEngine::IsInstalled(const std::wstring& packageId, const std::wstring& packageVersion) const
{
    TRACE("UpdateEngine::IsInstalled()");

    lock_guard<mutex> lock(_mutex);

    for (const PackageData& packageData : _packageList)
    {
        if (packageData.id == packageId && packageData.version == packageVersion)
        {
            return true;
        }
    }
    return false;
}

void UpdateEngine::Stage(const std::wstring& cabFullFileName)
{
    TRACEP(L"UpdateEngine::Stage = ", cabFullFileName.c_str());

    wstring stagingFolder(StagingFolder);
    Utils::EnsureFolderExists(stagingFolder);
    stagingFolder += L"\\";

    path sourceFullPath(cabFullFileName);
    wstring fileName = sourceFullPath.filename();
    path targetFullPath(stagingFolder + fileName);

    copy_file(sourceFullPath, targetFullPath, copy_options::overwrite_existing);
}

void UpdateEngine::CommitStaged()
{
    unsigned long returnCode = 0;
    string output;

    // build the command line
    wchar_t systemRoot[MAX_PATH] = { 0 };
    wchar_t cmdLine[MAX_PATH] = { 0 };
    if (0 != GetEnvironmentVariable(L"SystemRoot", systemRoot, ARRAYSIZE(systemRoot)))
    {
        swprintf_s(cmdLine, MAX_PATH, L"%s\\system32\\applyupdate.exe -commit", systemRoot);
    }

    Utils::LaunchProcess(cmdLine, returnCode, output);
    if (0 != returnCode)
    {
        throw DMException("Error: ApplyUpdate.exe -commit failed. Error code = ", static_cast<unsigned int>(returnCode));
    }

    // This causes the machine to restart immediately, so there is not much point to do anything.
    // ToDo: can we have this delayed or scheduled?
}
