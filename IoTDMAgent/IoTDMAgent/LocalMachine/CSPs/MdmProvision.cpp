#include "stdafx.h"
#include <vector>
#include "..\..\Utilities\Logger.h"
#include "..\..\Utilities\Utils.h"
#include "..\..\Utilities\XmlParser.h"
#include "PrivateAPIs\CSPController.h"
#include "MdmProvision.h"

#define ROOT_XML L"Root"
#define ROOT_START_TAG L"<" ROOT_XML L">"
#define ROOT_END_TAG L"</" ROOT_XML L">"
#define STATUS_XML_PATH ROOT_XML L"\\Status\\Data\\"
#define RESULTS_XML_PATH ROOT_XML L"\\Results\\Item\\Data\\"

using namespace std;

map<wstring, wstring> MdmProvision::s_errorCodeToMessage;

bool MdmProvision::s_errorVerbosity = false;

void MdmProvision::SetErrorVerbosity(bool verbosity)
{
    s_errorVerbosity = verbosity;
}

bool MdmProvision::RunSyncML(const wstring& sid, const wstring& inputSyncML, wstring& outputSyncML)
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

bool MdmProvision::RunAdd(const wstring& sid, const wstring& path, wstring& value)
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

    if (!IsSuccessful(inputSyncML, resultSyncML))
    {
        return false;
    }

    return true;
}

bool MdmProvision::RunGet(const wstring& sid, const wstring& path, wstring& value)
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

    if (!IsSuccessful(inputSyncML, resultSyncML))
    {
        return false;
    }

    // Extract the result data
    wstring wrappedResult = ROOT_START_TAG + resultSyncML + ROOT_END_TAG;
    if (!XmlParser::ReadXmlValue(wrappedResult, RESULTS_XML_PATH, value))
    {
        TRACEP(L"Error: Failed to read: ", RESULTS_XML_PATH);
        ReportError(inputSyncML, wrappedResult);
        return false;
    }

    return true;
}

bool MdmProvision::RunGet(const wstring& sid, const wstring& path, unsigned int& value)
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

    if (!IsSuccessful(inputSyncML, resultSyncML))
    {
        return false;
    }

    // Extract the result data
    wstring valueString;
    wstring wrappedResult = ROOT_START_TAG + resultSyncML + ROOT_END_TAG;
    if (!XmlParser::ReadXmlValue(wrappedResult, RESULTS_XML_PATH, valueString))
    {
        TRACEP(L"Error: Failed to read: ", RESULTS_XML_PATH);
        ReportError(inputSyncML, wrappedResult);
        return false;
    }

    return Utils::StringToInt(valueString, value);
}

bool MdmProvision::RunSet(const wstring& sid, const wstring& path, const wstring& value)
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

    if (!IsSuccessful(inputSyncML, resultSyncML))
    {
        return false;
    }
    return true;
}

bool MdmProvision::RunSet(const wstring& sid, const wstring& path, unsigned int value)
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

    if (!IsSuccessful(inputSyncML, resultSyncML))
    {
        return false;
    }
    return true;
}

bool MdmProvision::RunAdd(const std::wstring& path, std::wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    return RunAdd(L"", path, value);
}

bool MdmProvision::RunGet(const std::wstring& path, std::wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    return RunGet(L"", path, value);
}

bool MdmProvision::RunGet(const std::wstring& path, unsigned int& value)
{
    // empty sid is okay for device-wide CSPs.
    return RunGet(L"", path, value);
}

bool MdmProvision::RunSet(const std::wstring& path, const std::wstring& value)
{
    // empty sid is okay for device-wide CSPs.
    return RunSet(L"", path, value);
}

bool MdmProvision::RunSet(const std::wstring& path, unsigned int value)
{
    // empty sid is okay for device-wide CSPs.
    return RunSet(L"", path, value);
}

bool MdmProvision::IsSuccessful(const std::wstring& requestSyncML, const std::wstring& resultSyncML)
{
    // The results have two top elements: Status and Results.
    // Xml parser does not allow two top-level roots, so we have to wrap it in a root element first.
    wstring wrappedResult = ROOT_START_TAG + resultSyncML + ROOT_END_TAG;

    wstring returnCodeString;
    if (!XmlParser::ReadXmlValue(wrappedResult, STATUS_XML_PATH, returnCodeString))
    {
        TRACEP(L"Error: Failed to read: ", STATUS_XML_PATH);
        ReportError(requestSyncML, resultSyncML, returnCodeString);
        return false;
    }

    unsigned int returnCode = 0;
    if (!Utils::StringToInt(returnCodeString, returnCode))
    {
        return false;
    }
    if (200 <= returnCode && returnCode < 300)
    {
        return true;
    }
    ReportError(requestSyncML, resultSyncML, returnCodeString);
    return false;
}

void MdmProvision::InitializeErrorMessages()
{
    // http://technical.openmobilealliance.org/Technical/release_program/docs/Common/V1_2_2-20090724-A/OMA-TS-SyncML-RepPro-V1_2_2-20090724-A.pdf
    // 10. Response Status Codes 
    s_errorCodeToMessage[L"101"] = L"(Informational) In progress. The specified SyncML command is being carried out, but has not yet completed.";

    s_errorCodeToMessage[L"200"] = L"(Successful)OK.  The SyncML command completed successfully.";
    s_errorCodeToMessage[L"201"] = L"(Successful)Item added.  The requested item was added.";
    s_errorCodeToMessage[L"202"] = L"(Successful)Accepted for processing.  The request to either run a remote execution of an application or to alert a user or application was successfully performed.";
    s_errorCodeToMessage[L"203"] = L"(Successful)Non - authoritative response. The request is being responded to by an entity other than the one targeted. The response is only to be returned when the request would have been resulted in a 200 response code from the authoritative target.";
    s_errorCodeToMessage[L"204"] = L"(Successful)No content. The request was successfully completed but no data is being returned. The response code is also returned in response to a Get when the target has no content.";
    s_errorCodeToMessage[L"205"] = L"(Successful)Reset content. The source SHOULD update their content. The originator of the request is being told that their content SHOULD be synchronized to get an up to date version.";
    s_errorCodeToMessage[L"206"] = L"(Successful)Partial content. The response indicates that only part of the command was completed. If the remainder of the command can be completed later, then when completed another appropriate completion request status code SHOULD be created.";
    s_errorCodeToMessage[L"207"] = L"(Successful)Conflict resolved with merge. The response indicates that the request created a conflict; which was resolved with a merge of the client and server instances of the data. The response includes both the Target and Source URLs in the Item of the Status. In addition, a Replace command is returned with the merged data.";
    s_errorCodeToMessage[L"208"] = L"(Successful)Conflict resolved with client’s command \"winning\". The response indicates that there was an update conflict; which was resolved by the client command winning.";
    s_errorCodeToMessage[L"209"] = L"(Successful)Conflict resolved with duplicate. The response indicates that the request created an update conflict; which was resolved with a duplication of the client’s data being created in the server database. The response includes both the target URI of the duplicate in the Item of the Status. In addition, in the case of a two - way synchronization, an Add command is returned with the duplicate data definition.";
    s_errorCodeToMessage[L"210"] = L"(Successful)Delete without archive. The response indicates that the requested data was successfully deleted, but that it was not archived prior to deletion because this OPTIONAL feature was not supported by the implementation.";
    s_errorCodeToMessage[L"211"] = L"(Successful)Item not deleted. The requested item was not found. It could have been previously deleted.";
    s_errorCodeToMessage[L"212"] = L"(Successful)Authentication accepted. No further authentication is needed for the remainder of the synchronization session. This response code can only be used in response to a request in which the credentials were provided.";
    s_errorCodeToMessage[L"213"] = L"(Successful)Chunked item accepted and buffered.";
    s_errorCodeToMessage[L"214"] = L"(Successful)Operation cancelled. The SyncML command completed successfully, but no more commands will be processed within the session.";
    s_errorCodeToMessage[L"215"] = L"(Successful)Not executed. A command was not executed, as a result of user interaction and user chose not to accept the choice.";
    s_errorCodeToMessage[L"216"] = L"(Successful)Atomic roll back OK. A command was inside Atomic element and Atomic failed. This command was rolled back successfully.";

    s_errorCodeToMessage[L"300"] = L"(Redirection)Multiple choices. The requested target is one of a number of multiple alternatives requested target. The alternative SHOULD also be returned in the Item element type in the Status.";
    s_errorCodeToMessage[L"301"] = L"(Redirection)Moved permanently. The requested target has a new URI. The new URI SHOULD also be returned in the Item element type in the Status.";
    s_errorCodeToMessage[L"302"] = L"(Redirection)Found. The requested target has temporarily moved to a different URI. The original URI SHOULD continue to be used. The URI of the temporary location SHOULD also be returned in the Item element type in the Status. The requestor SHOULD confirm the identity and authority of the temporary URI to act on behalf of the original target URI.";
    s_errorCodeToMessage[L"303"] = L"(Redirection)See other. The requested target can be found at another URI. The other URI SHOULD be returned in the Item element type in the Status.";
    s_errorCodeToMessage[L"304"] = L"(Redirection)Not modified. The requested SyncML command was not executed on the target. This is an additional response that can be added to any of the other Redirection response codes.";
    s_errorCodeToMessage[L"305"] = L"(Redirection)Use proxy. The requested target MUST be accessed through the specified proxy URI. The proxy URI SHOULD also be returned in the Item element type in the Status.";

    s_errorCodeToMessage[L"400"] = L"(Originator Exception) Bad request. The requested command could not be performed because of malformed syntax in the command. The malformed command MAY also be returned in the Item element type in the Status.";
    s_errorCodeToMessage[L"401"] = L"(Originator Exception) Invalid credentials. The requested command failed because the requestor MUST provide proper authentication. If the property type of authentication was presented in the original request, then the response code indicates that the requested command has been refused for those credentials.";
    s_errorCodeToMessage[L"402"] = L"(Originator Exception) Paymentneeded. The requested command failed because proper payment isneeded. This version of SyncML does not standardizethe payment mechanism.";
    s_errorCodeToMessage[L"403"] = L"(Originator Exception) Forbidden. The requested command failed, but the recipient understood the requested command. Authentication will not help and the request SHOULD NOT be repeated. If the recipient wishes to make public why the request was denied, then a description MAY be specified in the Item element type in the Status. If the recipient does not wish to make public why the request was denied then the response code 404 MAY be used instead.";
    s_errorCodeToMessage[L"404"] = L"(Originator Exception) Not found. The requested target was not found. No indication is given as to whether this is a temporary or permanent condition. The response code 410 SHOULD be used when the condition is permanent and the recipient wishes to make this fact public. This response code is also used when the recipient does not want to make public the reason for why a requested command is not allowed or when no other response code is appropriate.";
    s_errorCodeToMessage[L"405"] = L"(Originator Exception) Command not allowed. The requested command is not allowed on the target. The recipient SHOULD return the allowed command for the target in the Item element type in the Status.";
    s_errorCodeToMessage[L"406"] = L"(Originator Exception) Optional feature not supported. The requested command failed because an OPTIONAL feature in the request was not supported. The unsupported feature SHOULD be specified by the Item element type in the Status.";
    s_errorCodeToMessage[L"407"] = L"(Originator Exception) Missing credentials. This response code is similar to 401 except that the response code indicates that the originator MUST first authenticate with the recipient. The recipient SHOULD also return the suitable challenge in the Chal element type in the Status.";
    s_errorCodeToMessage[L"408"] = L"(Originator Exception) Request timeout. An expected message was not received within the REQUIRED period of time. The request can be repeated at another time. The RespURI can be used to specify the URI and optionally the date / time after which the originator can repeat the request. See RespURI for details.";
    s_errorCodeToMessage[L"409"] = L"(Originator Exception) Conflict. The requested failed because of an update conflict between the client and server versions of the data. Setting of the conflict resolution policy is outside the scope of this version of SyncML. However, identification of conflict resolution performed, if any, is within the scope.";
    s_errorCodeToMessage[L"410"] = L"(Originator Exception) Gone. The requested target is no longer on the recipient and no forwarding URI is known.";
    s_errorCodeToMessage[L"411"] = L"(Originator Exception) SizeREQUIRED. The requested command MUST be accompanied by byte size or length information in the Meta element type.";
    s_errorCodeToMessage[L"412"] = L"(Originator Exception) Incomplete command. The requested command failed on the recipient because it was incomplete or incorrectly formed. The recipient SHOULD specify the portion of the command that was incomplete or incorrect in the Item element type in the Status.";
    s_errorCodeToMessage[L"413"] = L"(Originator Exception) Request entity too large. The recipient is refusing to perform the requested command because the requested item is larger than the recipient is able or willing to process. If the condition is temporary, the recipient SHOULD also include a Status with the response code 418 and specify a RespURI with the response URI and optionally the date / time that the command SHOULD be repeated.";
    s_errorCodeToMessage[L"414"] = L"(Originator Exception) URI too long. The requested command failed because the target URI is too long for what the recipient is able or willing to process. This response code is seldom encountered, but is used when a recipient perceives that an intruder might be attempting to exploit security holes or other defects in order to threaten the recipient.";
    s_errorCodeToMessage[L"415"] = L"(Originator Exception) Unsupported media type or format. The unsupported content type or format SHOULD also be identified in the Item element type in the Status.";
    s_errorCodeToMessage[L"416"] = L"(Originator Exception) Requested size too big. The request failed because the specified byte size in the request was too big.";
    s_errorCodeToMessage[L"417"] = L"(Originator Exception) Retry later. The request failed at this time and the originator SHOULD retry the request later. The recipient SHOULD specify a RespURI with the response URI and the date / time that the command SHOULD be repeated.";
    s_errorCodeToMessage[L"418"] = L"(Originator Exception) Already exists. The requested Put or Add command failed because the target already exists.";
    s_errorCodeToMessage[L"419"] = L"(Originator Exception) Conflict resolved with server data. The response indicates that the client request created a conflict; which was resolved by the server command winning. The normal information in the Status SHOULD be sufficient for the client to \"undo\" the resolution, if it is desired.";
    s_errorCodeToMessage[L"420"] = L"(Originator Exception) Device full. The response indicates that the recipient has no more storage space for the remaining synchronization data. The response includes the remaining number of data that could not be returned to the originator in the Item of the Status.";
    s_errorCodeToMessage[L"421"] = L"(Originator Exception) Unknown search grammar. The requested command failed on the server because the specified search grammar was not known. The client SHOULD re - specify the search using a known search grammar.";
    s_errorCodeToMessage[L"422"] = L"(Originator Exception) Bad CGI Script. The requested command failed on the server because the CGI scripting in the LocURI was incorrectly formed. The client SHOULD re - specify the portion of the command that was incorrect in the Item element type in the Status.";
    s_errorCodeToMessage[L"423"] = L"(Originator Exception) Soft - delete conflict. The requested command failed because the \"Soft Deleted\" item was previously \"Hard Deleted\" on the server.";
    s_errorCodeToMessage[L"424"] = L"(Originator Exception) Size mismatch. The chunked object was received, but the size of the received object did not match the size declared within the first chunk.";
    s_errorCodeToMessage[L"425"] = L"(Originator Exception) Permission Denied. The requested command failed because the sender does not have adequate access control permissions(ACL) on the recipient.";
    s_errorCodeToMessage[L"426"] = L"(Originator Exception) Partial item not accepted. Receiver of status code MAY resend the whole item in next package.";
    s_errorCodeToMessage[L"427"] = L"(Originator Exception) Item Not empty. Parent cannot be deleted since it contains children.";
    s_errorCodeToMessage[L"428"] = L"(Originator Exception) Move Failed";

    s_errorCodeToMessage[L"500"] = L"(Recipient Exception) Command failed. The recipient encountered an unexpected condition which prevented it from fulfilling the request";
    s_errorCodeToMessage[L"501"] = L"(Recipient Exception) Command not implemented. The recipient does not support the command REQUIRED to fulfill the request. This is the appropriate response when the recipient does not recognize the requested command and is not capable of supporting it for any resource.";
    s_errorCodeToMessage[L"502"] = L"(Recipient Exception) Bad gateway. The recipient, while acting as a gateway or proxy, received an invalid response from the upstream recipient it accessed in attempting to fulfill the request.";
    s_errorCodeToMessage[L"503"] = L"(Recipient Exception) Service unavailable. The recipient is currently unable to handle the request due to a temporary overloading or maintenance of the recipient. The implication is that this is a temporary condition; which will be alleviated after some delay. The recipient SHOULD specify the URI and date / time after which the originator SHOULD retry in the RespURI in the response.";
    s_errorCodeToMessage[L"504"] = L"(Recipient Exception) Gateway timeout. The recipient, while acting as a gateway or proxy, did not receive a timely response from the upstream recipient specified by the URI(e. g. HTTP, FTP, LDAP) or some other auxiliary recipient(e. g. DNS) it needed to access in attempting to complete the request.";
    s_errorCodeToMessage[L"505"] = L"(Recipient Exception) DTD Version not supported. The recipient does not support or refuses to support the specified version of SyncML DTD used in the request SyncML Message. The recipient MUST include the versions it does support in the Item element type in the Status.";
    s_errorCodeToMessage[L"506"] = L"(Recipient Exception) Processing error. An application error occurred while processing the request. The originator SHOULD retry the request. The RespURI can contain the URI and date / time after which the originator can retry the request.";
    s_errorCodeToMessage[L"507"] = L"(Recipient Exception) Atomic failed. The error caused all SyncML commands within an Atomic element type to fail.";
    s_errorCodeToMessage[L"508"] = L"(Recipient Exception) RefreshREQUIRED. An error occurred that necessitates a refresh of the current synchronization state of the client with the server. Client is requested to initiate the session type specified in the server’s ALERT(which is included in the same package as the Status 508). The only valid values for this ALERT are either a slow sync(201) or a refresh with the server.";
    s_errorCodeToMessage[L"509"] = L"(Recipient Exception) Reserved for future use.";
    s_errorCodeToMessage[L"510"] = L"(Recipient Exception) Data store failure. An error occurred while processing the request. The error is related to a failure in the recipient data store.";
    s_errorCodeToMessage[L"511"] = L"(Recipient Exception) Server failure. A severe error occurred in the server while processing the request. The originator SHOULD NOT retry the request.";
    s_errorCodeToMessage[L"512"] = L"(Recipient Exception) Synchronization failed. An application error occurred during the synchronization session. The originator SHOULD restart the synchronization session from the beginning.";
    s_errorCodeToMessage[L"513"] = L"(Recipient Exception) Protocol Version not supported. The recipient does not support or refuses to support the specified version of the SyncML Synchronization Protocol used in the request SyncML Message. The recipient MUST include the versions it does support in the Item element type in the Status.";
    s_errorCodeToMessage[L"514"] = L"(Recipient Exception) Operation cancelled. The SyncML command was not completed successfully, since the operation was already cancelled before processing the command. The originator SHOULD repeat the command in the next session.";
    s_errorCodeToMessage[L"516"] = L"(Recipient Exception) Atomic roll back failed. Command was inside Atomic element and Atomic failed. This command was not rolled back successfully. Server SHOULD take action to try to recover client back into original state.";
    s_errorCodeToMessage[L"517"] = L"(Recipient Exception) Atomic response too large to fit. The response to an atomic command was too large to fit in a single message.";
}

wstring MdmProvision::GetErrorMessage(const wstring& errorCodeString)
{
    static bool initialized = false;

    // ToDo: not thread safe.
    if (!initialized)
    {
        InitializeErrorMessages();
        initialized = true;
    }

    wstring errorMessage = errorCodeString + L"Unknown: check http://technical.openmobilealliance.org/Technical/release_program/docs/Common/V1_2_2-20090724-A/OMA-TS-SyncML-RepPro-V1_2_2-20090724-A.pdf";
    auto it = s_errorCodeToMessage.find(errorCodeString);
    if (it != s_errorCodeToMessage.end())
    {
        errorMessage = it->second;
    }
    return errorMessage;
}

void MdmProvision::ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse, const std::wstring& errorCodeString)
{
    if (s_errorVerbosity)
    {
        TRACE(L"Error:\n\n");
        TRACEP(L"Request:\n", syncMLRequest.c_str());
        TRACEP(L"Response:\n", syncMLResponse.c_str());
        TRACEP(L"Error:\n", errorCodeString.c_str());
        TRACEP(L"Error Message:\n", GetErrorMessage(errorCodeString).c_str());
    }
    else
    {
        TRACEP(L"Error:\n", GetErrorMessage(errorCodeString).c_str());
    }
}

void MdmProvision::ReportError(const std::wstring& syncMLRequest, const std::wstring& syncMLResponse)
{
    TRACE(L"Error:\n\n");
    TRACEP(L"Request:\n", syncMLRequest.c_str());
    TRACEP(L"Response:\n", syncMLResponse.c_str());
}
