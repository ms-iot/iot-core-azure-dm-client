#include "stdafx.h"
#include <vector>
#include "CSPClient.h"
#include "..\..\Utilities\Logger.h"
#include "..\..\Utilities\Utils.h"
#include "..\..\Utilities\XmlParser.h"
#include "PrivateAPIs\CSPController.h"
#include "SyncMLHelper.h"

#define ROOT_XML L"Root"
#define ROOT_START_TAG L"<" ROOT_XML L">"
#define ROOT_END_TAG L"</" ROOT_XML L">"
#define STATUS_XML_PATH ROOT_XML L"\\Status\\Data\\"
#define RESULTS_XML_PATH ROOT_XML L"\\Results\\Item\\Data\\"

using namespace std;

bool CSPClient::RunSyncML(const wstring& sid, const wstring& inputSyncML, wstring& outputSyncML)
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

        hr = MdmProvisionSyncBodyWithAttributes(inputSyncML.c_str(), nullptr, ARRAYSIZE(attrib), attrib, &output);
    }
    else
    {
        hr = MdmProvisionSyncBodyWithAttributes(inputSyncML.c_str(), nullptr, 0, nullptr, &output);
    }
    if (FAILED(hr))
    {
        return false;
    }

    if (output)
    {
        outputSyncML = output;
    }
    LocalFree(output);

    return true;
}

bool CSPClient::RunAdd(const wstring& sid, const wstring& path, wstring& value)
{
    wstring inputSyncML = LR"(
        <SyncBody>
            <Add>
                <CmdID>1</CmdID>
                <Item>
                    <Target>
                        <LocURI>)";
    inputSyncML += path;
    inputSyncML += value;
    inputSyncML += LR"(</LocURI>
                    </Target>
                </Item>
            </Add>
        </SyncBody>
        )";

    wstring resultSyncML;
    if (!RunSyncML(sid, inputSyncML, resultSyncML))
    {
        return false;
    }

    if (!ParseAddResults(inputSyncML, resultSyncML))
    {
        return false;
    }

    return true;
}

bool CSPClient::RunGet(const wstring& sid, const wstring& path, wstring& value)
{
    wstring inputSyncML = LR"(
        <SyncBody>
            <Get>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    inputSyncML += path;
    inputSyncML += LR"(</LocURI>
                </Target>
                <Meta>
                    <Type xmlns="syncml:metinf">text/plain</Type>
                </Meta>
              </Item>
            </Get>
        </SyncBody>
        )";

    wstring resultSyncML;
    if (!RunSyncML(sid, inputSyncML, resultSyncML))
    {
        return false;
    }

    if (!ParseGetResults(inputSyncML, resultSyncML, value))
    {
        return false;
    }
    return true;
}

bool CSPClient::RunGet(const wstring& sid, const wstring& path, unsigned int& value)
{
    wstring inputSyncML = LR"(
        <SyncBody>
            <Get>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    inputSyncML += path.c_str();
    inputSyncML += LR"(</LocURI>
                </Target>
                <Meta><Format xmlns="syncml:metinf">int</Format></Meta>
              </Item>
            </Get>
        </SyncBody>
        )";

    wstring resultSyncML;
    if (!RunSyncML(sid, inputSyncML, resultSyncML))
    {
        return false;
    }

    wstring valueString;
    if (!ParseGetResults(inputSyncML, resultSyncML, valueString))
    {
        return false;
    }
    return Utils::StringToInt(valueString, value);
}

bool CSPClient::RunSet(const wstring& sid, const wstring& path, const wstring& value)
{
    wstring inputSyncML = LR"(
        <SyncBody>
            <Replace>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    inputSyncML += path;
    inputSyncML += LR"(</LocURI>
                </Target>
                <Meta>
                    <Type xmlns="syncml:metinf">text/plain</Type>
                </Meta>
                <Data>)";
    inputSyncML += value;
    inputSyncML += LR"(</Data>
              </Item>
            </Replace>
        </SyncBody>
        )";

    wstring resultSyncML;
    if (!RunSyncML(sid, inputSyncML, resultSyncML))
    {
        return false;
    }

    if (!ParseSetResults(inputSyncML, resultSyncML))
    {
        return false;
    }
    return true;
}

bool CSPClient::RunSet(const wstring& sid, const wstring& path, unsigned int value)
{
    wstring inputSyncML = LR"(
        <SyncBody>
            <Replace>
              <CmdID>1</CmdID>
              <Item>
                <Target>
                  <LocURI>)";
    inputSyncML += path;
    inputSyncML += LR"(</LocURI>
                </Target>
                <Meta><Format xmlns="syncml:metinf">int</Format></Meta>
                <Data>)";
    inputSyncML += std::to_wstring(value);
    inputSyncML += LR"(</Data>
              </Item>
            </Replace>
        </SyncBody>
        )";

    wstring resultSyncML;
    if (!RunSyncML(sid, inputSyncML, resultSyncML))
    {
        return false;
    }

    if (!ParseSetResults(inputSyncML, resultSyncML))
    {
        return false;
    }
    return true;
}

bool CSPClient::RunAdd(const std::wstring& path, std::wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    return RunAdd(L"", path, value);
}

bool CSPClient::RunGet(const std::wstring& path, std::wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    return RunGet(L"", path, value);
}

bool CSPClient::RunGet(const std::wstring& path, unsigned int& value)
{
    // empty sid is okay for device-wide CSPs.
    return RunGet(L"", path, value);
}

bool CSPClient::RunSet(const std::wstring& path, const std::wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    return RunSet(L"", path, value);
}

bool CSPClient::RunSet(const std::wstring& path, unsigned int value)
{
    // empty sid is okay for device-wide CSPs.
    return RunSet(L"", path, value);
}

bool CSPClient::ParseGetResults(const wstring& requestSyncML, const wstring& resultSyncML, wstring& results)
{
    // The results have two top elements: Status and Results.
    // Xml parser does not allow two top-level roots, so we have to wrap it in a root element first.
    wstring wrappedResult = ROOT_START_TAG + resultSyncML + ROOT_END_TAG;

    wstring returnCode;
    if (!XmlParser::ReadXmlValue(wrappedResult, STATUS_XML_PATH, returnCode))
    {
        TRACEP(L"Error: Failed to read: ",  STATUS_XML_PATH);
        SyncML::ReportError(requestSyncML, resultSyncML, returnCode);
        return false;
    }

    if (!SyncML::IsSuccess(returnCode))
    {
        SyncML::ReportError(requestSyncML, resultSyncML, returnCode);
        return false;
    }

    if (!XmlParser::ReadXmlValue(wrappedResult, RESULTS_XML_PATH, results))
    {
        TRACEP(L"Error: Failed to read: ",  RESULTS_XML_PATH);
        SyncML::ReportError(requestSyncML, resultSyncML, returnCode);
        return false;
    }

    return true;
}

bool CSPClient::ParseSetResults(const wstring& requestSyncML, const wstring& resultSyncML)
{
    // The results have two top elements: Status and Results (see above).
    // Xml parser does not like two top-level roots, so we have to wrap it in a root element first.
    wstring wrappedResult = ROOT_START_TAG + resultSyncML + ROOT_END_TAG;

    wstring returnCode;
    if (!XmlParser::ReadXmlValue(wrappedResult, STATUS_XML_PATH, returnCode))
    {
        TRACEP(L"Error: Failed to read: ",  STATUS_XML_PATH);
        SyncML::ReportError(requestSyncML, resultSyncML, returnCode);
        return false;
    }

    if (!SyncML::IsSuccess(returnCode))
    {
        SyncML::ReportError(requestSyncML, resultSyncML, returnCode);
        return false;
    }

    return true;
}

bool CSPClient::ParseAddResults(const wstring& requestSyncML, const wstring& resultSyncML)
{
    // The results have two top elements: Status and Results (see above).
    // Xml parser does not like two top-level roots, so we have to wrap it in a root element first.
    wstring wrappedResult = ROOT_START_TAG + resultSyncML + ROOT_END_TAG;

    wstring returnCode;
    if (!XmlParser::ReadXmlValue(wrappedResult, STATUS_XML_PATH, returnCode))
    {
        TRACEP(L"Error: Failed to read: ",  STATUS_XML_PATH);
        SyncML::ReportError(requestSyncML, resultSyncML, returnCode);
        return false;
    }

    if (!SyncML::IsSuccess(returnCode))
    {
        SyncML::ReportError(requestSyncML, resultSyncML, returnCode);
        return false;
    }

    return true;
}
