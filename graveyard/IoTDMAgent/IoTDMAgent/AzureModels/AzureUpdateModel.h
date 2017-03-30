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
#include <mutex>
#include "..\LocalMachine\UpdateEngine.h"

#define AzureManifestsFolder L"manifests"
#define AzurePayloadFolder L"payload"

class AzureUpdateModel
{
    struct CabData
    {
        std::wstring containerName;
        std::wstring cabFullFileName;
        std::wstring id;
        std::wstring version;
    };

public:

    AzureUpdateModel(const std::wstring& updatesLocalRoot, const std::wstring& manifestFileName, const UpdateEngine& updateEngine);

    bool IsDownloaded() const;
    bool IsInstalled() const;

    void Download(const std::wstring& connectionString);
    void Install();

    Windows::Data::Json::JsonObject^ GetReportedProperties() const;

private:

    static void ParseManifest(const std::wstring& updatesLocalRoot, const std::wstring& manifestFileName, std::vector<CabData>& cabsData);
    static bool VerifyCabsDownloaded(const std::wstring& updatesLocalRoot, const std::vector<CabData> cabsData);
    static bool VerifyCabsInstalled(const std::vector<CabData>& cabsData, const UpdateEngine& updateEngine);

    // Data members
    std::wstring _updatesRootFolder;
    std::wstring _manifestFullFileName;
    std::wstring _manifestFileName;
    std::wstring _id;
    bool _manifestDownloaded;
    bool _cabsDownloaded;
    bool _cabsInstalled;
    std::vector<CabData> _cabsData;
};
