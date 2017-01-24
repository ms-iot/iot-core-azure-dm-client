#pragma once

#include <string>
#include "CertificateInfo.h"

class CertificateStoreCSP
{
public:
    // subPath: CA, Root, My
    static std::wstring GetSystemCertHashes(const std::wstring& subPath);

    static void AddSystemCertificate(const std::wstring& subPath, const std::wstring& certHash, const std::wstring& certificateInBase64);
    static void DeleteSystemCertificate(const std::wstring& subPath, const std::wstring& certHash);

    static CertificateInfo GetSystemCertificateInfo(const std::wstring& subPath, const std::wstring& certHash);

    class My
    {
    public:
        class WSTEP
        {
        public:
            class Renew
            {
            public:
                static int GetPeriod();
                static std::wstring GetServerUrl();
                static int GetRetryInterval();
                static bool GetROBOSupport();
                static int GetStatus();
                static int GetErrorCode();
                static std::wstring GetLastRenewalAttemptTime();
                static void GetExec();
            };

            static std::wstring GetCertThumbprint();
        };
    };
};