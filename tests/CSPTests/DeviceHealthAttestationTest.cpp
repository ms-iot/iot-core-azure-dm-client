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
#include <string>
#include <vector>
#include <iostream>
#include "..\..\src\SharedUtilities\DMException.h"
#include "..\..\src\SharedUtilities\Utils.h"
#include "..\..\src\SystemConfigurator\CSPs\DeviceHealthAttestationCSP.h"
#include "DeviceHealthAttestationTest.h"
#include "TestUtils.h"

using namespace std;

class EndpointContext
{
public:
    EndpointContext()
    {
        savedEndpoint = DeviceHealthAttestationCSP::GetHASEndpoint();
    }

    ~EndpointContext()
    {
        DeviceHealthAttestationCSP::SetHASEndpoint(savedEndpoint);
    }
private:
    wstring savedEndpoint;
};


void DeviceHealthAttestationTest::GetSetHasEndpointTest(const wstring& endpointValue)
{
    TRACE(__FUNCTION__);

    EndpointContext context;
    DeviceHealthAttestationCSP::SetHASEndpoint(endpointValue);
    wstring actualEndpoint = DeviceHealthAttestationCSP::GetHASEndpoint();
    Test::Utils::EnsureEqual(actualEndpoint, endpointValue, L"Endpoints mismatched");
}

void DeviceHealthAttestationTest::GetSetNonceTest(const wstring& nonceValue)
{
    TRACE(__FUNCTION__);

    DeviceHealthAttestationCSP::SetNonce(nonceValue);
    wstring actualNonce = DeviceHealthAttestationCSP::GetNonce();

    Test::Utils::EnsureEqual(actualNonce, nonceValue, L"Nonce mismatched");
}

void DeviceHealthAttestationTest::SetForceRetrieveTest()
{
    TRACE(__FUNCTION__);

    // We can only ensure the following calls don't fail.
    DeviceHealthAttestationCSP::SetForceRetrieve(false);
    DeviceHealthAttestationCSP::SetForceRetrieve(true);
}

void DeviceHealthAttestationTest::GetCorrelationIdTest()
{
    TRACE(__FUNCTION__);
    
    // Ensure the id is non-empty
    wstring id = DeviceHealthAttestationCSP::GetCorrelationId();
    wcout << L"CorrelationId: " << id << endl;
    Test::Utils::EnsureNotEmpty(id, L"Got null correlation id");
}

void DeviceHealthAttestationTest::GetStatusTest()
{
    TRACE(__FUNCTION__);

    // We can only ensure the following calls don't fail.
    wcout << L"GetStatus: " << DeviceHealthAttestationCSP::GetStatus() << endl;
}

bool DeviceHealthAttestationTest::RunTest()
{
    bool result = true;
    try
    {
        GetSetHasEndpointTest(L"dummy.endpoint.com");
        GetSetNonceTest(L"aaaaaaaa");
        GetSetNonceTest(L"ffffffffffffffff");
        SetForceRetrieveTest();

        bool tpmEnabled = true;
        try
        {
            DeviceHealthAttestationCSP::GetTpmReadyStatus();
        }
        catch (exception)
        {
            tpmEnabled = false;
        }

        if (tpmEnabled)
        {
            // These functions requires TPM
            GetCorrelationIdTest();
            GetStatusTest();
        }
        else
        {
            // Ensure these functions throws without a TPM
            Test::Utils::EnsureException<exception>("DeviceHealthAttestationCSP::GetCorrelationId()", []() { DeviceHealthAttestationCSP::GetCorrelationId();  });
            Test::Utils::EnsureException<exception>("DeviceHealthAttestationCSP::GetStatus()", []() { DeviceHealthAttestationCSP::GetStatus();  });
        }
    }
    catch (DMException& e)
    {
        TRACEP("Error: ", e.what());

        cout << "Error: " << e.what() << endl;
        result = false;
    }
    catch (exception e)
    {
        TRACEP("Error: ", e.what());

        cout << "Error: " << e.what() << endl;
        result = false;
    }

    return result;
}
