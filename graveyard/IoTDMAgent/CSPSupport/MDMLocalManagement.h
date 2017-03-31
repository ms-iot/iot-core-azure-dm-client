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
#include <windows.h>

/*++

Routine Description:

This API is used to register a device with Local MDM Management synchronously.
If a device is already registered, out parameter alreadyRegister is set to
TRUE and function returns S_OK. In all other cases, out parameter alreadyRegistered
is set to FALSE.

Return Value:

HRESULT indicating success or failure.
--*/
STDAPI
RegisterDeviceWithLocalManagement(
    _Out_opt_ BOOL* alreadyRegistered);

/*++

Routine Description:

This function is used to execute a SyncML.
The device must invoke RegisterDeviceWithLocalManagement prior to calling this function.

Arguments:

syncMLRequest - Null terminated string containing SyncML request.

syncMLResult - Null terminated string containing SyncML result.
Caller is responsible releasing memory allocated with LocalFree.

Return Value:

HRESULT indicating success or failure.
--*/
STDAPI
ApplyLocalManagementSyncML(
    _In_                          PCWSTR syncMLRequest,
    _Outptr_opt_result_maybenull_ PWSTR* syncMLResult
);

/*++

Routine Description:

This function is used to unregister a device with Local MDM Management synchronously.

Return Value:

HRESULT indicating success or failure.
--*/
STDAPI
UnregisterDeviceWithLocalManagement();



