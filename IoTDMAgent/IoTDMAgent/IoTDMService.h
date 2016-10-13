#pragma once
#include <thread>
#include <atomic>
#include <future>

class IoTDMService
{
public:

    IoTDMService(const std::wstring& serviceName);

    static void IoTDMService::Install(
        const std::wstring&  serviceName,
        const std::wstring&  displayName,
        DWORD startType,
        const std::wstring&  dependencies,
        const std::wstring&  account,
        const std::wstring&  password);

    static void IoTDMService::Uninstall(const std::wstring&  serviceName);

    static void Run(IoTDMService &service);

private:

    // Methods
    static void WINAPI ServiceMain(DWORD argc, LPWSTR *argv);
    static void WINAPI ServiceCtrlHandler(DWORD ctrl);

    static void ServiceWorkerThread(void* context);
    void ServiceWorkerThreadHelper();

    void Start(DWORD argc, PWSTR *argv);
    void Stop();
    void Pause();
    void Continue();
    void Shutdown();

    virtual void OnStart(DWORD argc, LPWSTR *lpszArgv);
    virtual void OnStop();

    // Helpers
    void SetServiceStatus(DWORD currentState, DWORD win32ExitCode = NO_ERROR);

    void WriteEventLogEntry(const std::wstring& message, WORD type);
    void WriteErrorLogEntry(const std::wstring& function, DWORD errorCode = GetLastError());

    std::string GetConnectionString(const std::chrono::system_clock::time_point& expiration);

    // Data members
    static IoTDMService* s_service;

    std::wstring _name;
    SERVICE_STATUS _status;
    SERVICE_STATUS_HANDLE _statusHandle;

    // Synchronization between worker thread and main thread for exiting...
    std::atomic<bool> _stopSignaled;
    std::promise<void> _stoppedPromise;
};