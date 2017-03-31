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
//
// Temporary code - will be eventually replaced.
//
#pragma once


/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 8.01.0618 */
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __provdpu_h__
#define __provdpu_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */

#ifndef __ISyncMLCmd_FWD_DEFINED__
#define __ISyncMLCmd_FWD_DEFINED__
typedef interface ISyncMLCmd ISyncMLCmd;

#endif 	/* __ISyncMLCmd_FWD_DEFINED__ */


#ifndef __IProvisioningDPU_FWD_DEFINED__
#define __IProvisioningDPU_FWD_DEFINED__
typedef interface IProvisioningDPU IProvisioningDPU;

#endif 	/* __IProvisioningDPU_FWD_DEFINED__ */


#ifndef __CProvisioningDPU_FWD_DEFINED__
#define __CProvisioningDPU_FWD_DEFINED__

#ifdef __cplusplus
typedef class CProvisioningDPU CProvisioningDPU;
#else
typedef struct CProvisioningDPU CProvisioningDPU;
#endif /* __cplusplus */

#endif 	/* __CProvisioningDPU_FWD_DEFINED__ */


#ifndef __CSyncMLDPU_FWD_DEFINED__
#define __CSyncMLDPU_FWD_DEFINED__

#ifdef __cplusplus
typedef class CSyncMLDPU CSyncMLDPU;
#else
typedef struct CSyncMLDPU CSyncMLDPU;
#endif /* __cplusplus */

#endif 	/* __CSyncMLDPU_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
//#include "cfgmgr2.h"
#include "xmllite.h"

#ifdef __cplusplus
extern "C" {
#endif 


    /* interface __MIDL_itf_provdpu_0000_0000 */
    /* [local] */

#ifndef _LPBYTE_DEFINED
#define _LPBYTE_DEFINED
    typedef /* [public] */ BYTE *LPBYTE;

#endif // !_LPBYTE_DEFINED
    typedef
        enum PROV_LOG_LEVEL
    {
        PROV_LOG_LEVEL_NONE = 0,
        PROV_LOG_LEVEL_ERROR = 1,
        PROV_LOG_LEVEL_DEBUG = 2,
        PROV_LOG_LEVEL_TRACE = 3
    } 	PROV_LOG_LEVEL;

    typedef struct tagSyncMLAttribute
    {
        /* [string] */ LPCWSTR pszName;
        /* [string] */ LPCWSTR pszValue;
    } 	SYNCMLATTRIBUTE;

    typedef struct tagSyncMLAttribute *PSYNCMLATTRIBUTE;

    typedef struct tagSyncMLDPUInit
    {
        BOOL fVerboseLogging;
        DWORD dwSessionRoles;
        LPCWSTR pszServerID;
        LPCWSTR pszDisplayedSrc;
        PROV_LOG_LEVEL filterloglevel;
        DWORD cCustomAttributes;
        /* [size_is] */ const SYNCMLATTRIBUTE *pCustomAttributes;
    } 	SYNCMLDPUINIT;

    typedef struct tagSyncMLDPUInit *PSYNCMLDPUINIT;

    typedef
        enum DPU_PROCESSING_MODE
    {
        DPU_PROCESSING_FULL_PASS = 0,
        DPU_PROCESSING_QUICK_PASS = 1
    } 	DPU_PROCESSING_MODE;

    typedef struct tagSyncMLDPUParams
    {
        LPCWSTR pszBody;
        DWORD dwMsgID;
        DWORD dwClientMsgID;
        BOOL fParseOnlySyncHdrStatus;
        LPCWSTR pszServerVersion;
        BOOL fIsPhoneOS;
        HRESULT hrParseOnly;
        DPU_PROCESSING_MODE processingMode;
    } 	SYNCMLDPUPARAMS;

    typedef struct tagSyncMLDPUParams *PSYNCMLDPUPARAMS;

    typedef
        enum AuthType
    {
        at_basic = 0,
        at_md5 = (at_basic + 1),
        at_hmac = (at_md5 + 1),
        at_transport = (at_hmac + 1),
        at_Unknown = (at_transport + 1)
    } 	AUTH_TYPE;

#define SYNCMLDPU_RESULTS_FINAL_ELEMENT_PRESENT            0x00000001
#define SYNCMLDPU_RESULTS_NEXT_MESSAGE_ALERT_PRESENT       0x00000002
#define SYNCMLDPU_RESULTS_NONSTATUS_OR_NONALERT_PRESENT    0x00000004
    typedef struct ALERTINFORESULT
    {
        LPWSTR pszCmdIDRef;
        DWORD dwStatusCode;
    } 	ALERTINFORESULT;

    typedef struct ALERTINFORESULT *PALERTINFORESULT;

    typedef struct tagSyncMLDPUResults
    {
        DWORD dwCmdCount;
        DWORD dwSyncHdrStatus;
        LPWSTR pszResultsXML;
        LPWSTR pszNextNonce;
        AUTH_TYPE atAuthType;
        BOOL fSessionAborted;
        PALERTINFORESULT rgairAlertStatuses;
        DWORD cairAlertStatuses;
        DWORD grfParseResults;
    } 	SYNCMLDPURESULTS;

    typedef struct tagSyncMLDPUResults *PSYNCMLDPURESULTS;



    extern RPC_IF_HANDLE __MIDL_itf_provdpu_0000_0000_v0_0_c_ifspec;
    extern RPC_IF_HANDLE __MIDL_itf_provdpu_0000_0000_v0_0_s_ifspec;

#ifndef __IProvisioningDPU_INTERFACE_DEFINED__
#define __IProvisioningDPU_INTERFACE_DEFINED__

    /* interface IProvisioningDPU */
    /* [uuid][object] */


    EXTERN_C const IID IID_IProvisioningDPU;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("ED3799A5-8188-4414-B9AB-BD37F064DDCD")
        IProvisioningDPU : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize(
            /* [size_is][in] */ __RPC__in_ecount_full(cbData) LPBYTE pbData,
            /* [in] */ DWORD cbData) = 0;

        virtual HRESULT STDMETHODCALLTYPE ProcessData(
            /* [size_is][in] */ __RPC__in_ecount_full(cbData) LPBYTE pbData,
            /* [in] */ DWORD cbData,
            /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*pcbResults) LPBYTE *ppbResults,
            /* [out] */ __RPC__out DWORD *pcbResults) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetResultsData(
            /* [size_is][out][in] */ __RPC__inout_ecount_full(cbData) LPBYTE pbData,
            /* [in] */ DWORD cbData) = 0;

    };


#else 	/* C style interface */

    typedef struct IProvisioningDPUVtbl
    {
        BEGIN_INTERFACE

            HRESULT(STDMETHODCALLTYPE *QueryInterface)(
                __RPC__in IProvisioningDPU * This,
                /* [in] */ __RPC__in REFIID riid,
                /* [annotation][iid_is][out] */
                _COM_Outptr_  void **ppvObject);

        ULONG(STDMETHODCALLTYPE *AddRef)(
            __RPC__in IProvisioningDPU * This);

        ULONG(STDMETHODCALLTYPE *Release)(
            __RPC__in IProvisioningDPU * This);

        HRESULT(STDMETHODCALLTYPE *Initialize)(
            __RPC__in IProvisioningDPU * This,
            /* [size_is][in] */ __RPC__in_ecount_full(cbData) LPBYTE pbData,
            /* [in] */ DWORD cbData);

        HRESULT(STDMETHODCALLTYPE *ProcessData)(
            __RPC__in IProvisioningDPU * This,
            /* [size_is][in] */ __RPC__in_ecount_full(cbData) LPBYTE pbData,
            /* [in] */ DWORD cbData,
            /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*pcbResults) LPBYTE *ppbResults,
            /* [out] */ __RPC__out DWORD *pcbResults);

        HRESULT(STDMETHODCALLTYPE *GetResultsData)(
            __RPC__in IProvisioningDPU * This,
            /* [size_is][out][in] */ __RPC__inout_ecount_full(cbData) LPBYTE pbData,
            /* [in] */ DWORD cbData);

        END_INTERFACE
    } IProvisioningDPUVtbl;

    interface IProvisioningDPU
    {
        CONST_VTBL struct IProvisioningDPUVtbl *lpVtbl;
    };



#ifdef COBJMACROS


#define IProvisioningDPU_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IProvisioningDPU_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IProvisioningDPU_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IProvisioningDPU_Initialize(This,pbData,cbData)	\
    ( (This)->lpVtbl -> Initialize(This,pbData,cbData) ) 

#define IProvisioningDPU_ProcessData(This,pbData,cbData,ppbResults,pcbResults)	\
    ( (This)->lpVtbl -> ProcessData(This,pbData,cbData,ppbResults,pcbResults) ) 

#define IProvisioningDPU_GetResultsData(This,pbData,cbData)	\
    ( (This)->lpVtbl -> GetResultsData(This,pbData,cbData) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IProvisioningDPU_INTERFACE_DEFINED__ */



#ifndef __ProvisioningDPU_LIBRARY_DEFINED__
#define __ProvisioningDPU_LIBRARY_DEFINED__

    /* library ProvisioningDPU */
    /* [helpstring][uuid] */


    EXTERN_C const IID LIBID_ProvisioningDPU;

    EXTERN_C const CLSID CLSID_CProvisioningDPU;

#ifdef __cplusplus

    class DECLSPEC_UUID("5AF61EC4-BF8D-4094-A340-F50E3D52B8B1")
        CProvisioningDPU;
#endif

    EXTERN_C const CLSID CLSID_CSyncMLDPU;

#ifdef __cplusplus

    class DECLSPEC_UUID("E3784839-6BD5-4702-A7B0-59C7592FB7B8")
        CSyncMLDPU;
#endif
#endif /* __ProvisioningDPU_LIBRARY_DEFINED__ */

    /* Additional Prototypes for ALL interfaces */

    /* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


