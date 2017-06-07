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
#include <vector>
#include <map>
#include "Logger.h"
#include "PermissionsTracer.h"

using namespace std;

const wstring TrustedInstallerSidString = L"S-1-5-80-956008885-3418522649-1831038044-1853292631-2271478464";

void PermissionsTracer::TraceSID(const wstring& format, PSID sid)
{
    if (sid != nullptr)
    {
        LPWSTR s = NULL;
        if (ConvertSidToStringSid(sid, &s))
        {
            TRACEP(format.c_str(), s);
            LocalFree(s);
        }
        else
        {
            TRACE(L"Error: Failed to convert sid to a string!");
        }
    }
    else
    {
        TRACEP(format.c_str(), L"NULL");
    }
}

void PermissionsTracer::TraceSD(const std::wstring& objectName, PSECURITY_DESCRIPTOR sd)
{
    LPWSTR securityDescriptorString = NULL;
    ULONG securityDescriptorStringLen = 0;
    if (ConvertSecurityDescriptorToStringSecurityDescriptor(
        sd,
        SDDL_REVISION_1,
        ATTRIBUTE_SECURITY_INFORMATION |
        LABEL_SECURITY_INFORMATION |
        PROTECTED_DACL_SECURITY_INFORMATION |
        PROTECTED_SACL_SECURITY_INFORMATION |
        SCOPE_SECURITY_INFORMATION |
        UNPROTECTED_DACL_SECURITY_INFORMATION |
        UNPROTECTED_SACL_SECURITY_INFORMATION |
        OWNER_SECURITY_INFORMATION |
        GROUP_SECURITY_INFORMATION |
        DACL_SECURITY_INFORMATION |
        SACL_SECURITY_INFORMATION,
        &securityDescriptorString,
        &securityDescriptorStringLen))
    {
        TRACE(L"--------------------------------------------------------------");
        TRACEP(L"............Security Descriptor for ", objectName.c_str());
        TRACEP(L"............Security Descriptor String = ", securityDescriptorString);
        TRACE(L"--------------------------------------------------------------");
        TraceSDString(securityDescriptorString);
        LocalFree(securityDescriptorString);
    }

    TRACE(L"............Successfully saved current owner and acl.");
}

void PermissionsTracer::TraceSDString(const wstring& sdString)
{
    wstring owner;
    wstring group;
    wstring daclFlags;
    wstring saclFlags;

    Component currentComponent = Unknown;

    for (size_t i = 0; i < sdString.length(); ++i)
    {
        if (i < sdString.length() - 1)
        {
            if (sdString[i] == L'O' && sdString[i + 1] == L':')
            {
                currentComponent = Component::Owner;
                ++i;
                continue;
            }
            else if (sdString[i] == L'G' && sdString[i + 1] == L':')
            {
                currentComponent = Component::Group;
                ++i;
                continue;
            }
            else if (sdString[i] == L'D' && sdString[i + 1] == L':')
            {
                currentComponent = Component::DaclFlags;
                ++i;
                continue;
            }
            else if (sdString[i] == L'S' && sdString[i + 1] == L':')
            {
                currentComponent = Component::SaclFlags;
                ++i;
                continue;
            }
        }

        switch (currentComponent)
        {
        case Component::Owner:
            owner += sdString[i];
            break;
        case Component::Group:
            group += sdString[i];
            break;
        case Component::DaclFlags:
            daclFlags += sdString[i];
            break;
        case Component::SaclFlags:
            saclFlags += sdString[i];
            break;
        }
    }

    wprintf(L"\n");

    if (owner == TrustedInstallerSidString)
    {
        TRACE(L"owner = Trusted Installer");
    }
    else
    {
        TRACEP(L"owner = ", owner.c_str());
    }
    if (group == TrustedInstallerSidString)
    {
        TRACE(L"group = Trusted Installer");
    }
    else
    {
        TRACEP(L"group = ", group.c_str());
    }

    TraceDaclString(daclFlags);
    TRACEP(L"saclFlags = ", saclFlags.c_str());
}

wstring PermissionsTracer::TypeToString(const wstring& type)
{
    static map<wstring, wstring> typeToString;
    if (typeToString.size() == 0)
    {
        typeToString[L"A"] = L"ACCESS_ALLOWED";
        typeToString[L"D"] = L"ACCESS_DENIED";
        typeToString[L"OA"] = L"OBJECT_ACCESS_ALLOWED";
        typeToString[L"OD"] = L"OBJECT_ACCESS_DENIED";
        typeToString[L"AU"] = L"AUDIT";
        typeToString[L"AL"] = L"ALARM";
        typeToString[L"OU"] = L"OBJECT_AUDIT";
        typeToString[L"OL"] = L"OBJECT_ALARM";
        typeToString[L"ML"] = L"MANDATORY_LABEL";

        typeToString[L"XA"] = L"CALLBACK_ACCESS_ALLOWED";
        typeToString[L"XD"] = L"CALLBACK_ACCESS_DENIED";
        typeToString[L"RA"] = L"RESOURCE_ATTRIBUTE";

        typeToString[L"SP"] = L"SCOPED_POLICY_ID";
        typeToString[L"XU"] = L"CALLBACK_AUDIT";
        typeToString[L"ZA"] = L"CALLBACK_OBJECT_ACCESS_ALLOWED";
    }

    wstring s;
    s += L"(";
    s += type;
    s += L") ";
    s += typeToString[type];
    return s;
}

wstring PermissionsTracer::FlagsToString(const wstring& flags)
{
    static map<wstring, wstring> flagsToString;
    if (flagsToString.size() == 0)
    {
        flagsToString[L"CI"] = L"CONTAINER_INHERIT";
        flagsToString[L"OI"] = L"OBJECT_INHERIT";
        flagsToString[L"NP"] = L"NO_PROPAGATE";
        flagsToString[L"IO"] = L"INHERIT_ONLY";
        flagsToString[L"ID"] = L"INHERITED";
        flagsToString[L"SA"] = L"AUDIT_SUCCESS";
        flagsToString[L"FA"] = L"AUDIT_FAILURE";
    }

    wstring s;

    if (flags.size() != 0)
    {
        s += L"(";
        s += flags;
        s += L") ";

        for (size_t i = 0; i < flags.size() - 1; i += 2)
        {
            wstring flag = flags.substr(i, 2);
            s += flagsToString[flag] + L" ";
        }
    }

    return s;
}

wstring PermissionsTracer::RightsToString(const wstring& rights)
{
    static map<wstring, wstring> rightsToString;
    if (rightsToString.size() == 0)
    {
        rightsToString[L"GA"] = L"GENERIC_ALL";
        rightsToString[L"GR"] = L"GENERIC_READ";
        rightsToString[L"GW"] = L"GENERIC_WRITE";
        rightsToString[L"GX"] = L"GENERIC_EXECUTE";
        rightsToString[L"RC"] = L"READ_CONTROL";
        rightsToString[L"SD"] = L"DELETE";
        rightsToString[L"WD"] = L"WRITE_DAC";
        rightsToString[L"WO"] = L"WRITE_OWNER";
        rightsToString[L"RP"] = L"ADS_RIGHT_DS_READ_PROP";
        rightsToString[L"WP"] = L"ADS_RIGHT_DS_WRITE_PROP";
        rightsToString[L"CC"] = L"ADS_RIGHT_DS_CREATE_CHILD";
        rightsToString[L"DC"] = L"ADS_RIGHT_DS_DELETE_CHILD";
        rightsToString[L"LC"] = L"ADS_RIGHT_ACTRL_DS_LIST";
        rightsToString[L"SW"] = L"ADS_RIGHT_DS_SELF";
        rightsToString[L"LO"] = L"ADS_RIGHT_DS_LIST_OBJECT";
        rightsToString[L"DT"] = L"ADS_RIGHT_DS_DELETE_TREE";
        rightsToString[L"CR"] = L"ADS_RIGHT_DS_CONTROL_ACCESS";
        rightsToString[L"FA"] = L"FILE_ALL_ACCESS";
        rightsToString[L"FR"] = L"FILE_GENERIC_READ";
        rightsToString[L"FW"] = L"FILE_GENERIC_WRITE";
        rightsToString[L"FX"] = L"FILE_GENERIC_EXECUTE";
        rightsToString[L"KA"] = L"KEY_ALL_ACCESS";
        rightsToString[L"KR"] = L"KEY_READ";
        rightsToString[L"KW"] = L"KEY_WRITE";
        rightsToString[L"KX"] = L"KEY_EXECUTE";
        rightsToString[L"NR"] = L"SYSTEM_MANDATORY_LABEL_NO_READ_UP";
        rightsToString[L"NW"] = L"SYSTEM_MANDATORY_LABEL_NO_WRITE_UP";
        rightsToString[L"NX"] = L"SYSTEM_MANDATORY_LABEL_NO_EXECUTE_UP";
    }

    wstring s;

    if (rights.size() != 0)
    {
        s += L"(";
        s += rights;
        s += L") ";

        for (size_t i = 0; i < rights.size() - 1; i += 2)
        {
            wstring right = rights.substr(i, 2);
            s += rightsToString[right] + L" ";
        }
    }

    return s;
}

wstring PermissionsTracer::SidToAccountString(const wstring& sidString)
{
    static map<wstring, wstring> sidToString;
    if (sidToString.size() == 0)
    {
        sidToString[L"AN"] = L"SDDL_ANONYMOUS";
        sidToString[L"AO"] = L"SDDL_ACCOUNT_OPERATORS";
        sidToString[L"AU"] = L"SDDL_AUTHENTICATED_USERS";
        sidToString[L"BA"] = L"SDDL_BUILTIN_ADMINISTRATORS";
        sidToString[L"BG"] = L"SDDL_BUILTIN_GUESTS";
        sidToString[L"BO"] = L"SDDL_BACKUP_OPERATORS";
        sidToString[L"BU"] = L"SDDL_BUILTIN_USERS";
        sidToString[L"CA"] = L"SDDL_CERT_SERV_ADMINISTRATORS";
        sidToString[L"CD"] = L"SDDL_CERTSVC_DCOM_ACCESS";
        sidToString[L"CG"] = L"SDDL_CREATOR_GROUP";
        sidToString[L"CO"] = L"SDDL_CREATOR_OWNER";
        sidToString[L"DA"] = L"SDDL_DOMAIN_ADMINISTRATORS";
        sidToString[L"DC"] = L"SDDL_DOMAIN_COMPUTERS";
        sidToString[L"DD"] = L"SDDL_DOMAIN_DOMAIN_CONTROLLERS";
        sidToString[L"DG"] = L"SDDL_DOMAIN_GUESTS";
        sidToString[L"DU"] = L"SDDL_DOMAIN_USERS";
        sidToString[L"EA"] = L"SDDL_ENTERPRISE_ADMINS";
        sidToString[L"ED"] = L"SDDL_ENTERPRISE_DOMAIN_CONTROLLERS";
        sidToString[L"HI"] = L"SDDL_ML_HIGH";
        sidToString[L"IU"] = L"SDDL_INTERACTIVE";
        sidToString[L"LA"] = L"SDDL_LOCAL_ADMIN";
        sidToString[L"LG"] = L"SDDL_LOCAL_GUEST";
        sidToString[L"LS"] = L"SDDL_LOCAL_SERVICE";
        sidToString[L"LW"] = L"SDDL_ML_LOW";
        sidToString[L"ME"] = L"SDDL_MLMEDIUM";
        sidToString[L"MU"] = L"SDDL_PERFMON_USERS";
        sidToString[L"NO"] = L"SDDL_NETWORK_CONFIGURATION_OPS";
        sidToString[L"NS"] = L"SDDL_NETWORK_SERVICE";
        sidToString[L"NU"] = L"SDDL_NETWORK";
        sidToString[L"PA"] = L"SDDL_GROUP_POLICY_ADMINS";
        sidToString[L"PO"] = L"SDDL_PRINTER_OPERATORS";
        sidToString[L"PS"] = L"SDDL_PERSONAL_SELF";
        sidToString[L"PU"] = L"SDDL_POWER_USERS";
        sidToString[L"RC"] = L"SDDL_RESTRICTED_CODE";
        sidToString[L"RD"] = L"SDDL_REMOTE_DESKTOP";
        sidToString[L"RE"] = L"SDDL_REPLICATOR";
        sidToString[L"RO"] = L"SDDL_ENTERPRISE_RO_DCs";
        sidToString[L"RS"] = L"SDDL_RAS_SERVERS";
        sidToString[L"RU"] = L"SDDL_ALIAS_PREW2KCOMPACC";
        sidToString[L"SA"] = L"SDDL_SCHEMA_ADMINISTRATORS";
        sidToString[L"SI"] = L"SDDL_ML_SYSTEM";
        sidToString[L"SO"] = L"SDDL_SERVER_OPERATORS";
        sidToString[L"SU"] = L"SDDL_SERVICE";
        sidToString[L"SY"] = L"SDDL_LOCAL_SYSTEM";
        sidToString[L"WD"] = L"SDDL_EVERYONE";
        sidToString[L"AC"] = L"<undocumented>";
    }

    wstring s;

    if (sidString.size() == 0)
    {
        return s;
    }

    if (sidString == TrustedInstallerSidString)
    {
        s += L"Trusted Installer";
        return s;
    }

    wstring unknownSid = L"S-1-15-3-1024-1065365936-1281604716-3511738428-1654721687-432734479-3232135806-4053264122-3456934681";
    if (sidString == unknownSid)
    {
        s += L"Cap";
        return s;
    }

    s += L"(";
    s += sidString;
    s += L") ";
    if (sidToString.find(sidString) != sidToString.end())
    {
        s += sidToString[sidString];
        return s;
    }

    PSID sid;
    if (ConvertStringSidToSid(sidString.c_str(), &sid))
    {
        DWORD nameLen = 0;
        DWORD domainLen = 0;
        SID_NAME_USE sidNameUse;

        if (!LookupAccountSid(NULL /*machine name*/, sid, NULL /*name*/, &nameLen, NULL /*domain*/, &domainLen, &sidNameUse))
        {
            TRACEP(L"LookupAccountSid()-1 failed with error code = ", GetLastError());
        }
        else
        {
            vector<wchar_t> name(nameLen);
            vector<wchar_t> domain(domainLen);

            if (!LookupAccountSid(NULL /*machine name*/, sid, name.data(), &nameLen, domain.data(), &domainLen, &sidNameUse))
            {
                TRACEP(L"LookupAccountSid()-2 failed with error code = ", GetLastError());
            }
            else
            {
                if (domain.size() > 1)
                {
                    s += wstring(domain.data());
                    s += L"\\";
                }
                if (name.size() > 1)
                {
                    s += wstring(name.data());
                }
            }
        }
    }

    return s;
}

void PermissionsTracer::TraceDaclString(const wstring& daclString)
{
    TRACE(L"daclFlags =");

    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa374928(v=vs.85).aspx
    wchar_t* aceFieldNames[] =
    {
        L"Type . . . . . . . ",
        L"Flags  . . . . . . ",
        L"Rights . . . . . . ",
        L"Object GUID  . . . ",
        L"Inherit Object GUID",
        L"Account SID  . . . ",
        L"Resource Attribute "
    };

    vector<vector<wstring>> aceList;
    wstring attributes;
    bool acesStarted = false;
    wstring permission;
    vector<wstring> ace;
    for (size_t i = 0; i < daclString.length(); ++i)
    {
        switch (daclString[i])
        {
        case L'(':
            acesStarted = true;
            break;
        case L')':
            if (ace.size() != 0)
            {
                ace.push_back(permission);
                permission.clear();

                aceList.emplace_back(ace);
                ace.clear();
            }
            break;
        default:
            if (!acesStarted)
            {
                attributes += daclString[i];
            }
            else
            {
                if (daclString[i] == ';')
                {
                    ace.push_back(permission);
                    permission.clear();
                }
                else
                {
                    permission += daclString[i];
                }
            }
        }
    }

    TRACEP(L"    Attributes : ", attributes.c_str());
    TRACE(L"    ------------------------------");
    for (vector<vector<wstring>>::iterator aceListIt = aceList.begin(); aceListIt < aceList.end(); ++aceListIt)
    {
        size_t i = 0;
        for (vector<wstring>::iterator aceIt = aceListIt->begin(); aceIt < aceListIt->end(); ++aceIt)
        {
            TRACE(aceFieldNames[i]);
            switch (i)
            {
            case 0:
                TRACE(TypeToString(*aceIt).c_str());
                break;
            case 1:
                TRACE(FlagsToString(*aceIt).c_str());
                break;
            case 2:
                TRACE(RightsToString(*aceIt).c_str());
                break;
            case 5:
                TRACE(SidToAccountString(*aceIt).c_str());
                break;
            default:
                TRACE(aceIt->c_str());
            }
            ++i;
        }
        TRACE(L"    ------------------------------");
    }
}
