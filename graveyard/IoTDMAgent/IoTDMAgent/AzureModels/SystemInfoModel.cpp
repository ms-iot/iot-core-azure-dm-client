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
