#pragma once
#include <windows.h>
#include <Aclapi.h>
#include <Sddl.h>
#include <string>

class PermissionsTracer
{
    enum Component
    {
        Unknown,
        Owner,
        Group,
        DaclFlags,
        SaclFlags,
    };

public:
    static void TraceSID(const std::wstring& format, PSID sid);
    static void TraceSD(const std::wstring& objectName, PSECURITY_DESCRIPTOR sd);

private:
    static void TraceSDString(const std::wstring& sdString);
    static void TraceDaclString(const std::wstring& daclString);

    static std::wstring TypeToString(const std::wstring& type);
    static std::wstring FlagsToString(const std::wstring& flags);
    static std::wstring RightsToString(const std::wstring& rights);
    static std::wstring SidToAccountString(const std::wstring& sidString);
};