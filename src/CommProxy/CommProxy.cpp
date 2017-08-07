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

#include "stdafx.h"
#include <Windows.h>
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"
#include "Blob.h"
#include "..\SharedUtilities\SystemConfiguratorPipe.h"
#include "StringResponse.h"

using namespace Microsoft::Devices::Management::Message;
using namespace std;
using namespace Utils;

int main(Platform::Array<Platform::String^>^ args)
{
    TRACE(__FUNCTION__);

    Utils::AutoCloseHandle stdinHandle(GetStdHandle(STD_INPUT_HANDLE));
    Utils::AutoCloseHandle stdoutHandle(GetStdHandle(STD_OUTPUT_HANDLE));

    int retCode = 1;

    try
    {
        TRACE("Reading request from stdin...");
        Blob^ request = Blob::ReadFromNativeHandle(stdinHandle.Get64());
        request->ValidateVersion();

        Blob^ response = SystemConfiguratorPipe::Send(request);

        TRACE("Writing to stdout...");
        response->WriteToNativeHandle(stdoutHandle.Get64());

        TRACE("Completed successfully.");

        // Return code 0 means the caller should get the output from the output stream
        retCode = 0;
    }
    catch (Exception^ ex)
    {
        TRACEP(L"Exception caught in CommProxy.exe: ", ex->Message->Data());

        auto response = ref new StringResponse(ResponseStatus::Failure, ex->Message, DMMessageKind::ErrorResponse);
        response->Serialize()->WriteToNativeHandle(stdoutHandle.Get64());
    }
    catch (...)
    {
        TRACE(L"Unknown exception caught in CommProxy.exe.");
    }

    return retCode;
}

