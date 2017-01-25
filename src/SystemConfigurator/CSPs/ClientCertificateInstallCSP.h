#pragma once

#include <string>
#include <vector>

class ClientCertificateInstallCSP
{
public:

    class PFXCertInstall
    {
    public:
        static std::wstring GetCertHashes();

        // ToDo: 'add's might need to be combined under an atomic transaction.

        static int GetKeyLocation(const std::wstring& hash);
        static void SetKeyLocation(const std::wstring& hash, int value);
        static void AddKeyLocation(const std::wstring& hash, int value);

        static std::wstring GetContainerName(const std::wstring& hash);
        static void SetContainerName(const std::wstring& hash, const std::wstring& value);
        static void AddContainerName(const std::wstring& hash, const std::wstring& value);

        static std::wstring GetBlob(const std::wstring& hash);
        static void SetBlob(const std::wstring& hash, const std::wstring& blobInBase64);
        static void AddBlob(const std::wstring& hash, const std::wstring& blobInBase64);

        static std::wstring GetPassword(const std::wstring& hash);
        static void SetPassword(const std::wstring& hash, const std::wstring& password);
        static void AddPassword(const std::wstring& hash, const std::wstring& password);

        static int GetPasswordEncryptionType(const std::wstring& hash);
        static void SetPasswordEncryptionType(const std::wstring& hash, int type);
        static void AddPasswordEncryptionType(const std::wstring& hash, int type);

        static bool GetKeyExportable(const std::wstring& hash);
        static void SetKeyExportable(const std::wstring& hash, bool exportable);
        static void AddKeyExportable(const std::wstring& hash, bool exportable);

        // Get-only properties.
        static std::wstring GetThumbprint(const std::wstring& hash);
        static int GetStatus(const std::wstring& hash);
        static std::wstring GetPasswordEncryptionStore(const std::wstring& hash);
    };

    class SCEP
    {
    public:
        static std::wstring GetCertHashes();

        class Install
        {
            // ToDo: 'add's might need to be combined under an atomic transaction.

            static std::wstring GetServerUrl(const std::wstring& hash);
            static void SetServerUrl(const std::wstring& hash, const std::wstring& value);
            static void AddServerUrl(const std::wstring& hash, const std::wstring& value);

            static std::wstring GetChallenge(const std::wstring& hash);
            static void SetChallenge(const std::wstring& hash, const std::wstring& value);
            static void AddChallenge(const std::wstring& hash, const std::wstring& value);

            static std::wstring GetEKUMapping(const std::wstring& hash);
            static void SetEKUMapping(const std::wstring& hash, const std::wstring& value);
            static void AddEKUMapping(const std::wstring& hash, const std::wstring& value);

            static int GetKeyUsage(const std::wstring& hash);
            static void SetKeyUsage(const std::wstring& hash, int value);
            static void AddKeyUsage(const std::wstring& hash, int value);

            static std::wstring GetSubjectName(const std::wstring& hash);
            static void SetSubjectName(const std::wstring& hash, const std::wstring& value);
            static void AddSubjectName(const std::wstring& hash, const std::wstring& value);

            static int GetKeyProtection(const std::wstring& hash);
            static void SetKeyProtection(const std::wstring& hash, int value);
            static void AddKeyProtection(const std::wstring& hash, int value);

            static int GetRetryDelay(const std::wstring& hash);
            static void SetRetryDelay(const std::wstring& hash, int value);
            static void AddRetryDelay(const std::wstring& hash, int value);

            static int GetRetryCount(const std::wstring& hash);
            static void SetRetryCount(const std::wstring& hash, int value);
            static void AddRetryCount(const std::wstring& hash, int value);

            static int GetKeyLength(const std::wstring& hash);
            static void SetKeyLength(const std::wstring& hash, int value);
            static void AddKeyLength(const std::wstring& hash, int value);

            static std::wstring GetTemplateName(const std::wstring& hash);
            static void SetTemplateName(const std::wstring& hash, const std::wstring& value);
            static void AddTemplateName(const std::wstring& hash, const std::wstring& value);

            static std::wstring GetHashAlgorithm(const std::wstring& hash);
            static void SetHashAlgorithm(const std::wstring& hash, const std::wstring& value);
            static void AddHashAlgorithm(const std::wstring& hash, const std::wstring& value);

            static std::wstring GetCAThumbprint(const std::wstring& hash);
            static void SetCAThumbprint(const std::wstring& hash, const std::wstring& value);
            static void AddCAThumbprint(const std::wstring& hash, const std::wstring& value);

            static std::wstring GetSubjectAlternativeNames(const std::wstring& hash);
            static void SetSubjectAlternativeNames(const std::wstring& hash, const std::wstring& value);
            static void AddSubjectAlternativeNames(const std::wstring& hash, const std::wstring& value);

            static std::wstring GetValidPeriod(const std::wstring& hash);
            static void SetValidPeriod(const std::wstring& hash, const std::wstring& value);
            static void AddValidPeriod(const std::wstring& hash, const std::wstring& value);

            static std::wstring GetValidPeriodUnits(const std::wstring& hash);
            static void SetValidPeriodUnits(const std::wstring& hash, const std::wstring& value);
            static void AddValidPeriodUnits(const std::wstring& hash, const std::wstring& value);

            static std::wstring GetContainerName(const std::wstring& hash);
            static void SetContainerName(const std::wstring& hash, const std::wstring& value);
            static void AddContainerName(const std::wstring& hash, const std::wstring& value);

            static std::wstring GetCustomTextToShowInPrompt(const std::wstring& hash);
            static void SetCustomTextToShowInPrompt(const std::wstring& hash, const std::wstring& value);
            static void AddCustomTextToShowInPrompt(const std::wstring& hash, const std::wstring& value);

            static void Enroll(const std::wstring& hash);
        };

        static std::wstring GetThumbprint(const std::wstring& hash);
        static int GetStatus(const std::wstring& hash);
        static int GetErrorCode(const std::wstring& hash);
        static std::wstring GetRespondentServerUrl(const std::wstring& hash);

    };

};