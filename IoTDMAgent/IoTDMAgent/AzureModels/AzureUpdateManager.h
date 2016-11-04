#pragma once
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include "..\LocalMachine\LocalMachine.h"
#include "AzureUpdateModel.h"

class AzureUpdateManager
{
    struct UpdateOperation
    {
        std::wstring manifestFileName;
        bool download;
        bool install;

        UpdateOperation() :
            download(false),
            install(false)
        {}
    };

public:
    AzureUpdateManager();

    static std::wstring NodeName();

    // Desired
    void SetDesiredProperties(Windows::Data::Json::IJsonValue^ rebootNode);

    // Reported
    Windows::Data::Json::JsonObject^ GetReportedProperties() const;

private:
    void LoadLocalState(const std::wstring& updatesLocalRoot, const std::wstring& manifestsFolder);

    static void SetDesiredOperations(Windows::Data::Json::IJsonValue^ cabsNode, std::vector<UpdateOperation>& cabOperations);
    void ExecuteDesiredOperations(const std::wstring& connectionString, const std::wstring& containerName, const std::vector<UpdateOperation>& cabOperations);

    // Data members
    std::map<std::wstring, std::shared_ptr<AzureUpdateModel>> _localUpdates;
    UpdateEngine _updateEngine;
};