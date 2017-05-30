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
#include "MdmProvision.h"
#include "DeviceHealthAttestationCSP.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"

using namespace std;

// Device Health Attestation CSP docs
// https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/healthattestation-csp
//

void DeviceHealthAttestationCSP::ExecVerifyHealth()
{
    TRACE(__FUNCTION__);
    MdmProvision::RunExec(L"./Device/Vendor/MSFT/HealthAttestation/VerifyHealth");
}

uint32_t DeviceHealthAttestationCSP::GetStatus()
{
    TRACE(__FUNCTION__);
    return MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/HealthAttestation/Status");
}

void DeviceHealthAttestationCSP::SetForceRetrieve(bool value)
{
    TRACE(__FUNCTION__);
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/HealthAttestation/ForceRetrieve", value);
}

std::wstring DeviceHealthAttestationCSP::GetCertificate()
{
    TRACE(__FUNCTION__);
    return MdmProvision::RunGetBase64(L"./Device/Vendor/MSFT/HealthAttestation/Certificate");
}

// The nonce is in hex format, with a minimum size of 8 bytes, and a maximum size of 32 bytes.
std::wstring DeviceHealthAttestationCSP::GetNonce()
{
    TRACE(__FUNCTION__);
    return MdmProvision::RunGetString(L"./Device/Vendor/MSFT/HealthAttestation/Nonce");
}

void DeviceHealthAttestationCSP::SetNonce(const std::wstring& nonce)
{
    TRACE(__FUNCTION__);
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/HealthAttestation/Nonce", nonce);
}

std::wstring DeviceHealthAttestationCSP::GetCorrelationId()
{
    TRACE(__FUNCTION__);
    return MdmProvision::RunGetString(L"./Device/Vendor/MSFT/HealthAttestation/CorrelationId");
}

std::wstring DeviceHealthAttestationCSP::GetHASEndpoint()
{
    TRACE(__FUNCTION__);
    return MdmProvision::RunGetString(L"./Device/Vendor/MSFT/HealthAttestation/HASEndpoint");
}

void DeviceHealthAttestationCSP::SetHASEndpoint(const std::wstring& endpoint)
{
    TRACE(__FUNCTION__);
    MdmProvision::RunSet(L"./Device/Vendor/MSFT/HealthAttestation/HASEndpoint", endpoint);
}

int DeviceHealthAttestationCSP::GetTpmReadyStatus()
{
    TRACE(__FUNCTION__);
    return MdmProvision::RunGetUInt(L"./Device/Vendor/MSFT/HealthAttestation/TpmReadyStatus");
}
