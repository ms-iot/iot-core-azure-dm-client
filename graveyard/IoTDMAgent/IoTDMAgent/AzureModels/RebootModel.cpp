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
#include "RebootModel.h"
#include "..\LocalMachine\CSPs\RebootCSP.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

#define Reboot L"reboot"
#define LastRebootTime L"lastReboot"
#define LastRestartTime L"lastRestart"
#define SingleRebootTime L"singleReboot"
#define DailyRebootTime L"dailyReboot"

const wchar_t* IoTDMRegistryRoot = L"Software\\Microsoft\\IoTDM";
const wchar_t* IoTDMRegistryLastRebootCmd = L"LastRebootCmd";

RebootModel::RebootModel()
{
    TRACE(L"RebootModel::RebootModel()");

    try
    {
        _lastRebootCmdTime = Utils::ReadRegistryValue(IoTDMRegistryRoot, IoTDMRegistryLastRebootCmd);
    }
    catch (const exception&)
    {
        // An exception is throw if the value does not exist. This is okay.
    }

    _lastRestartTime = Utils::GetCurrentDateTimeString();
}

wstring RebootModel::NodeName()
{
    return Reboot;
}

void RebootModel::SetDesiredProperties(Windows::Data::Json::IJsonValue^ rebootNode)
{
    TRACE(L"RebootModel::SetDesiredProperties()");

    // Empty out values...
    wstring singleRebootTime = L"";
    wstring dailyRebootTime = L"";

    // Then, try to read them if the format is valid...
    JsonValueType type = rebootNode->ValueType;
    if (type == JsonValueType::Object)
    {
        JsonObject^ object = rebootNode->GetObject();
        if (object != nullptr)
        {
            for (IIterator<IKeyValuePair<String^, IJsonValue^>^>^ iter = object->First();
                iter->HasCurrent;
                iter->MoveNext())
            {
                IKeyValuePair<String^, IJsonValue^>^ pair = iter->Current;
                String^ childKey = pair->Key;
                if (childKey == SingleRebootTime)
                {
                    if (pair->Value->ValueType == JsonValueType::String)
                    {
                        singleRebootTime = pair->Value->GetString()->Data();
                    }
                }
                else if (childKey == DailyRebootTime)
                {
                    if (pair->Value->ValueType == JsonValueType::String)
                    {
                        dailyRebootTime = pair->Value->GetString()->Data();
                    }
                }
            }
        }
        else
        {
            TRACE("Warning: Unexpected reboot properties contents. Skipping.");
            return;
        }
    }
    else
    {
        TRACE("Warning: Unexpected reboot properties contents. Skipping.");
    }

    // Reflect the state on the local machine
    RebootCSP::SetSingleScheduleTime(singleRebootTime);
    RebootCSP::SetDailyScheduleTime(dailyRebootTime);
}

JsonObject^ RebootModel::GetReportedProperties() const
{
    TRACE(L"RebootModel::GetReportedProperties()");

    JsonObject^ rebootProperties = ref new JsonObject();
    rebootProperties->Insert(LastRebootTime, JsonValue::CreateStringValue(ref new String(_lastRebootCmdTime.c_str())));
    rebootProperties->Insert(LastRestartTime, JsonValue::CreateStringValue(ref new String(_lastRestartTime.c_str())));
    rebootProperties->Insert(SingleRebootTime, JsonValue::CreateStringValue(ref new String(RebootCSP::GetSingleScheduleTime().c_str())));
    rebootProperties->Insert(DailyRebootTime, JsonValue::CreateStringValue(ref new String(RebootCSP::GetDailyScheduleTime().c_str())));

    string jsonString = Utils::WideToMultibyte(rebootProperties->Stringify()->Data());
    TRACEP("Reboot Model Json = ", jsonString.c_str());

    return rebootProperties;
}

std::string RebootModel::ExecRebootNow()
{
    TRACE(__FUNCTION__);

    _lastRebootCmdTime = Utils::GetCurrentDateTimeString();
    Utils::WriteRegistryValue(IoTDMRegistryRoot, IoTDMRegistryLastRebootCmd, _lastRebootCmdTime);

    // Note: it is not 'now' - the CSP implementation has a 5 minute delay.
    RebootCSP::ExecRebootNow();
    return "";  // no (json) content is returned for reboot.
}
