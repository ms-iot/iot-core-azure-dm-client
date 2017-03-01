#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <deque>
#include <thread>
#include <time.h>
#include <sstream>
#include <windows.h>

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
    void WriteRegistryValue(const std::wstring& subkey, const std::wstring& propName, const std::wstring& propValue);
    std::wstring ReadRegistryValue(const std::wstring& subkey, const std::wstring& propName);

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

    class AutoCloseHandle
    {
    public:
        AutoCloseHandle() :
            _handle(NULL)
        {}

        AutoCloseHandle(HANDLE&& handle) :
            _handle(handle)
        {
            handle = NULL;
        }

        HANDLE operator=(HANDLE&& handle)
        {
            _handle = handle;
            handle = NULL;
            return _handle;
        }

        HANDLE Get() { return _handle; }
        uint64_t Get64() { return reinterpret_cast<uint64_t>(_handle); }
        HANDLE* GetAddress() { return &_handle; }

        BOOL Close()
        {
            BOOL result = TRUE;
            if (_handle != NULL)
            {
                result = CloseHandle(_handle);
                _handle = NULL;
            }
            return result;
        }

        ~AutoCloseHandle()
        {
            Close();
        }

    private:
        AutoCloseHandle(const AutoCloseHandle &);            // prevent copy
        AutoCloseHandle& operator=(const AutoCloseHandle&);  // prevent assignment

        HANDLE _handle;
    };

    void LoadFile(const std::wstring& fileName, std::vector<char>& buffer);
    std::wstring ToBase64(std::vector<char>& buffer);
    std::wstring FileToBase64(const std::wstring& fileName);
}