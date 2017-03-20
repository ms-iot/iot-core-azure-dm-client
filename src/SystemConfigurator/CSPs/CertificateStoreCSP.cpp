#include "stdafx.h"
#include "CertificateStoreCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"

using namespace std;

const wchar_t* CertificateStorePath = L"./Vendor/MSFT/CertificateStore";
const wchar_t* CertificateStoreMyWSTEPRenew = L"./Vendor/MSFT/CertificateStore/My/WSTEP/Renew";

// CertificateStore CSP docs
// https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/certificatestore-csp
//
int CertificateStoreCSP::My::WSTEP::Renew::GetPeriod()
{
    TRACE(__FUNCTION__);

    wstring path = CertificateStoreMyWSTEPRenew;
    path += L"/RenewPeriod";

    return MdmProvision::RunGetUInt(path);
}

wstring CertificateStoreCSP::My::WSTEP::Renew::GetServerUrl()
{
    TRACE(__FUNCTION__);

    wstring path = CertificateStoreMyWSTEPRenew;
    path += L"/ServerURL";

    return MdmProvision::RunGetString(path);
}

int CertificateStoreCSP::My::WSTEP::Renew::GetRetryInterval()
{
    TRACE(__FUNCTION__);

    wstring path = CertificateStoreMyWSTEPRenew;
    path += L"/RetryInterval";

    return MdmProvision::RunGetUInt(path);
}

bool CertificateStoreCSP::My::WSTEP::Renew::GetROBOSupport()
{
    TRACE(__FUNCTION__);

    wstring path = CertificateStoreMyWSTEPRenew;
    path += L"/ROBOSupport";

    return MdmProvision::RunGetBool(path);
}

int CertificateStoreCSP::My::WSTEP::Renew::GetStatus()
{
    TRACE(__FUNCTION__);

    wstring path = CertificateStoreMyWSTEPRenew;
    path += L"/Status";

    return MdmProvision::RunGetUInt(path);
}

int CertificateStoreCSP::My::WSTEP::Renew::GetErrorCode()
{
    TRACE(__FUNCTION__);

    wstring path = CertificateStoreMyWSTEPRenew;
    path += L"/ErrorCode";

    return MdmProvision::RunGetUInt(path);
}

wstring CertificateStoreCSP::My::WSTEP::Renew::GetLastRenewalAttemptTime()
{
    TRACE(__FUNCTION__);

    wstring path = CertificateStoreMyWSTEPRenew;
    path += L"/LastRenewalAttemptTime";

    return MdmProvision::RunGetString(path);
}

void CertificateStoreCSP::My::WSTEP::Renew::GetExec()
{
    TRACE(__FUNCTION__);

    wstring path = CertificateStoreMyWSTEPRenew;
    path += L"/RenewNow";

    MdmProvision::RunExec(path);
}

wstring CertificateStoreCSP::My::WSTEP::GetCertThumbprint()
{
    TRACE(__FUNCTION__);

    wstring path = CertificateStoreMyWSTEPRenew;
    path += L"/CertThumbprint";

    return MdmProvision::RunGetString(path);
}