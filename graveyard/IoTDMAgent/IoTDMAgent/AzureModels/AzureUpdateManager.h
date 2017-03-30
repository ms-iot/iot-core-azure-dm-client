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
