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

namespace Utils
{

    template<class T>
    class AutoCloseBase
    {
    public:

        AutoCloseBase(T&& handle, const std::function<BOOL(T)>& cleanUp) :
            _handle(handle),
            _cleanUp(cleanUp)
        {
            handle = NULL;
        }

        void SetHandle(T&& handle)
        {
            _handle = handle;
            handle = NULL;
        }

        T Get() { return _handle; }
        uint64_t Get64() { return reinterpret_cast<uint64_t>(_handle); }
        T* GetAddress() { return &_handle; }

        BOOL Close()
        {
            BOOL result = TRUE;
            if (_handle != NULL && _cleanUp)
            {
                result = _cleanUp(_handle);
                _handle = NULL;
            }
            return result;
        }

        ~AutoCloseBase()
        {
            Close();
        }

    private:
        AutoCloseBase(const AutoCloseBase &);            // prevent copy
        AutoCloseBase& operator=(const AutoCloseBase&);  // prevent assignment

        T _handle;
        std::function<BOOL(T)> _cleanUp;
    };
}