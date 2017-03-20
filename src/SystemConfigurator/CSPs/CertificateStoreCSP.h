#pragma once

#include <string>
#include "CertificateInfo.h"

class CertificateStoreCSP
{
public:
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