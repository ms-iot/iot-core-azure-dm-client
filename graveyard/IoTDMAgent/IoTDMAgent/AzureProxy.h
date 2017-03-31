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

#include <string>
#include "TaskQueue.h"
#include "iothub_client.h"

class AzureProxy
{
public:
    AzureProxy(TaskQueue* taskQueue);
    ~AzureProxy();

    void Connect(const std::string& connectionString);
    void Disconnect();

private:

    // Azure callbacks
    static void OnReportedPropertiesSent(int status_code, void* userContextCallback);
    static void OnDesiredProperties(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payload, size_t size, void* userContextCallback);
    static IOTHUBMESSAGE_DISPOSITION_RESULT OnMessageReceived(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback);
    static int OnMethodCalled(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback);

    // Helpers
    void ProcessMessage(const std::string& command);
    int ProcessMethodCall(const std::string& name, const std::string& payload, std::string& response);
    void ProcessReportAllCall();
    std::string ProcessRebootCall();
    std::string ProcessRemoteWipe();

    void ReportProperties(std::string allJson) const;

    // Data members
    IOTHUB_CLIENT_HANDLE _iotHubClientHandle;
    TaskQueue* _taskQueue;
};


