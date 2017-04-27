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
#include <sstream>
#include <iomanip>
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\DMException.h"

class CertificateInfo
{
public:
    CertificateInfo(const std::wstring& certFile);

    std::wstring GetCertificateInBase64();
    static void SetCertificateInBase64(const std::wstring& certPath, const std::wstring& certificateInBase64);
    static void AddCertificateInBase64(const std::wstring& certPath, const std::wstring& certificateInBase64);

    std::wstring GetIssuedBy();
    static void SetIssuedBy(const std::wstring& certPath, const std::wstring& value);
    static void AddIssuedBy(const std::wstring& certPath, const std::wstring& value);

    std::wstring GetIssuedTo();
    static void SetIssuedTo(const std::wstring& certPath, const std::wstring& value);
    static void AddIssuedTo(const std::wstring& certPath, const std::wstring& value);

    std::wstring GetValidFrom();
    static void SetValidFrom(const std::wstring& certPath, const std::wstring& value);
    static void AddValidFrom(const std::wstring& certPath, const std::wstring& value);

    std::wstring GetValidTo();
    static void SetValidTo(const std::wstring& certPath, const std::wstring& value);
    static void AddValidTo(const std::wstring& certPath, const std::wstring& value);

    std::wstring GetTemplateName();
    static void SetTemplateName(const std::wstring& certPath, const std::wstring& value);
    static void AddTemplateName(const std::wstring& certPath, const std::wstring& value);

    static void AddCertificate(const std::wstring& path, const std::wstring& hash, const std::wstring& certificateInBase64);
    static void DeleteCertificate(const std::wstring& path, const std::wstring& hash);

private:
    std::wstring _certPath;
};

class CertificateFile
{
public:
    CertificateFile(const std::wstring& certFileName);

    std::wstring FullFileName() const;
    std::wstring ThumbPrint() const;

    void Install(const std::wstring& certStorePath);

private:
    std::wstring _fullFileName;
    std::wstring _thumbPrint;
};
