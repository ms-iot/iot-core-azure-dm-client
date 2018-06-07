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
#include <thread>
#include <queue>
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"
#include "..\TaskQueue.h"
#include "PrivateAPIs\WinSDKRS2.h"
#include "..\resource.h"
#include "MdmProvision.h"

#define ROOT_XML L"SyncML\\SyncBody"
#define ROOT_START_TAG L"<" ROOT_XML L">"
#define ROOT_END_TAG L"</" ROOT_XML L">"
#define STATUS_XML_PATH ROOT_XML L"\\Status\\Data\\"
#define RESULTS_XML_PATH ROOT_XML L"\\Results\\Item\\Data\\"

using namespace std;

bool MdmProvision::s_errorVerbosity = false;

class SyncMLServer
{
public:
    SyncMLServer()
    {
        TRACE(__FUNCTION__);
        _workerThread = thread(ServiceWorkerThread, this);
    }

    void Process(const wstring& requestSyncML, wstring& outputSyncML)
    {
        TRACE(__FUNCTION__);
        TaskQueue::Task task([&]()
        {
            return ProcessInternal(requestSyncML);
        });

        future<wstring> futureResult = _queue.Enqueue(move(task));

        // This will block until the result is available...
        outputSyncML = futureResult.get();
    }

private:

    static void ServiceWorkerThread(void* context)
    {
        TRACE(__FUNCTION__);

        SyncMLServer* syncMLServer = static_cast<SyncMLServer*>(context);
        syncMLServer->Listen();
    }

    void Listen()
    {
        TRACE(__FUNCTION__);

        while (true)
        {
            TRACE("Worker thread waiting for a task to be queued...");
            TaskQueue::Task task = _queue.Dequeue();

            TRACE("A task has been dequeued...");
            task();

            TRACE("Task has completed.");
        }
    }

    wstring ProcessInternal(const wstring& requestSyncML)
    {
        TRACE(__FUNCTION__);

        TRACEP(L"SyncMLServer - Request : ", requestSyncML.c_str());

        HRESULT hr = RegisterDeviceWithLocalManagement(NULL);
        if (FAILED(hr))
        {
            throw DMExceptionWithErrorCode("RegisterDeviceWithLocalManagement", hr);
        }

        PWSTR output = nullptr;
        hr = ApplyLocalManagementSyncML(requestSyncML.c_str(), &output);
        if (FAILED(hr))
        {
            throw DMExceptionWithErrorCode("ApplyLocalManagementSyncML", hr);
        }

        wstring outputSyncML;
        if (output)
        {
            outputSyncML = output;
        }
        LocalFree(output);

        return outputSyncML;
    }

    thread _workerThread;
    TaskQueue _queue;
};

void MdmProvision::SetErrorVerbosity(bool verbosity) noexcept
{
    s_errorVerbosity = verbosity;
}

void MdmProvision::RunSyncML(const wstring&, const wstring& requestSyncML, wstring& outputSyncML)
{
    TRACEP(L"Request : ", requestSyncML.c_str());

    static SyncMLServer syncMLServer;
    syncMLServer.Process(requestSyncML, outputSyncML);

    TRACEP(L"Response: ", outputSyncML.c_str());

    vector<wstring> resultCodes;
    Utils::ReadXmlValues(outputSyncML, STATUS_XML_PATH, resultCodes);

    // Check the result of all reported commands...
    for (const wstring& s : resultCodes)
    {
        unsigned int returnCode = stoi(s);
        if (returnCode >= 300)
        {
            ReportError(requestSyncML, outputSyncML, returnCode);
            throw DMExceptionWithErrorCode(returnCode);
        }
    }
}

void MdmProvision::RunAdd(const wstring& sid, const wstring& path, const wstring& value)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Add>
                <CmdID>1</CmdID>
                <Item>
                    <Target>
                        <LocURI>)";
    requestSyncML += path;
    requestSyncML += L"/";
    requestSyncML += value;
    requestSyncML += LR"(</LocURI>
                    </Target>
                </Item>
            </Add>
        </SyncBody>
        )";

    wstring resultSyncML;
    try
    {
        RunSyncML(sid, requestSyncML, resultSyncML);
    }
    catch (const DMExceptionWithErrorCode& e)
    {
        if (e.ErrorCode() != OMA_DM_ERROR_ALREADY_EXISTS)
        {
            throw;
        }
    }
}

void MdmProvision::RunAddData(const wstring& sid, const wstring& path, const wstring& value, const wstring& type)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Add>
                <CmdID>1</CmdID>
                <Item>
                    <Target>
                        <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                    </Target>
                    <Meta>
                        <Format xmlns="syncml:metinf">)";
    requestSyncML += type;
    requestSyncML += LR"(</Format>
                    </Meta>
                    <Data>)";
    requestSyncML += value;
    requestSyncML += LR"(</Data>
                </Item>
            </Add>
        </SyncBody>
        )";

    wstring resultSyncML;

    try
    {
        RunSyncML(sid, requestSyncML, resultSyncML);
    }
    catch (const DMExceptionWithErrorCode& e)
    {
        if (e.ErrorCode() != OMA_DM_ERROR_ALREADY_EXISTS)
        {
            throw;
        }
    }
}

void MdmProvision::RunAddTyped(const wstring& sid, const wstring& path, const wstring& type)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Add>
                <CmdID>1</CmdID>
                <Item>
                    <Target>
                        <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                    </Target>
                    <Meta>
                        <Format xmlns="syncml:metinf">)";
    requestSyncML += type;
    requestSyncML += LR"(</Format>
                    </Meta>
                </Item>
            </Add>
        </SyncBody>
        )";

    wstring resultSyncML;

    try
    {
        RunSyncML(sid, requestSyncML, resultSyncML);
    }
    catch (const DMExceptionWithErrorCode& e)
    {
        if (e.ErrorCode() != OMA_DM_ERROR_ALREADY_EXISTS)
        {
            throw;
        }
    }
}

void MdmProvision::RunAddDataBase64(const wstring& sid, const std::wstring& path, const std::wstring& value)
{
    RunAddData(sid, path, value, L"b64");
}

void MdmProvision::RunDelete(const std::wstring& sid, const std::wstring& path)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Delete>
                <CmdID>1</CmdID>
                <Item>
                    <Target>
                        <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                    </Target>
                </Item>
            </Delete>
        </SyncBody>
        )";

    TRACEP(L"RunDelete : ", requestSyncML.c_str());

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);
}

wstring MdmProvision::RunGetString(const wstring& sid, const wstring& path)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Get>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                </Target>
                <Meta>
                    <Type xmlns="syncml:metinf">text/plain</Type>
                </Meta>
              </Item>
            </Get>
        </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);

    wstring value;
    Utils::ReadXmlValue(resultSyncML, RESULTS_XML_PATH, value);
    return value;
}

std::wstring MdmProvision::RunGetBase64(const std::wstring& sid, const std::wstring& path)
{
    // http://www.openmobilealliance.org/tech/affiliates/syncml/syncml_metinf_v101_20010615.pdf
    // Section 5.3.

    wstring requestSyncML = LR"(
        <SyncBody>
            <Get>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                </Target>
                <Meta>
                    <Type xmlns="syncml:metinf">b64</Type>
                </Meta>
              </Item>
            </Get>
        </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);

    wstring value;
    Utils::ReadXmlValue(resultSyncML, RESULTS_XML_PATH, value);
    return value;
}

void MdmProvision::RunGetStructData(const std::wstring& path, Utils::ELEMENT_HANDLER handler)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Get>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                </Target>
                <Meta>
                    <Type xmlns="syncml:metinf">text/plain</Type>
                </Meta>
              </Item>
            </Get>
        </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(L"", requestSyncML, resultSyncML);

    // Extract the result data
    Utils::ReadXmlStructData(resultSyncML, handler);
}

unsigned int MdmProvision::RunGetUInt(const wstring& sid, const wstring& path)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Get>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    requestSyncML += path.c_str();
    requestSyncML += LR"(</LocURI>
                </Target>
                <Meta>
                    <Format xmlns="syncml:metinf">int</Format>
                </Meta>
              </Item>
            </Get>
        </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);

    // Extract the result data
    wstring valueString;
    Utils::ReadXmlValue(resultSyncML, RESULTS_XML_PATH, valueString);
    return stoi(valueString);
}

bool MdmProvision::RunGetBool(const wstring& sid, const wstring& path)
{
    wstring result = RunGetString(sid, path);

    return 0 == _wcsicmp(result.c_str(), L"true");
}

bool MdmProvision::TryGetBool(const wstring& path, bool& value)
{
    bool success = true;
    try
    {
        value = RunGetBool(L"", path);
    }
    catch (DMException& e)
    {
        success = false;
        TRACEP(L"Error: GetBool() - path     : ", path.c_str());
        TRACEP("Error: GetBool() - exception: ", e.what());
    }
    return success;
}

void MdmProvision::RunSet(const wstring& sid, const wstring& path, const wstring& value)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Replace>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                </Target>
                <Meta>
                    <Type xmlns="syncml:metinf">text/plain</Type>
                </Meta>
                <Data>)";
    requestSyncML += value;
    requestSyncML += LR"(</Data>
              </Item>
            </Replace>
        </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);
}

void MdmProvision::RunSetBase64(const wstring& sid, const std::wstring& path, const std::wstring& value)
{
    // http://www.openmobilealliance.org/tech/affiliates/syncml/syncml_metinf_v101_20010615.pdf
    // Section 5.3.

    wstring requestSyncML = LR"(
        <SyncBody>
            <Replace>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                </Target>
                <Meta>
                    <Type xmlns="syncml:metinf">b64</Type>
                </Meta>
                <Data>)";
    requestSyncML += value;
    requestSyncML += LR"(</Data>
              </Item>
            </Replace>
        </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);
}

void MdmProvision::RunSet(const wstring& sid, const wstring& path, int value)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Replace>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                </Target>
                <Meta><Format xmlns="syncml:metinf">int</Format></Meta>
                <Data>)";
    requestSyncML += to_wstring(value);
    requestSyncML += LR"(</Data>
              </Item>
            </Replace>
        </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);
}

void MdmProvision::RunSet(const wstring& sid, const wstring& path, bool value)
{
    wstring requestSyncML = LR"(
        <SyncBody>
            <Replace>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                </Target>
                <Meta><Format xmlns="syncml:metinf">bool</Format></Meta>
                <Data>)";
    requestSyncML += value ? L"True" : L"False";
    requestSyncML += LR"(</Data>
              </Item>
            </Replace>
        </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);
}

void MdmProvision::RunExec(const wstring& sid, const wstring& path)
{
    wstring requestSyncML = LR"(
    <SyncBody>
        <Exec>
            <CmdID>1</CmdID>
            <Item>
                <Target>
                    <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                </Target>
            </Item>
        </Exec>
    </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);
}

void MdmProvision::RunExecWithParameters(const wstring& sid, const wstring& path, const wstring& params)
{
    wstring requestSyncML = LR"(
    <SyncBody>
        <Exec>
            <CmdID>1</CmdID>
            <Item>
                <Target>
                    <LocURI>)";
    requestSyncML += path;
    requestSyncML += LR"(</LocURI>
                </Target>
                <Meta>
                    <Format xmlns="syncml:metinf">chr</Format>
                </Meta>
                <Data>)";
    requestSyncML += params;
    requestSyncML += LR"(</Data>
            </Item>
        </Exec>
    </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);
}

void MdmProvision::RunAdd(const wstring& path, const wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    RunAdd(L"", path, value);
}

void MdmProvision::RunAddData(const std::wstring& path, int value)
{
    // empty sid is okay for device-wide CSPs.
    RunAddData(L"", path, Utils::MultibyteToWide(to_string(value).c_str()), L"int");
}

void MdmProvision::RunAddTyped(const wstring& path, const wstring& type)
{
    RunAddTyped(L"", path, type);
}

void MdmProvision::RunAddData(const std::wstring& path, bool value)
{
    // empty sid is okay for device-wide CSPs.
    RunAddData(L"", path, Utils::MultibyteToWide(to_string(value).c_str()), L"bool");
}

void MdmProvision::RunAddData(const wstring& path, const wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    RunAddData(L"", path, value);
}

void MdmProvision::RunAddDataBase64(const wstring& path, const wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    RunAddDataBase64(L"", path, value);
}

void MdmProvision::RunDelete(const std::wstring& path)
{
    // empty sid is okay for device-wide CSPs.
    RunDelete(L"", path);
}

wstring MdmProvision::RunGetString(const wstring& path)
{
    // empty sid is okay for device-wide CSPs.
    return RunGetString(L"", path);
}

bool MdmProvision::TryGetString(const wstring& path, wstring& value)
{
    bool success = true;
    try
    {
        value = RunGetString(path);
    }
    catch (DMException& e)
    {
        success = false;
        TRACEP(L"Error: GetString() - path     : ", path.c_str());
        TRACEP("Error: GetString() - exception: ", e.what());
    }
    return success;
}

wstring MdmProvision::RunGetBase64(const wstring& path)
{
    // empty sid is okay for device-wide CSPs.
    return RunGetBase64(L"", path);
}

unsigned int MdmProvision::RunGetUInt(const wstring& path)
{
    // empty sid is okay for device-wide CSPs.
    return RunGetUInt(L"", path);
}

bool MdmProvision::RunGetBool(const wstring& path)
{
    // empty sid is okay for device-wide CSPs.
    return RunGetBool(L"", path);
}

void MdmProvision::RunSet(const wstring& path, const wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    RunSet(L"", path, value);
}

void MdmProvision::RunSet(const wstring& path, int value)
{
    // empty sid is okay for device-wide CSPs.
    RunSet(L"", path, value);
}

void MdmProvision::RunSet(const wstring& path, bool value)
{
    // empty sid is okay for device-wide CSPs.
    RunSet(L"", path, value);
}

void MdmProvision::RunSetBase64(const std::wstring& path, const std::wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    RunSetBase64(L"", path, value);
}

void MdmProvision::RunExec(const wstring& path)
{
    // empty sid is okay for device-wide CSPs.
    RunExec(L"", path);
}

void MdmProvision::RunExecWithParameters(const std::wstring& path, const std::wstring& params)
{
    RunExecWithParameters(L"", path, params);
}

void MdmProvision::ReportError(const wstring& syncMLRequest, const wstring& syncMLResponse, int errorCode)
{
    if (s_errorVerbosity)
    {
        TRACE(L"Error:\n\n");
        TRACEP(L"Request:\n", syncMLRequest.c_str());
        TRACEP(L"Response:\n", syncMLResponse.c_str());
        TRACEP(L"Error:\n", errorCode);
    }
    else
    {
        TRACEP(L"Error:\n", errorCode);
    }
}

void MdmProvision::ReportError(const wstring& syncMLRequest, const wstring& syncMLResponse)
{
    TRACE(L"Error:\n\n");
    TRACEP(L"Request:\n", syncMLRequest.c_str());
    TRACEP(L"Response:\n", syncMLResponse.c_str());
}
