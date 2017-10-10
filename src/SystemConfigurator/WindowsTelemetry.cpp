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
#include "Utils.h"
#include "DMException.h"
#include "WindowsTelemetry.h"
#include "../DMShared/ErrorCodes.h"

using namespace std;
using namespace Microsoft::Devices::Management::Message;

const wchar_t* JsonUnknown = L"<unknown>";
const wchar_t* JsonSecurity = L"security";
const wchar_t* JsonBasic = L"basic";
const wchar_t* JsonEnhanced = L"enhanced";
const wchar_t* JsonFull = L"full";

const wchar_t* RegWindowsTelemetrySubKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\DataCollection";
const wchar_t* RegAllowTelemetry = L"AllowTelemetry";

wstring WindowsTelemetry::LevelToString(DWORD level)
{
    wstring levelString;
    switch (level)
    {
    case 0:
        levelString = JsonSecurity;
        break;
    case 1:
        levelString = JsonBasic;
        break;
    case 2:
        levelString = JsonEnhanced;
        break;
    case 3:
        levelString = JsonFull;
        break;
    default:
        {
            basic_ostringstream<char> msg;
            msg << "Unknown level: " << level;
            throw DMExceptionWithErrorCode(msg.str().c_str(), ERROR_DM_WINDOWS_TELEMETRY_INVALID_LEVEL);
        }
    }

    return levelString;
}

unsigned long WindowsTelemetry::StringToLevel(const wstring& levelString)
{
    TRACE(__FUNCTION__);

    unsigned int level = 0;

    if (levelString == JsonSecurity)
    {
        level = 0;
    }
    else if(levelString == JsonBasic)
    {
        level = 1;
    }
    else if (levelString == JsonEnhanced)
    {
        level = 2;
    }
    else if (levelString == JsonFull)
    {
        level = 3;
    }
    else
    {
        basic_ostringstream<char> msg;
        msg << "Unknown level: " << Utils::WideToMultibyte(levelString.c_str());
        throw DMExceptionWithErrorCode(msg.str().c_str(), ERROR_DM_WINDOWS_TELEMETRY_INVALID_LEVEL);
    }

    return level;
}

GetWindowsTelemetryResponse^ WindowsTelemetry::Get()
{
    TRACE(__FUNCTION__);

    auto data = ref new WindowsTelemetryData();

    DWORD value;
    if (ERROR_SUCCESS == Utils::TryReadRegistryValue(RegWindowsTelemetrySubKey, RegAllowTelemetry, value))
    {
        data->level = ref new String(LevelToString(value).c_str());
    }
    else
    {
        throw DMExceptionWithErrorCode(ERROR_DM_WINDOWS_TELEMETRY_MISSING_LEVEL);
    }

    return ref new GetWindowsTelemetryResponse(ResponseStatus::Success, data);
}

void WindowsTelemetry::Set(SetWindowsTelemetryRequest^ request)
{
    DWORD value = StringToLevel(request->data->level->Data());

    Utils::WriteRegistryValue(RegWindowsTelemetrySubKey, RegAllowTelemetry, value);
}
