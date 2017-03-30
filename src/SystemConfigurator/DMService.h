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

    void Start();
    void Stop();
    void Pause();
    void Continue();
    void Shutdown();

    virtual void OnStart();
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

    // timer handle
    HANDLE _timerQueueHandle;

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
