#include "stdafx.h"
#include "PrivateAPIs\CSPController.h"
#include "MdmProvision.h"
#include "..\CSPSupport\MDMLocalManagement.h"

#define ROOT_XML L"Root"
#define ROOT_START_TAG L"<" ROOT_XML L">"
#define ROOT_END_TAG L"</" ROOT_XML L">"
// ToDo: we need to find the 'status' that corresponds to the command we just issued.
//       sometimes, more than one status are reported for a single command.
#define STATUS_XML_PATH L"SyncML\\SyncBody\\Status\\Data\\"
#define RESULTS_XML_PATH L"SyncML\\SyncBody\\Results\\Item\\Data\\"

using namespace std;

bool MdmProvision::s_errorVerbosity = false;

void MdmProvision::SetErrorVerbosity(bool verbosity) noexcept
{
    s_errorVerbosity = verbosity;
}

void MdmProvision::RunSyncML(const wstring& sid, const wstring& requestSyncML, wstring& outputSyncML)
{
    // Potentially two attributes (session variables) you might need.
    // 1) OMADM_TARGETEDUSERSID_VARIABLE_NAME: this is the user SID for configuration that's per-user.
    //    This example is using the DefApps SID from the Phone, which is ignored for the sample XML.
    //    Use "whoami /user" to get your SID on the desktop.
    // 2) OMADM_ACCOUNTID_VARIABLE_NAME: this is the enrollment ID.
    //    Currently sample is using the default enrollment, so we don't specify anything here.

    PWSTR output = nullptr;
    HRESULT hr = E_FAIL;
    hr = RegisterDeviceWithLocalManagement(NULL);
    if (FAILED(hr))
    {
        TRACEP(L"Error: RegisterDeviceWithLocalManagement failed. Error code = ", hr);
        throw DMException("MdmProvisionSyncBodyWithAttributes");
    }

    PWSTR syncMLResult = NULL;
    hr = ApplyLocalManagementSyncML(requestSyncML.c_str(), &output);
    if (FAILED(hr))
    {
        TRACEP(L"Error: RegisterDeviceWithLocalManagement failed. Error code = ", hr);
        throw DMException("MdmProvisionSyncBodyWithAttributes");
    }

    if (output)
    {
        outputSyncML = output;
    }
    LocalFree(output);

    TRACEP(L"Response: ", outputSyncML.c_str());

    // The results have two top elements: Status and Results.
    // Xml parser does not allow two top-level roots, so we have to wrap it in a root element first.
    wstring returnCodeString;
    wstring wrappedResult = outputSyncML;

    TRACEP(L"Response2: ", wrappedResult.c_str());

    Utils::ReadXmlValue(wrappedResult, STATUS_XML_PATH, returnCodeString);

    unsigned int returnCode = stoi(returnCodeString);
    if (returnCode >= 300)
    {
        ReportError(requestSyncML, wrappedResult, returnCode);
        throw DMExceptionWithErrorCode(returnCode);
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

    // Extract the result data
    wstring wrappedResult = ROOT_START_TAG + resultSyncML + ROOT_END_TAG;

    wstring value;
    Utils::ReadXmlValue(wrappedResult, RESULTS_XML_PATH, value);
    return value;
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
                <Meta><Format xmlns="syncml:metinf">int</Format></Meta>
              </Item>
            </Get>
        </SyncBody>
        )";

    wstring resultSyncML;
    RunSyncML(sid, requestSyncML, resultSyncML);

    // Extract the result data
    wstring valueString;
    wstring wrappedResult = ROOT_START_TAG + resultSyncML + ROOT_END_TAG;
    Utils::ReadXmlValue(wrappedResult, RESULTS_XML_PATH, valueString);
    return stoi(valueString);
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

void MdmProvision::RunSet(const wstring& sid, const wstring& path, unsigned int value)
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

void MdmProvision::RunAdd(const wstring& path, const wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    RunAdd(L"", path, value);
}

wstring MdmProvision::RunGetString(const wstring& path)
{
    // empty sid is okay for device-wide CSPs.
    return RunGetString(L"", path);
}

unsigned int MdmProvision::RunGetUInt(const wstring& path)
{
    // empty sid is okay for device-wide CSPs.
    return RunGetUInt(L"", path);
}

void MdmProvision::RunSet(const wstring& path, const wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    RunSet(L"", path, value);
}

void MdmProvision::RunSet(const wstring& path, unsigned int value)
{
    // empty sid is okay for device-wide CSPs.
    RunSet(L"", path, value);
}

void MdmProvision::RunExec(const wstring& path)
{
    // empty sid is okay for device-wide CSPs.
    RunExec(L"", path);
}

void MdmProvision::ReportError(const wstring& syncMLRequest, const wstring& syncMLResponse, int errorCode)
{
    if (s_errorVerbosity)
    {
        TRACE(L"Error:\n\n");
        TRACEP(L"Request:\n", syncMLRequest.c_str());
        TRACEP(L"Response:\n", syncMLResponse.c_str());
        TRACEP(L"Error:\n", errorCode);
        TRACEP(L"Error Message:\n", Utils::GetResourceString(errorCode).c_str());
    }
    else
    {
        TRACEP(L"Error:\n", Utils::GetResourceString(errorCode).c_str());
    }
}

void MdmProvision::ReportError(const wstring& syncMLRequest, const wstring& syncMLResponse)
{
    TRACE(L"Error:\n\n");
    TRACEP(L"Request:\n", syncMLRequest.c_str());
    TRACEP(L"Response:\n", syncMLResponse.c_str());
}
