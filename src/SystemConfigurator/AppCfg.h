#pragma once

#include <string>
#include <windows.h>

class AppCfg
{
public:
	static void StartApp(const std::wstring& appId);
	static void StopApp(const std::wstring& appId);
//private:
	static void ManageApp(const std::wstring& appId, bool start);
};