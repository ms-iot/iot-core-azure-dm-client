#include "stdafx.h"
#include "AzureBlobCfg.h"

#ifdef AZURE_BLOB_SDK_FOR_ARM

// azure
#include <limits>
#include "was/storage_account.h"
#include "was/blob.h"
#include "cpprest/filestream.h"

#include "..\SharedUtilities\Logger.h"

using namespace std;
//using namespace Utils;

void AzureBlobCfg::DownloadFile(const wstring& storageConnection, const wstring& containerName, const wstring& blobName, const wstring& dest)
{
    TRACE(__FUNCTION__);

	// Retrieve storage account from connection string.
	azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storageConnection);

	// Create the blob client.
	azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();

	// Retrieve a reference to (previously created) container.
	azure::storage::cloud_blob_container container = blob_client.get_container_reference(containerName);

	// Retrieve reference to blob.
	azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(blobName);

	// Create or overwrite the blob with contents from local file.
	blockBlob.download_to_file(dest);
}

void AzureBlobCfg::UploadFile(const wstring& src, const wstring& storageConnection, const wstring& containerName, const wstring& blobName)
{
	TRACE(__FUNCTION__);

	// Retrieve storage account from connection string.
	azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storageConnection);

	// Create the blob client.
	azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();

	// Retrieve a reference to a previously created container.
	azure::storage::cloud_blob_container container = blob_client.get_container_reference(containerName);

	// Create the container if it doesn't already exist.
	container.create_if_not_exists();

	// Retrieve reference to a blob named "my-blob-1".
	azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(blobName);

	// Save blob contents to a file.
	blockBlob.upload_from_file(src);
}

#endif
