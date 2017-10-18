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
#include "PolicyHelper.h"
#include "../DMShared/ErrorCodes.h"
#include "Constants.h"
#include "Utils.h"
#include "DMException.h"

using namespace std;
using namespace Microsoft::Devices::Management::Message;

namespace Utils
{
    const wchar_t* SourceLocal = L"local";
    const wchar_t* SourceRemote = L"remote";
    const wchar_t* SourceUnknown = L"unknown";

    wstring PolicyHelper::PolicyToRegString(PolicySource source)
    {
        switch (source)
        {
        case PolicySource::Local:
            return SourceLocal;
        case PolicySource::Remote:
            return SourceRemote;
        }
        return SourceUnknown;
    }

    PolicySource PolicyHelper::RegStringToPolicy(const wstring& source)
    {
        if (source == SourceLocal)
        {
            return PolicySource::Local;
        }
        else if (source == SourceRemote)
        {
            return PolicySource::Remote;
        }
        return PolicySource::Unknown;
    }

    void PolicyHelper::SaveToRegistry(Policy^ policy, const wstring& regSectionRoot)
    {
        if (!policy)
        {
            return;
        }

        wstring sourcePriorities;
        if (policy->sourcePriorities)
        {
            for each (PolicySource policySource in policy->sourcePriorities)
            {
                if (sourcePriorities.length() > 0)
                {
                    sourcePriorities += RegPolicySeparator;
                }
                sourcePriorities += PolicyHelper::PolicyToRegString(policySource);
            }
        }
        Utils::WriteRegistryValue(regSectionRoot.c_str(), RegSourcePriorities, sourcePriorities.c_str());
        Utils::WriteRegistryValue(regSectionRoot.c_str(), RegPolicySource, PolicyToRegString(policy->source));
    }

    Policy^ PolicyHelper::LoadFromRegistry(const wstring& regSectionRoot)
    {
        wstring sourcePrioritiesString;
        if (ERROR_SUCCESS != Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegSourcePriorities, sourcePrioritiesString))
        {
            return nullptr;
        }

        wstring policySourceString;
        if (ERROR_SUCCESS != Utils::TryReadRegistryValue(regSectionRoot.c_str(), RegPolicySource, policySourceString))
        {
            return nullptr;
        }

        vector<wstring> tokens;
        Utils::SplitString(sourcePrioritiesString, RegPolicySeparator, tokens);

        Policy^ policy = ref new Policy();
        policy->source = RegStringToPolicy(policySourceString);
        policy->sourcePriorities = ref new Vector<PolicySource>();
        for (auto& t : tokens)
        {
            PolicySource priPolicySource = RegStringToPolicy(t);
            if (priPolicySource == PolicySource::Unknown)
            {
                throw DMExceptionWithErrorCode(ERROR_DM_INVALID_POLICY_SOURCE);
            }
            policy->sourcePriorities->Append(priPolicySource);
        }

        return policy;
    }
}