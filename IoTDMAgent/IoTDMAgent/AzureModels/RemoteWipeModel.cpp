#include "stdafx.h"
#include "RemoteWipeModel.h"
#include "..\LocalMachine\CSPs\RemoteWipeCSP.h"

using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace std;

#define RemoteWipeNodeName L"remoteWipe"
#define LastRemoteWipeTime L"lastRemoteWipe"

RemoteWipeModel::RemoteWipeModel()
{
    TRACE(L"RemoteWipeModel::RemoteWipeModel()");
}

wstring RemoteWipeModel::NodeName()
{
    return RemoteWipeNodeName;
}

JsonObject^ RemoteWipeModel::GetReportedProperties()
{
    TRACE(L"RemoteWipeModel::GetReportedProperties()");

    JsonObject^ properties = ref new JsonObject();
    properties->Insert(LastRemoteWipeTime, JsonValue::CreateStringValue(ref new String(_lastRemoteWipeCmdTime.c_str())));

    string jsonString = Utils::WideToMultibyte(properties->Stringify()->Data());
    TRACEP("RemoteWipe Model Json = ", jsonString.c_str());

    return properties;
}

string RemoteWipeModel::ExecWipe()
{
    TRACE(__FUNCTION__);

    RemoteWipeCSP::DoWipe();
    _lastRemoteWipeCmdTime = Utils::GetCurrentDateTimeString();
    return "";  // no (json) content is returned for remote wipe.
}
