#pragma once

#include <string>
#include <windows.h>

class AppCfg
{
public:
    static void ManageApp(const std::wstring& appId, bool start);
};