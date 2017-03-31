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

HANDLE create_pipe_worker(const wchar_t* pipeName)
{
    HANDLE hPipe;

    // Wait for named pipe to become available
    while (true)
    {
        hPipe = CreateFileW(pipeName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            printf("Could not open pipe. Is DM client running? GetLastError()=%d\n", GetLastError());
            return 0;
        }

        // All pipe instances are busy, so wait for 10 seconds. 

        if (!WaitNamedPipeW(pipeName, 10 * 1000))
        {
            printf("Could not open pipe: 10 second wait timed out.");
            return 0;
        }
    }

    return hPipe;
}

HANDLE create_r_pipe()
{
    return create_pipe_worker(L"\\\\.\\pipe\\dm-client-pipe-r");
}

HANDLE create_w_pipe()
{
    return create_pipe_worker(L"\\\\.\\pipe\\dm-client-pipe-w");
}

void send_data_to_named_pipe(HANDLE hPipe, char* request, uint32_t request_len)
{
    DWORD dwWritten = 0;

    BOOL bWrite = WriteFile(hPipe,
        request,
        request_len,
        &dwWritten,
        NULL);
    if (!bWrite)
    {
        printf("WriteFile failed, GetLastError()=%d\n", GetLastError());
    }
}

uint32_t receive_data_from_named_pipe(HANDLE hPipe, char* response, uint32_t max_response_len)
{
    DWORD dwRead = 0;

    while (true)
    {
        BOOL bRead = ReadFile(hPipe, response, max_response_len, &dwRead, NULL);
        if (!bRead)
        {
            DWORD dwLastError = GetLastError();
            if (dwLastError == ERROR_MORE_DATA)
            {
                // repeat loop if ERROR_MORE_DATA 
                continue;
            }
            else
            {
                printf("ReadFile failed, GetLastError()=%d\n", dwLastError);
            }
        }
        break;
    }

    return dwRead;
}

