#include "stdafx.h"
#include "ClientCertificateInstallCSP.h"
#include "MdmProvision.h"
#include "..\SharedUtilities\Logger.h"

using namespace std;

const wchar_t* PFXCertInstallPath = L"./Device/Vendor/MSFT/ClientCertificateInstall/PFXCertInstall";
const wchar_t* SCEPPath = L"./Device/Vendor/MSFT/ClientCertificateInstall/SCEP";

// CertificateStore CSP docs
// https://msdn.microsoft.com/en-us/windows/hardware/commercialize/customize/mdm/clientcertificateinstall-csp
//

wstring ClientCertificateInstallCSP::PFXCertInstall::GetCertHashes()
{
    TRACE(__FUNCTION__);

    wstring hashes = MdmProvision::RunGetString(PFXCertInstallPath);
    TRACEP(L" hashes = ", hashes.c_str());
    return hashes;
}

int ClientCertificateInstallCSP::PFXCertInstall::GetKeyLocation(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/KeyLocation";

    int value = MdmProvision::RunGetUInt(path);
    TRACEP(L" KeyLocation = ", value);
    return value;
}

void ClientCertificateInstallCSP::PFXCertInstall::SetKeyLocation(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/KeyLocation";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::PFXCertInstall::AddKeyLocation(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/KeyLocation";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::PFXCertInstall::GetContainerName(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/ContainerName";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" ContainerName = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::PFXCertInstall::SetContainerName(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/ContainerName";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::PFXCertInstall::AddContainerName(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/ContainerName";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::PFXCertInstall::GetBlob(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertBlob";

    wstring valueInBase64 = MdmProvision::RunGetBase64(path);
    TRACEP(L" PFXCertBlob = ", valueInBase64.c_str());
    return valueInBase64;
}

void ClientCertificateInstallCSP::PFXCertInstall::SetBlob(const wstring& hash, const wstring& blobInBase64)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertBlob";

    MdmProvision::RunSetBase64(path, blobInBase64);
}

void ClientCertificateInstallCSP::PFXCertInstall::AddBlob(const wstring& hash, const wstring& blobInBase64)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertBlob";

    MdmProvision::RunAddDataBase64(path, blobInBase64);
}

wstring ClientCertificateInstallCSP::PFXCertInstall::GetPassword(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertPassword";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" PFXCertPassword = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::PFXCertInstall::SetPassword(const wstring& hash, const wstring& password)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertPassword";

    MdmProvision::RunSet(path, password);
}

void ClientCertificateInstallCSP::PFXCertInstall::AddPassword(const wstring& hash, const wstring& password)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertPassword";

    MdmProvision::RunAddData(path, password);
}

int ClientCertificateInstallCSP::PFXCertInstall::GetPasswordEncryptionType(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertPasswordEncryptionType";

    int value = MdmProvision::RunGetUInt(path);
    TRACEP(L" PasswordEncryptionType = ", value);
    return value;
}

void ClientCertificateInstallCSP::PFXCertInstall::SetPasswordEncryptionType(const wstring& hash, int type)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertPasswordEncryptionType";

    MdmProvision::RunSet(path, type);
}

void ClientCertificateInstallCSP::PFXCertInstall::AddPasswordEncryptionType(const wstring& hash, int type)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertPasswordEncryptionType";

    MdmProvision::RunAddData(path, type);
}

bool ClientCertificateInstallCSP::PFXCertInstall::GetKeyExportable(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXKeyExportable";

    bool value = MdmProvision::RunGetBool(path);
    TRACEP(L" KeyExportable = ", (value ? L"True" : L"False"));
    return value;
}

void ClientCertificateInstallCSP::PFXCertInstall::SetKeyExportable(const wstring& hash, bool exportable)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXKeyExportable";

    MdmProvision::RunSet(path, exportable);
}

void ClientCertificateInstallCSP::PFXCertInstall::AddKeyExportable(const wstring& hash, bool exportable)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXKeyExportable";

    MdmProvision::RunAddData(path, exportable);
}

wstring ClientCertificateInstallCSP::PFXCertInstall::GetThumbprint(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/Thumbprint";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" Thumbprint = ", value.c_str());
    return value;
}

int ClientCertificateInstallCSP::PFXCertInstall::GetStatus(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/Status";

    int value = MdmProvision::RunGetUInt(path);
    TRACEP(L" Status = ", value);
    return value;
}

wstring ClientCertificateInstallCSP::PFXCertInstall::GetPasswordEncryptionStore(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = PFXCertInstallPath;
    path += L"/";
    path += hash;
    path += L"/PFXCertPasswordEncryptionStore";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" PasswordEncryptionStore = ", value.c_str());
    return value;
}

wstring ClientCertificateInstallCSP::SCEP::GetCertHashes()
{
    TRACE(__FUNCTION__);

    wstring hashes = MdmProvision::RunGetString(SCEPPath);
    TRACEP(L" hashes = ", hashes.c_str());
    return hashes;
}

wstring ClientCertificateInstallCSP::SCEP::GetThumbprint(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Thumbprint";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" Thumbprint = ", value.c_str());
    return value;
}

int ClientCertificateInstallCSP::SCEP::GetStatus(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Status";

    unsigned int  value = MdmProvision::RunGetUInt(path);
    TRACEP(L" Status = ", value);
    return value;
}

int ClientCertificateInstallCSP::SCEP::GetErrorCode(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/ErrorCode";

    unsigned int  value = MdmProvision::RunGetUInt(path);
    TRACEP(L" ErrorCode = ", value);
    return value;
}

wstring ClientCertificateInstallCSP::SCEP::GetRespondentServerUrl(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/RespondentServerUrl";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" RespondentServerUrl = ", value.c_str());
    return value;
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetServerUrl(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ServerURL";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" ServerURL = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetServerUrl(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ServerURL";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddServerUrl(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ServerURL";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetChallenge(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/Challenge";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" Challenge = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetChallenge(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/Challenge";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddChallenge(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/Challenge";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetEKUMapping(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/EKUMapping";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" EKUMapping = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetEKUMapping(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/EKUMapping";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddEKUMapping(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/EKUMapping";

    MdmProvision::RunAddData(path, value);
}

int ClientCertificateInstallCSP::SCEP::Install::GetKeyUsage(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/KeyUsage";

    unsigned int value = MdmProvision::RunGetUInt(path);
    TRACEP(L" KeyUsage = ", value);
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetKeyUsage(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/KeyUsage";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddKeyUsage(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/KeyUsage";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetSubjectName(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/SubjectName";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" SubjectName = ", value);
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetSubjectName(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/SubjectName";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddSubjectName(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/SubjectName";

    MdmProvision::RunAddData(path, value);
}

int ClientCertificateInstallCSP::SCEP::Install::GetKeyProtection(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/KeyProtection";

    unsigned int value = MdmProvision::RunGetUInt(path);
    TRACEP(L" KeyProtection = ", value);
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetKeyProtection(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/KeyProtection";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddKeyProtection(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/KeyProtection";

    MdmProvision::RunAddData(path, value);
}

int ClientCertificateInstallCSP::SCEP::Install::GetRetryDelay(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/RetryDelay";

    unsigned int value = MdmProvision::RunGetUInt(path);
    TRACEP(L" RetryDelay = ", value);
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetRetryDelay(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/RetryDelay";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddRetryDelay(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/RetryDelay";

    MdmProvision::RunAddData(path, value);
}

int ClientCertificateInstallCSP::SCEP::Install::GetRetryCount(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/RetryCount";

    unsigned int value = MdmProvision::RunGetUInt(path);
    TRACEP(L" RetryCount = ", value);
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetRetryCount(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/RetryCount";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddRetryCount(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/RetryCount";

    MdmProvision::RunAddData(path, value);
}

int ClientCertificateInstallCSP::SCEP::Install::GetKeyLength(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/KeyLength";

    unsigned int value = MdmProvision::RunGetUInt(path);
    TRACEP(L" KeyLength = ", value);
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetKeyLength(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/KeyLength";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddKeyLength(const wstring& hash, int value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/KeyLength";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetTemplateName(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/TemplateName";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" TemplateName = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetTemplateName(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/TemplateName";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddTemplateName(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/TemplateName";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetHashAlgorithm(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/HashAlgorithm";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" HashAlgorithm = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetHashAlgorithm(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/HashAlgorithm";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddHashAlgorithm(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/HashAlgorithm";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetCAThumbprint(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/CAThumbprint";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" CAThumbprint = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetCAThumbprint(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/CAThumbprint";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddCAThumbprint(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/CAThumbprint";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetSubjectAlternativeNames(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/SubjectAlternativeNames";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" SubjectAlternativeNames = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetSubjectAlternativeNames(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/SubjectAlternativeNames";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddSubjectAlternativeNames(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/SubjectAlternativeNames";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetValidPeriod(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ValidPeriod";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" ValidPeriod = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetValidPeriod(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ValidPeriod";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddValidPeriod(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ValidPeriod";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetValidPeriodUnits(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ValidPeriodUnits";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" ValidPeriodUnits = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetValidPeriodUnits(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ValidPeriodUnits";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddValidPeriodUnits(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ValidPeriodUnits";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetContainerName(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ContainerName";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" ContainerName = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetContainerName(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ContainerName";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddContainerName(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/ContainerName";

    MdmProvision::RunAddData(path, value);
}

wstring ClientCertificateInstallCSP::SCEP::Install::GetCustomTextToShowInPrompt(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/CustomTextToShowInPrompt";

    wstring value = MdmProvision::RunGetString(path);
    TRACEP(L" CustomTextToShowInPrompt = ", value.c_str());
    return value;
}

void ClientCertificateInstallCSP::SCEP::Install::SetCustomTextToShowInPrompt(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/CustomTextToShowInPrompt";

    MdmProvision::RunSet(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::AddCustomTextToShowInPrompt(const wstring& hash, const wstring& value)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/CustomTextToShowInPrompt";

    MdmProvision::RunAddData(path, value);
}

void ClientCertificateInstallCSP::SCEP::Install::Enroll(const wstring& hash)
{
    TRACE(__FUNCTION__);

    wstring path = SCEPPath;
    path += L"/";
    path += hash;
    path += L"/Install/Enroll";

    MdmProvision::RunExec(path);
}