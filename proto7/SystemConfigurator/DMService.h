#pragma once
#include <thread>
#include <atomic>
#include <future>
#include "..\SharedUtilities\Utils.h"

class DMService
{
public:

    DMService(const std::wstring& serviceName);

    static void DMService::Install(
        const std::wstring&  serviceName,
        const std::wstring&  displayName,
        DWORD startType,
        const std::wstring&  dependencies,
        const std::wstring&  account,
        const std::wstring&  password);

    static void DMService::Uninstall(const std::wstring&  serviceName);

    static void Run(DMService &service);

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

    void DisableEnqueue();

    // Helpers
    void SetServiceStatus(DWORD currentState, DWORD win32ExitCode = NO_ERROR);

    void WriteEventLogEntry(const std::wstring& message, WORD type);
    void WriteErrorLogEntry(const std::wstring& function, DWORD errorCode = GetLastError());

    // Data members
    static DMService* s_service;

    std::wstring _name;
    SERVICE_STATUS _status;
    SERVICE_STATUS_HANDLE _statusHandle;

    // threads
    Utils::JoiningThread _workerThread;
    // Utils::JoiningThread _connectionRenewerThread;

    // TaskQueue _taskQueue;

    // Synchronization between worker thread and main thread for exiting...
    std::atomic<bool> _stopSignaled;
    bool _renewConnectionString;

    // Azure proxy
    // AzureProxy _cloudProxy;

    // Models
    // ModelManager _modelManager;
};