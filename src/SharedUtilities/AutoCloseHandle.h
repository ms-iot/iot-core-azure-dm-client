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

#include <windows.h>
#include <utility>
#include "AutoCloseBase.h"

namespace Utils
{
    class AutoCloseHandle : public AutoCloseBase<HANDLE>
    {
    public:
        AutoCloseHandle() :
            AutoCloseBase(NULL, [](HANDLE h) { CloseHandle(h); return TRUE; })
        {}

        AutoCloseHandle(HANDLE&& handle) :
            AutoCloseBase(std::move(handle), [](HANDLE h) { CloseHandle(h); return TRUE; })
        {}

    private:
        AutoCloseHandle(const AutoCloseHandle &);            // prevent copy
        AutoCloseHandle& operator=(const AutoCloseHandle&);  // prevent assignment
    };
}