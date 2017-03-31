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

JsonObject^ RemoteWipeModel::GetReportedProperties() const
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
