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

#include <stdafx.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\Utils.h"
#include "DMStorage.h"

using namespace std;
using namespace experimental::filesystem;
using namespace Microsoft::Devices::Management::Message;

Vector<String^>^ GetFSObjectNames(const wstring& path, file_type type)
{
    TRACEP(L"Scanning: ", path.c_str());

    Vector<String^>^ vector = ref new Vector<String^>();

    for (const directory_entry& dirEntry : directory_iterator(path))
    {
        TRACEP(L"Found: ", dirEntry.path());
        if (dirEntry.status().type() != type)
        {
            continue;
        }

        wstring folderName = dirEntry.path().filename().c_str();
        vector->Append(ref new String(folderName.c_str()));

        TRACEP(L"  Picked: ", folderName.c_str());
    }

    return vector;
}

IResponse^ DMStorage::HandleGetDMFolders(IRequest^ request)
{
    TRACE(__FUNCTION__);

    wstring path = Utils::GetDmUserFolder();

    StringListResponse^ response = ref new StringListResponse(ResponseStatus::Success);
    response->List = GetFSObjectNames(path, file_type::directory);
    return response;
}

IResponse^ DMStorage::HandleGetDMFiles(IRequest^ request)
{
    TRACE(__FUNCTION__);

    GetDMFilesRequest^ filesRequest = dynamic_cast<GetDMFilesRequest^>(request);

    wstring path;
    path += Utils::GetDmUserFolder();
    path += L"\\";
    path += filesRequest->DMFolderName->Data();

    StringListResponse^ response = ref new StringListResponse(ResponseStatus::Success);
    response->List = GetFSObjectNames(path, file_type::regular);
    return response;
}

IResponse^ DMStorage::HandleDeleteDMFile(IRequest^ request)
{
    TRACE(__FUNCTION__);

    DeleteDMFileRequest^ deleteRequest = dynamic_cast<DeleteDMFileRequest^>(request);

    wstring fullFileName;
    fullFileName += Utils::GetDmUserFolder();
    fullFileName += L"\\";
    fullFileName += deleteRequest->DMFolderName->Data();
    fullFileName += L"\\";
    fullFileName += deleteRequest->DMFileName->Data();

    TRACEP(L"Deleting: ", fullFileName.c_str());
    DeleteFile(fullFileName.c_str());

    return ref new StatusCodeResponse(ResponseStatus::Success, request->Tag);
}
