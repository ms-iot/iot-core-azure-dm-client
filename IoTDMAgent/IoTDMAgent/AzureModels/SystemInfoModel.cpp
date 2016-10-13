#include "stdafx.h"
#include "SystemInfoModel.h"
#include "..\LocalMachine\LocalMachine.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

#define SystemInfoNode L"systemInfo"
#define OSVersion L"osVersion"
#define TotalMemory "totalMemory"
#define AvailableMemory "availableMemory"
#define TotalStorage "totalStorage"
#define AvailableStorage "availableStorage"

SystemInfoModel::SystemInfoModel()
{
    TRACE(L"SystemInfoModel::SystemInfoModel()");
}

wstring SystemInfoModel::NodeName()
{
    return SystemInfoNode;
}

Windows::Data::Json::JsonObject^ SystemInfoModel::GetReportedProperties() const
{
    TRACE(L"SystemInfoModel::GetReportedProperties()");

    unsigned int totalMemoryMB = 0;
    unsigned int availableMemoryMB = 0;
    LocalMachine::GetMemoryInfoMB(totalMemoryMB, availableMemoryMB);

    unsigned int totalStorageMB = 0;
    unsigned int availableStorageMB = 0;
    LocalMachine::GetStorageInfoMB(totalStorageMB, availableStorageMB);

    JsonObject^ systemInfoProperties = ref new JsonObject();
    systemInfoProperties->Insert(OSVersion, JsonValue::CreateStringValue(ref new String(LocalMachine::GetOSVersionString().c_str())));
    systemInfoProperties->Insert(TotalMemory, JsonValue::CreateNumberValue(totalMemoryMB));
    systemInfoProperties->Insert(AvailableMemory, JsonValue::CreateNumberValue(availableMemoryMB));
    systemInfoProperties->Insert(TotalStorage, JsonValue::CreateNumberValue(totalStorageMB));
    systemInfoProperties->Insert(AvailableStorage, JsonValue::CreateNumberValue(availableStorageMB));

    string jsonString = Utils::WideToMultibyte(systemInfoProperties->Stringify()->Data());
    TRACEP("Time Model Json = ", jsonString.c_str());

    return systemInfoProperties;
}