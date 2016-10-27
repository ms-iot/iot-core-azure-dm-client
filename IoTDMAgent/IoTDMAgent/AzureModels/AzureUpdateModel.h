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
    mutable std::mutex _mutex;

    std::wstring _updatesRootFolder;
    std::wstring _manifestFullFileName;
    std::wstring _manifestFileName;
    std::wstring _id;
    bool _manifestDownloaded;
    bool _cabsDownloaded;
    bool _cabsInstalled;
    std::vector<CabData> _cabsData;
};