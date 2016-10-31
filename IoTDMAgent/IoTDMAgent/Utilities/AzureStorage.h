#pragma once

#include <string>

namespace AzureStorage
{
    void Download(const std::wstring& connectionString, const std::wstring& containerName, const std::wstring& fileName, const std::wstring& targetFolder);
}