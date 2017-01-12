#include "stdafx.h"
#include "DeviceStatusCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"

#define SecureBootState L"secureBootState"
#define MacIpAddressV4 L"macIpAddressV4"
#define MacIpAddressV6 L"macIpAddressV6"
#define MacAddressIsConnected L"macAddressIsConnected"
#define MacAddressType L"macAddressType"
#define OsType L"osType"
#define BatteryStatus L"batteryStatus"
#define BatteryRemaining L"batteryRemaining"
#define BatteryRuntime L"batteryRuntime"

using namespace Windows::System;
using namespace Platform;
using namespace Windows::Data::Json;
using namespace Windows::System::Profile;
using namespace Windows::Foundation::Collections;

using namespace std;
using namespace Utils;

void DeviceStatusCSP::GetDeviceStatus(DeviceStatusInfo& deviceStatusInfo)
{
    TRACE(__FUNCTION__);

    deviceStatusInfo.secureBootState = MdmProvision::RunGetUInt(L"./Vendor/MSFT/DeviceStatus/SecureBootState");

    /*
    // Not applicable to IoT Core.
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/CellularIdentities/IMEI/IMSI");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/CellularIdentities/IMEI/ICCID");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/CellularIdentities/IMEI/PhoneNumber");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/CellularIdentities/IMEI/CommercializationOperator");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/CellularIdentities/IMEI/RoamingStatus");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/CellularIdentities/IMEI/RoamingCompliance");
    */
    deviceStatusInfo.macAddressIpV4 = MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/NetworkIdentifiers/MacAddress/IPAddressV4");
    deviceStatusInfo.macAddressIpV6 = MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/NetworkIdentifiers/MacAddress/IPAddressV6");
    deviceStatusInfo.macAddressIsConnected = MdmProvision::RunGetBool(L"./Vendor/MSFT/DeviceStatus/NetworkIdentifiers/MacAddress/IsConnected");
    deviceStatusInfo.macAddressType = MdmProvision::RunGetUInt(L"./Vendor/MSFT/DeviceStatus/NetworkIdentifiers/MacAddress/Type");

    /*
    // Failing with error 405 "not allowed".
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/Compliance/EncryptionCompliance");
    */
    /*
    // Failing with error 500 "failed"
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/TPM/SpecificationVersion");
    */
    deviceStatusInfo.osType = MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/OS/Edition");
    /*
    // Failing with error 405 "not allowed" or 500 "failed".
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/Antivirus/SignatureStatus");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/Antivirus/Status");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/Antispyware/SignatureStatus");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/Antispyware/Status");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/Firewall/Status");
    MdmProvision::RunGetString(L"./Vendor/MSFT/DeviceStatus/UAC/Status");
    */

    deviceStatusInfo.batteryStatus = MdmProvision::RunGetUInt(L"./Vendor/MSFT/DeviceStatus/Battery/Status");
    deviceStatusInfo.batteryRemaining = MdmProvision::RunGetUInt(L"./Vendor/MSFT/DeviceStatus/Battery/EstimatedChargeRemaining");
    deviceStatusInfo.batteryRuntime = MdmProvision::RunGetUInt(L"./Vendor/MSFT/DeviceStatus/Battery/EstimatedRuntime");
}

std::wstring DeviceStatusCSP::GetDeviceStatusJson()
{
    TRACE(__FUNCTION__);

    DeviceStatusInfo deviceStatusInfo;
    GetDeviceStatus(deviceStatusInfo);

    JsonObject^ deviceStatusJson = ref new JsonObject();
    deviceStatusJson->Insert(ref new Platform::String(SecureBootState),
                             JsonValue::CreateNumberValue(deviceStatusInfo.secureBootState));

    deviceStatusJson->Insert(ref new Platform::String(MacIpAddressV4),
                            JsonValue::CreateStringValue(ref new Platform::String(deviceStatusInfo.macAddressIpV4.c_str())));

    deviceStatusJson->Insert(ref new Platform::String(MacIpAddressV6),
                            JsonValue::CreateStringValue(ref new Platform::String(deviceStatusInfo.macAddressIpV6.c_str())));

    deviceStatusJson->Insert(ref new Platform::String(MacAddressIsConnected),
                             JsonValue::CreateBooleanValue(deviceStatusInfo.macAddressIsConnected));

    deviceStatusJson->Insert(ref new Platform::String(MacAddressType),
                            JsonValue::CreateNumberValue(deviceStatusInfo.macAddressType));

    deviceStatusJson->Insert(ref new Platform::String(OsType),
                            JsonValue::CreateStringValue(ref new Platform::String(deviceStatusInfo.osType.c_str())));

    deviceStatusJson->Insert(ref new Platform::String(BatteryStatus),
                            JsonValue::CreateNumberValue(deviceStatusInfo.batteryStatus));

    deviceStatusJson->Insert(ref new Platform::String(BatteryRemaining),
                            JsonValue::CreateNumberValue(deviceStatusInfo.batteryRemaining));

    deviceStatusJson->Insert(ref new Platform::String(BatteryRuntime),
                            JsonValue::CreateNumberValue(deviceStatusInfo.batteryRuntime));

    wstring json = deviceStatusJson->Stringify()->Data();

    TRACEP(L" json = ", json.c_str());

    return json;
}

