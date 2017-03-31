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

#include "LimpetDefs.h"

#define LIMPET_API extern "C" __declspec (dllexport) UINT32 __cdecl

LIMPET_API _LimpetInitialize();

LIMPET_API _LimpetDestroy();

LIMPET_API _LimpetGetTpmKey2BPubFromCert(
    __in_bcount(cbCert) PBYTE pbCert,
    UINT32 cbCert,
    __out_bcount_part(cbIdKeyPub, *pcbIdKeyPub) PBYTE pbIdKeyPub,
    __in UINT32 cbIdKeyPub,
    __out PUINT32 pcbIdKeyPub,
    __out_bcount_opt(SHA256_DIGEST_SIZE + sizeof(UINT16)) PBYTE idKeyName
);

LIMPET_API _LimpetGetTpmKey2BPub(
    __in UINT32 tpmKeyHandle,
    __out_bcount_part(cbPubKey, *pcbPubKey) PBYTE pbPubKey,
    __in UINT32 cbPubKey,
    __out PUINT32 pcbPubKey,
    __out_bcount_opt(SHA256_DIGEST_SIZE + sizeof(UINT16)) PBYTE pubKeyName
);

LIMPET_API _LimpetReadDeviceId(
    __in UINT32 LogicalDeviceNumber,
    __out_ecount_z(LIMPET_DEVICE_ID_STRING_LENGTH) WCHAR* wcDeviceId
);

LIMPET_API _LimpetStoreURI(
    __in UINT32 logicalDeviceNumber,
    __in_ecount_z(LIMPET_STRING_SIZE) WCHAR* szURI,
    __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcName
);

LIMPET_API _LimpetReadURI(
    __in UINT32 logicalDeviceNumber,
    __out_bcount_part(cbUriData, *pcbUriData) PBYTE pbUriData,
    __in UINT32 cbUriData,
    __out PUINT32 pcbUriData
);

LIMPET_API _LimpetDestroyURI(
    __in UINT32 logicalDeviceNumber,
    __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcName
);

LIMPET_API _LimpetCreateHmacKey(
    __in UINT32 logicalDeviceNumber,
    __in_bcount(cbKey) PBYTE pbKey,
    __in UINT32 cbKey,
    __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcName
);

LIMPET_API _LimpetEvictHmacKey(
    __in UINT32 logicalDeviceNumber,
    __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcName
);

LIMPET_API _LimpetSymmetricPayloadProtection(
    __in bool encrypt,
    __in_bcount(MAX_AES_KEY_BYTES) PBYTE aes128Key,
    __in_ecount_z(LIMPET_STRING_SIZE) WCHAR* wcfileName
);

LIMPET_API _LimpetReadOrCreateIdentityKeyCert(
    __in UINT32 logicalDeviceNumber,
    __in_z LPCWSTR wcAlgorithm,
    __out_bcount_part(cbCert, *pcbCert) PBYTE pbCert,
    __in UINT32 cbCert,
    __out PUINT32 pcbCert,
    __out_bcount(SHA1_DIGEST_SIZE) PBYTE certThumbPrint,
    __out_bcount_part(cbIdKeyPub, *pcbIdKeyPub) PBYTE pbIdKeyPub,
    __in UINT32 cbIdKeyPub,
    __out PUINT32 pcbIdKeyPub,
    __out_bcount(SHA256_DIGEST_SIZE + sizeof(UINT16)) PBYTE idKeyName,
    __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcName
);

LIMPET_API _LimpetDestroyIdentityCert(
    __in UINT32 logicalDeviceNumber,
    __in_z LPCWSTR wcAlgorithm,
    __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcName
);

LIMPET_API _LimpetImportIdentityCert(
    __in_bcount(cbNewCert) PBYTE pbNewCert,
    __in UINT32 cbNewCert,
    __out_bcount(SHA1_DIGEST_SIZE) PBYTE certThumbPrint,
    __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcName
);

LIMPET_API _LimpetSignWithIdentityCert(
    __in_bcount(SHA1_DIGEST_SIZE) PBYTE certThumbPrint,
    __in_bcount(cbDataToSign) PBYTE pbDataToSign,
    __in UINT32 cbDataToSign,
    __out_ecount_z(LIMPET_STRING_SIZE) WCHAR* subjectName,
    __out_bcount_part(cbSignature, *pcbSignature) PBYTE pbSignature,
    __in UINT32 cbSignature,
    __out PUINT32 pcbSignature
);

LIMPET_API _LimpetSignWithIdentityHmacKey(
    __in UINT32 LogicalDeviceNumber,
    __in_bcount(cbDataToSign) PBYTE pbDataToSign,
    __in UINT32 cbDataToSign,
    __out_bcount(SHA256_DIGEST_SIZE) PBYTE pbSignature
);

LIMPET_API _LimpetGenerateSASToken(
    __in UINT32 logicalDeviceNumber,
    __in LARGE_INTEGER* expiration,
    __out_ecount_z(MAX_CONNECTION_STRING_LEN) WCHAR* szConnectionString
);

LIMPET_API _LimpetIssueCertificate(
    __in_bcount(SHA1_DIGEST_SIZE) PBYTE certThumbPrint,
    __in_bcount(cbCertReq) PBYTE pbCertReq,
    UINT32 cbCertReq,
    __in_ecount_z(LIMPET_STRING_SIZE) WCHAR* deviceName,
    __in_ecount_z(LIMPET_STRING_SIZE) WCHAR* url,
    __out_bcount_opt(sizeof(UINT16) + SHA256_DIGEST_SIZE) PBYTE tpmKeyName,
    __out_bcount(SHA1_DIGEST_SIZE) PBYTE newCertThumbPrint,
    __out_bcount_part(cbOutput, *pcbResult) PBYTE pbOutput,
    UINT32 cbOutput,
    __out PUINT32 pcbResult
);

LIMPET_API _LimpetChallengeIdentityCert(
    __in_bcount(cbEkPub) PBYTE pbEkPub,
    UINT32 cbEkPub,
    __in_bcount(sizeof(UINT16) + SHA256_DIGEST_SIZE) PBYTE idkName,
    __inout_bcount(MAX_AES_KEY_BYTES) PBYTE activationSecret,
    __out_bcount_part(cbOutput, *pcbResult) PBYTE pbOutput,
    UINT32 cbOutput,
    __out PUINT32 pcbResult
);

LIMPET_API _LimpetActivateIdentityCert(
    __in UINT32 logicalDeviceNumber,
    __in_z LPCWSTR wcAlgorithm,
    __in_bcount(cbActivation) PBYTE pbActivation,
    UINT32 cbActivation,
    __out_bcount(MAX_AES_KEY_BYTES) PBYTE activationSecret
);

LIMPET_API _LimpetCreateHostageKey(
    __in_bcount(cbEkPub) PBYTE pbEkPub,
    UINT32 cbEkPub,
    __in_bcount(cbSrkPub) PBYTE pbSrkPub,
    UINT32 cbSrkPub,
    __in_ecount_z(LIMPET_STRING_SIZE) WCHAR* deviceName,
    __in_ecount_z(LIMPET_STRING_SIZE) WCHAR* url,
    __in_bcount(SHA256_DIGEST_SIZE) PBYTE hmacKey,
    __inout_bcount(MAX_AES_KEY_BYTES) PBYTE activationSecret,
    __out_bcount_part(cbOutput, *pcbResult) PBYTE pbOutput,
    UINT32 cbOutput,
    __out PUINT32 pcbResult
);

LIMPET_API _LimpetImportHostageKey(
    __in UINT32 logicalDeviceNumber,
    __in_bcount(cbHostageBlob) PBYTE pbHostageBlob,
    UINT32 cbHostageBlob,
    __inout_bcount(MAX_AES_KEY_BYTES) PBYTE activationSecret,
    __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcKeyName,
    __out_ecount_z_opt(LIMPET_STRING_SIZE) WCHAR* wcUriName
);
