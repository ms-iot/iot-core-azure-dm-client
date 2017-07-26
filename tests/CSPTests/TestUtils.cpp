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
#include "TestUtils.h"
#include "Utils.h"
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

namespace Test { 
    namespace Utils{

        void EnsureEqual(const wstring& actual, const wstring& expected, const wstring& errorMessage)
        {
            if (actual.compare(expected) != 0)
            {
                wstringstream ws;
                ws << errorMessage << endl;
                ws << L"\tExpected:" << expected << endl;
                ws << L"\tActual: " << actual << endl;
                wcout << ws.str();
                throw TestFailureException(::Utils::WideToMultibyte(ws.str().c_str()).c_str());
            }
        }

        void EnsureNotEmpty(const std::wstring& value, const std::wstring& errorMessage)
        {
            if (value.empty())
            {
                wcout << errorMessage << endl;
                throw TestFailureException(errorMessage.c_str());
            }
        }

} // end namespace Util
} // end namespace Test