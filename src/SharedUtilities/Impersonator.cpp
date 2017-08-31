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
#include <windows.h>
#include <tlhelp32.h>

#include "Utils.h"
#include "Logger.h"
#include "Impersonator.h"

Impersonator::Impersonator() :
    _snapshot(nullptr),
    _hProcess(nullptr),
    _hToken(nullptr)
{
}

Impersonator::~Impersonator()
{
    Close();
}

bool Impersonator::ImpersonateShellHost()
{
    TRACE("Impersonating...");

    Close();

    bool result = false;
    Utils::GetDmUserInfo([&result](HANDLE token, PTOKEN_USER /*tokenUser*/) {
        if (!ImpersonateLoggedOnUser(token))
        {
            TRACE("Error: Failed to impersonate user...");
        }
        else
        {
            result = true;
        }
    });

    if (!result)
    {
        Close();
    }
    if (result)
    {
        TRACE("Impersonating succeeded!");
    }
    else
    {
        TRACE("Impersonating failed!");
    }

    return result;
}

void Impersonator::Close()
{
    RevertToSelf();

    if (_hToken)
    {
        CloseHandle(_hToken);
        _hToken = nullptr;
    }

    if (_hProcess)
    {
        CloseHandle(_hProcess);
        _hProcess = nullptr;
    }

    if (_snapshot)
    {
        CloseHandle(_snapshot);
        _snapshot = nullptr;
    }
}
