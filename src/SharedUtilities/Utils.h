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

#include <stdio.h>
#include <string>
#include <vector>
#include <deque>
#include <thread>
#include <time.h>
#include <sstream>
#include <windows.h>

#define IoTDMRegistryRoot L"Software\\Microsoft\\IoTDM"
#define IoTDMRegistryLastRebootCmd L"LastRebootCmd"
#define IoTDMRegistryWindowsUpdateRebootAllowed L"WindowsUpdateRebootAllowed"
#define IoTDMRegistryWindowsUpdatePolicySectionReporting L"WindowsUpdatePolicySectionReporting"
#define IoTDMRegistryTrue L"True"
#define IoTDMRegistryFalse L"False"

namespace Utils
{
    typedef std::function<void(std::vector<std::wstring>&, std::wstring&)>& ELEMENT_HANDLER;

    // Sid helper
    std::wstring GetSidForAccount(const wchar_t* userAccount);

    // String helpers
    std::string WideToMultibyte(const wchar_t* s);
    std::wstring MultibyteToWide(const char* s);

    template<class T>
    void SplitString(const std::basic_string<T> &s, T delim, std::vector<std::basic_string<T>>& tokens)
    {
        std::basic_stringstream<T> ss;
        ss.str(s);
        std::basic_string<T> item;
        while (getline<T>(ss, item, delim))
        {
            tokens.push_back(item);
        }
    }

    template<class T>
    T TrimString(const T& s, const T& chars)
    {
        T trimmedString;

        // trim leading characters
        size_t startpos = s.find_first_not_of(chars);
        if (T::npos != startpos)
        {
            trimmedString = s.substr(startpos);
        }

        // trim trailing characters
        size_t endpos = trimmedString.find_last_not_of(chars);
        if (T::npos != endpos)
        {
            trimmedString = trimmedString.substr(0, endpos + 1);
        }
        return trimmedString;
    }

    template<class CharType, class ParamType>
    std::basic_string<CharType> ConcatString(const CharType* s, ParamType param)
    {
        std::basic_ostringstream<CharType> messageStream;
        messageStream << s << param;
        return messageStream.str();
    }

    // Replaces invalid characters (like .) with _ so that the string can be used
    // as a json property name.
    std::wstring ToJsonPropertyName(const std::wstring& propertyName);

    // System helpers
    std::wstring GetCurrentDateTimeString();
    std::wstring GetDateTimeString(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute, unsigned int second);
    std::wstring GetOSVersionString();
    std::wstring GetEnvironmentVariable(const std::wstring& variableName);
    std::wstring GetSystemRootFolder();
    std::wstring GetProgramDataFolder();

    // Xml helpers
    void ReadXmlValue(const std::wstring& resultSyncML, const std::wstring& targetXmlPath, std::wstring& value);
    void ReadXmlStructData(const std::wstring& resultSyncML, ELEMENT_HANDLER handler);

    // Registry helpers
    void WriteRegistryValue(const std::wstring& subKey, const std::wstring& propName, const std::wstring& propValue);
    LSTATUS TryReadRegistryValue(const std::wstring& subKey, const std::wstring& propName, std::wstring& propValue);
    std::wstring ReadRegistryValue(const std::wstring& subKey, const std::wstring& propName);

    // File helpers
    bool FileExists(const std::wstring& fullFileName);
    void EnsureFolderExists(const std::wstring& folder);

    // Process helpers
    void LaunchProcess(const std::wstring& commandString, unsigned long& returnCode, std::string& output);

    // Threading helpers
    class JoiningThread
    {
    public:
        std::thread& operator=(std::thread&& t)
        {
            _thread = std::move(t);
            return _thread;
        }

        void Join()
        {
            if (_thread.joinable())
            {
                _thread.join();
            }
        }

        ~JoiningThread()
        {
            Join();
        }
    private:
        std::thread _thread;
    };

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

    class AutoCloseSID : public AutoCloseBase<PSID>
    {
    public:
        AutoCloseSID() :
            AutoCloseBase(NULL, [](PSID h) { CloseHandle(h); return TRUE; })
        {}

        AutoCloseSID(PSID&& handle) :
            AutoCloseBase(std::move(handle), [](PSID h) { FreeSid(h); return TRUE; })
        {}

    private:
        AutoCloseSID(const AutoCloseSID &);            // prevent copy
        AutoCloseSID& operator=(const AutoCloseSID&);  // prevent assignment
    };

    class AutoCloseACL : public AutoCloseBase<PACL>
    {
    public:
        AutoCloseACL() :
            AutoCloseBase(NULL, [](PSID h) { CloseHandle(h); return TRUE; })
        {}

        AutoCloseACL(PACL&& handle) :
            AutoCloseBase(std::move(handle), [](PACL h) { LocalFree(h); return TRUE; })
        {}

    private:
        AutoCloseACL(const AutoCloseACL &);            // prevent copy
        AutoCloseACL& operator=(const AutoCloseACL&);  // prevent assignment
    };


#if 0
    template<class T>
    class SafeResource
    {
    public:
        SafeResource(const T& sid, const std::function<void(T)>& cleanUp) :
            _sid(sid),
            _cleanUp(cleanUp)
        {
        }
        T Get() const
        {
            return _sid;
        }
        ~SafeResource()
        {
            _cleanUp(_sid);
        }
    private:
        SafeResource(const AutoCloseHandle &);            // prevent copy
        SafeResource& operator=(const AutoCloseHandle&);  // prevent assignment

        T _sid;
        std::function<void(T)> _cleanUp;
    };
#endif

    void LoadFile(const std::wstring& fileName, std::vector<char>& buffer);
    std::wstring ToBase64(std::vector<char>& buffer);
    std::wstring FileToBase64(const std::wstring& fileName);
}
