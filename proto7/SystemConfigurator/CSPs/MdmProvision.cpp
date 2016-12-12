#include "stdafx.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"
#include "..\SharedUtilities\Utils.h"
#include "PrivateAPIs\CSPController.h"
#include "..\resource.h"
#include "MdmProvision.h"

#define ROOT_XML L"Root"
#define ROOT_START_TAG L"<" ROOT_XML L">"
#define ROOT_END_TAG L"</" ROOT_XML L">"
#define STATUS_XML_PATH ROOT_XML L"\\Status\\Data\\"
#define RESULTS_XML_PATH ROOT_XML L"\\Results\\Item\\Data\\"

using namespace std;

bool MdmProvision::s_errorVerbosity = false;

wstring GetResourceString(int id)
{
    const int bufferSize = 1024;
    wchar_t buffer[bufferSize];
    if (!LoadString(GetModuleHandle(NULL), id, buffer, bufferSize))
    {
        if (!LoadString(GetModuleHandle(NULL), IDS_OMA_SYNCML_STATUS_UNKNOWN, buffer, bufferSize))
        {
            return wstring(L"Unknown OMA SyncML status code.");
        }
    }
    return wstring(buffer);
}

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
    if (sid.length())
    {
        SYNCMLATTRIBUTE attrib[1] = { 0 };
        attrib[0].pszName = OMADM_TARGETEDUSERSID_VARIABLE_NAME;
        attrib[0].pszValue = sid.c_str();

        hr = MdmProvisionSyncBodyWithAttributes(requestSyncML.c_str(), nullptr, ARRAYSIZE(attrib), attrib, &output);
    }
    else
    {
        hr = MdmProvisionSyncBodyWithAttributes(requestSyncML.c_str(), nullptr, 0, nullptr, &output);
    }
    if (FAILED(hr))
    {
        TRACEP(L"Error: MdmProvisionSyncBodyWithAttributes failed. Error code = ", hr);
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
    wstring wrappedResult = ROOT_START_TAG + outputSyncML + ROOT_END_TAG;
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

map<wstring, map<wstring, wstring>> MdmProvision::RunGetStructData(const std::wstring& path)
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
    wstring wrappedResult = ROOT_START_TAG + resultSyncML + ROOT_END_TAG;

    map<wstring, map<wstring, wstring>> dataMap;
    Utils::ReadXmlStructData(wrappedResult, dataMap);
    return dataMap;
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
        TRACEP(L"Error Message:\n", GetResourceString(errorCode).c_str());
    }
    else
    {
        TRACEP(L"Error:\n", GetResourceString(errorCode).c_str());
    }
}

void MdmProvision::ReportError(const wstring& syncMLRequest, const wstring& syncMLResponse)
{
    TRACE(L"Error:\n\n");
    TRACEP(L"Request:\n", syncMLRequest.c_str());
    TRACEP(L"Response:\n", syncMLResponse.c_str());
}
