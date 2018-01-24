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
#pragma once

#include <string>

class Tpm
{
public:
    static std::string GetEndorsementKey();
    static std::string GetSRootKey();
    static void DestroyServiceUrl(int logicalId);
    static void StoreServiceUrl(int logicalId, const std::string& url);
    static void ImportSymetricIdentity(int logicalId, const std::string& hostageFile);
    static void EvictHmacKey(int logicalId);
    static std::string GetServiceUrl(int logicalId);
    static std::string GetSASToken(int logicalId, unsigned int durationInSeconds);
    static void ClearTPM();
private:
    static std::string Tpm::RunLimpet(const std::wstring& params);
};
