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

// ToDo: This can be moved into either AppCfg.h or EnteprriseModernAppManagementCSP.h once we decide on one of them.

struct ApplicationInfo
{
    std::wstring packageFamilyName;
    std::wstring name;
    std::wstring version;
    std::wstring installDate;
    std::wstring errorMessage;
    int errorCode;
    bool pending;

    ApplicationInfo() :
        errorCode(0),
        pending(false)
    {}

    ApplicationInfo(const std::wstring& pkgFamilyName) :
        packageFamilyName(pkgFamilyName),
        errorCode(0),
        pending(false)
    {}

    ApplicationInfo(const std::wstring& pkgFamilyName, int errCode, const std::wstring& errMessage, bool installPending) :
        packageFamilyName(pkgFamilyName),
        errorCode(errCode),
        errorMessage(errMessage),
        pending(installPending)
    {}

    ApplicationInfo(const std::wstring& pkgFamilyName, const std::wstring& appName, const std::wstring& appVersion, const std::wstring& appInstallDate, int errCode, const std::wstring& errMessage, bool installPending) :
        packageFamilyName(pkgFamilyName),
        name(appName),
        version(appVersion),
        installDate(appInstallDate),
        errorCode(errCode),
        errorMessage(errMessage),
        pending(installPending)
    {}

};
