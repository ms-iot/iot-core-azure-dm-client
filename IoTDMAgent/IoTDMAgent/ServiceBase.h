#pragma once

#include <windows.h>
#include <string>

class CServiceBase
{
public:

    CServiceBase(const std::wstring& serviceName);

    static void Run(CServiceBase &service);

protected:

    virtual void OnStart(DWORD argc, LPWSTR *lpszArgv);
    virtual void OnStop();

    void SetServiceStatus(DWORD currentState, DWORD win32ExitCode = NO_ERROR);

    void WriteEventLogEntry(const std::wstring& message, WORD type);
    void WriteErrorLogEntry(const std::wstring& function, DWORD errorCode = GetLastError());

private:

    static void WINAPI ServiceMain(DWORD argc, LPWSTR *lpszArgv);
    static void WINAPI ServiceCtrlHandler(DWORD ctrl);
    void Start(DWORD argc, PWSTR *argv);
    void Stop();
    void Pause();
    void Continue();
    void Shutdown();

    static CServiceBase *s_service;

    std::wstring _name;
    SERVICE_STATUS _status;
    SERVICE_STATUS_HANDLE _statusHandle;
};