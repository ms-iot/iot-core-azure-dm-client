#pragma once

#include <Windows.h>

STDAPI
RegisterDeviceWithLocalManagement(
    _Out_opt_ BOOL* alreadyRegistered);

STDAPI
ApplyLocalManagementSyncML(
    _In_                          PCWSTR syncMLRequest,
    _Outptr_opt_result_maybenull_ PWSTR* syncMLResult
);

STDAPI
UnregisterDeviceWithLocalManagement();

