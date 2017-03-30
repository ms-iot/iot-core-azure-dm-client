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
#include <sstream>
#include <windows.h>
#include "Utils.h"
#include "LimpetLib.h"
#include "Limpet.h"

typedef UINT32 TPM_RC;
#define    TPM_RC_SUCCESS              (TPM_RC)(0x000)

Limpet & Limpet::GetInstance()
{
    // One and only Magic statics instance
    static Limpet instance;
    return instance;
}

Limpet::Limpet()
{
    TRACE("Limpet.ctor()");
    auto result = _LimpetInitialize();
    if (result != TPM_RC_SUCCESS)
    {
        Limpet::ThrowException("TPM initialize error", result);
    }
}

Limpet::~Limpet()
{
    TRACE("Limpet.dtor()");
    _LimpetDestroy();
}

std::string Limpet::GetSASToken(UINT deviceNumber, const std::chrono::system_clock::time_point& expiration)
{
    using namespace std::chrono;

    TRACE("Limpet.GetSASToken()");

    LARGE_INTEGER expirationLargeInteger;
    expirationLargeInteger.QuadPart = duration_cast<seconds>(expiration.time_since_epoch()).count();

    std::wstring output(LIMPET_STRING_SIZE, L'\0');
    auto result = _LimpetGenerateSASToken(deviceNumber, &expirationLargeInteger, &output[0]);
    if (result != TPM_RC_SUCCESS)
    {
        ThrowException("Error generating SAS Token", result);
    }
    output.resize(output.find(L'\0'));
    return Utils::WideToMultibyte(output.c_str());
}

void Limpet::ThrowException(const char * message, UINT32 errorCode)
{
    std::ostringstream ss;
    ss << message << " (ErrorCode: 0x" << std::hex << errorCode << ")";
    throw DMException(ss.str().c_str());
}

