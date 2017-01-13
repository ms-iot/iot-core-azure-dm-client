//
// Temporary code - will be eventually replaced.
//
#include "stdafx.h"
#include <wrl/client.h>
#include "CSPController.h"

using namespace Microsoft::WRL;

// Input XML is just SyncBody (skip top-level SyncML and sibling SyncHdr elements).
// <SyncBody>
//    <Replace>
//      <CmdID>100</CmdID>
//      <Item>
//        <Target>
//          <LocURI>./Device/Vendor/MSFT/EnterpriseDataProtection/Settings/EDPEnforcementLevel</LocURI>
//        </Target>
//        <Meta>
//          <Format xmlns="syncml:metinf">int</Format>
//        </Meta>
//        <Data>2</Data>
//      </Item>
//    </Replace>
//    <Final/>
//  </SyncBody>

WCHAR const c_szUtf16Encoding[] = L"UTF-16";

// *******************************************************
// Copied from syncmldpu.h
// *******************************************************
static void FreeResultsData(_In_opt_ PSYNCMLDPURESULTS psmlResults)
{
    if (nullptr == psmlResults)
    {
        return;
    }

    LocalFree(psmlResults->pszResultsXML);
    psmlResults->pszResultsXML = nullptr;

    LocalFree(psmlResults->pszNextNonce);
    psmlResults->pszNextNonce = nullptr;

    for (DWORD i = 0; i < psmlResults->cairAlertStatuses; i++)
    {
        LocalFree(psmlResults->rgairAlertStatuses[i].pszCmdIDRef);
        psmlResults->rgairAlertStatuses[i].pszCmdIDRef = nullptr;
    }

    LocalFree(psmlResults->rgairAlertStatuses);
    psmlResults->rgairAlertStatuses = nullptr;
}

HRESULT MdmProvisionSyncBodyWithAttributesInternal(
    _In_                   PCWSTR pszInContent,
    _In_opt_               PCWSTR pszServerID,
                           DWORD  cVariables,
    _In_reads_(cVariables) SYNCMLATTRIBUTE* rgVariables,
    _Outptr_               PWSTR* ppszOutContent
)
{
    HRESULT hr = S_OK;
    const DWORD c_dwOmaDmStatusSuccess = 200;
    const DWORD c_dwSecRole = 0;
    const DWORD c_dwServerMessageId = 1;
    const DWORD c_dwClientMessageId = 2;
    const WCHAR c_szServerId[] = L"IoTConfigSource";
    const WCHAR c_szDisplayedSrc[] = L"Sample IoT API";
    ComPtr<IProvisioningDPU>  pDPU;
    SYNCMLDPUINIT dpui = { 0 };
    SYNCMLDPUPARAMS sdpuParams = { 0 };
    SYNCMLDPURESULTS smlResults = { 0 };

    // Create instance of SyncML DPU
    hr = CoCreateInstance(__uuidof(CSyncMLDPU), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(pDPU.GetAddressOf()));
    if (FAILED(hr))
    {
        return hr;
    }

    // Initialize DPU.
    dpui.fVerboseLogging = TRUE;
    dpui.dwSessionRoles = c_dwSecRole;
    dpui.pszDisplayedSrc = c_szDisplayedSrc;

    if (NULL == pszServerID)
    {
        dpui.pszServerID = c_szServerId;
    }
    else
    {
        dpui.pszServerID = pszServerID;
    }

    dpui.pCustomAttributes = rgVariables;
    dpui.cCustomAttributes = cVariables;

    hr = pDPU->Initialize(reinterpret_cast<LPBYTE>(&dpui), sizeof(dpui));
    if (FAILED(hr))
    {
        return hr;
    }

    // Get size of input data plus one NULL terminator
    size_t cchInContent = 0;
    hr = StringCchLength(pszInContent, STRSAFE_MAX_CCH - 1, &cchInContent);
    if (FAILED(hr))
    {
        return hr;
    }
    ++cchInContent;

    // This is where we actually process in the incoming XML.
    sdpuParams.dwMsgID = c_dwServerMessageId;
    sdpuParams.pszBody = pszInContent;
    sdpuParams.dwClientMsgID = c_dwClientMessageId;
    sdpuParams.fParseOnlySyncHdrStatus = FALSE;
    sdpuParams.hrParseOnly = S_OK;

    hr = pDPU->ProcessData(reinterpret_cast<LPBYTE>(&sdpuParams), sizeof(SYNCMLDPUPARAMS), nullptr, nullptr);
    if (FAILED(hr))
    {
        return hr;
    }

    // Get results of processing.
    hr = pDPU->GetResultsData(reinterpret_cast<LPBYTE>(&smlResults), sizeof(smlResults));
    if (FAILED(hr))
    {
        return hr;
    }

    // Hand-off output to caller
    if (nullptr != ppszOutContent)
    {
        *ppszOutContent = smlResults.pszResultsXML;
        smlResults.pszResultsXML = nullptr;
    }

    FreeResultsData(&smlResults);

    return S_OK;
}

HRESULT MdmProvisionSyncBodyWithAttributes(
    _In_                   PCWSTR  pszInContent,
    _In_opt_               PCWSTR  pszServerID,
                           DWORD   cVariables,
    _In_reads_(cVariables) SYNCMLATTRIBUTE* rgVariables,
    _Outptr_               PWSTR* ppszOutContent
)
{
    // ToDo: Parameter check.
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        MdmProvisionSyncBodyWithAttributesInternal(
            pszInContent,
            pszServerID,
            cVariables,
            reinterpret_cast<SYNCMLATTRIBUTE*>(rgVariables),
            ppszOutContent);

        CoUninitialize();
    }

    return S_OK;
}
