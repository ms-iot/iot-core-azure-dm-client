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

#define LIMPET_TPM_OBJECT_NAME             (sizeof(UINT16) + SHA256_DIGEST_SIZE)
#define LIMPET_DEVICE_ID_STRING_LENGTH     (70) 
#define LIMPET_STRING_SIZE                 (512)
#define MAX_CONNECTION_STRING_LEN          (1024)

#define LIMPET_TPM20_SRK_HANDLE            (0x81000001)
#define LIMPET_TPM20_ERK_HANDLE            (0x81010001)
#define LIMPET_TPM20_PERSISTED_KEY_INDEX   (0x81000100)

#define LIMPET_TPM20_HT_NV_INDEX           (0x01)
#define LIMPET_TPM20_OWNER_NV_SPACE        ((LIMPET_TPM20_HT_NV_INDEX << 24) | (0x01 << 22))
#define LIMPET_TPM20_PERSISTED_URI_INDEX   (LIMPET_TPM20_OWNER_NV_SPACE + 0x100)
#define LIMPET_TPM20_MAX_LOGICAL_DEVICE    (10)
#define LIMPET_HARDWARE_DEVICE_ID_SIZE     (0x22)
