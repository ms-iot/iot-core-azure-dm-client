#include "stdafx.h"
#include "AzureStorage.h"
#include "DMException.h"

// Install-Package wastorage
#include "was/storage_account.h"
#include "was/blob.h"

using namespace std;

namespace AzureStorage
{

void Download(const wstring& connectionString, const wstring& containerName, const wstring& fileName, const wstring& targetFolder)
{
    TRACE(L"AzureStorage::Download()");
    TRACEP(L"Container    : ", containerName.c_str());
    TRACEP(L"File Name    : ", fileName.c_str());
    TRACEP(L"Target Folder: ", targetFolder.c_str());

    Utils::EnsureFolderExists(targetFolder);

    const utility::string_t storage_connection_string(connectionString);
    azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);
    azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();
    azure::storage::cloud_blob_container container = blob_client.get_container_reference(containerName);
    azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(fileName.c_str());

    concurrency::streams::container_buffer<vector<uint8_t>> buffer;
    concurrency::streams::ostream output_stream(buffer);
    blockBlob.download_to_stream(output_stream);

    wstring fullTargetName = targetFolder + L"\\" + fileName;
    ofstream outfile(fullTargetName.c_str(), ofstream::binary);

    vector<unsigned char>& data = buffer.collection();

    outfile.write((char *)&data[0], buffer.size());
    outfile.close();
}

}