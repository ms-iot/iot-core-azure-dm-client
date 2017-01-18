#pragma once

#include <string>

class AzureBlobCfg
{
public:
#ifdef AZURE_BLOB_SDK_FOR_ARM
	static void DownloadFile(const std::wstring& storageConnection, const std::wstring& container, const std::wstring& blob, const std::wstring& dest);
	static void UploadFile(const std::wstring& src, const std::wstring& storageConnection, const std::wstring& container, const std::wstring& blob);
#endif
};