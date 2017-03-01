#include "stdafx.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMException.h"
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
    PWSTR output = nullptr;
    HRESULT hr = RegisterDeviceWithLocalManagement(NULL);
    if (FAILED(hr))
    {
        throw DMException("RegisterDeviceWithLocalManagement", hr);
    }

    hr = ApplyLocalManagementSyncML(requestSyncML.c_str(), &output);
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

    TRACEP(L"Request : ", requestSyncML.c_str());
    TRACEP(L"Response: ", outputSyncML.c_str());

    wstring returnCodeString;
    Utils::ReadXmlValue(outputSyncML, STATUS_XML_PATH, returnCodeString);

    unsigned int returnCode = stoi(returnCodeString);
    if (returnCode >= 300)
    {
        ReportError(requestSyncML, outputSyncML, returnCode);
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
    RunSyncML(sid, requestSyncML, resultSyncML);
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
