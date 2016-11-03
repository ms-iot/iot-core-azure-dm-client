#include "stdafx.h"

class security_attributes
{
    SECURITY_ATTRIBUTES _securityAttributes;
    PSID pEveryoneSID;
    PACL pACL;
    PSECURITY_DESCRIPTOR pSD;
public:
    security_attributes(DWORD permissions) : pEveryoneSID(nullptr), pACL(nullptr), pSD(nullptr)
    {
        SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

        // Create a well-known SID for the Everyone group.
        if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
        {
            printf("AllocateAndInitializeSid Error %u\n", GetLastError());
            return;
        }

        EXPLICIT_ACCESS ea = { 0 };
        ea.grfAccessPermissions = permissions;
        ea.grfAccessMode = SET_ACCESS;
        ea.grfInheritance = NO_INHERITANCE;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea.Trustee.ptstrName = (LPTSTR)pEveryoneSID;

        // Create a new ACL that contains the new ACEs.
        DWORD dwRes = SetEntriesInAclW(1, &ea, NULL, &pACL);
        if (dwRes != ERROR_SUCCESS)
        {
            printf("SetEntriesInAcl Error %u\n", GetLastError());
            return;
        }

        // Initialize a security descriptor.  
        pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (pSD == NULL)
        {
            printf("LocalAlloc Error %u\n", GetLastError());
            return;
        }

        if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
        {
            printf("InitializeSecurityDescriptor Error %u\n", GetLastError());
            return;
        }

        // Add the ACL to the security descriptor. 
        if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
        {
            printf("SetSecurityDescriptorDacl Error %u\n", GetLastError());
            return;
        }

        // Initialize a security attributes structure.
        _securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        _securityAttributes.lpSecurityDescriptor = pSD;
        _securityAttributes.bInheritHandle = FALSE;
    }

    ~security_attributes()
    {
        if (pEveryoneSID)
            FreeSid(pEveryoneSID);
        if (pACL)
            LocalFree(pACL);
        if (pSD)
            LocalFree(pSD);
    }

    LPSECURITY_ATTRIBUTES get_sa()
    {
        return &_securityAttributes;
    }
};

// HACK: this gives all access to anyone, allows client impersonation
void init_security_attributes2(LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    auto pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        printf("InitializeSecurityDescriptor Error %d\n", GetLastError());
        return;
    }

    if (!SetSecurityDescriptorDacl(pSD, TRUE, NULL, FALSE))
    {
        printf("SetSecurityDescriptorDacl Error %d\n", GetLastError());
        return;
    }

    lpSecurityAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
    lpSecurityAttributes->lpSecurityDescriptor = pSD;
    lpSecurityAttributes->bInheritHandle = FALSE;
}

#define BUFSIZE 4096

const wchar_t* pipename_r = L"\\\\.\\pipe\\dm-client-pipe-r";
const wchar_t* pipename_w = L"\\\\.\\pipe\\dm-client-pipe-w";

class MessageQueue
{
    std::queue<std::string> _messages;
    std::mutex _mutex;
    std::condition_variable _cond;
public:
    void push(std::string msg)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _messages.push(msg);
        _cond.notify_one();
    }

    std::string pop()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (_messages.empty())
        {
            _cond.wait(lock);
        }
        auto str = _messages.front();
        _messages.pop();
        return str;
    }
};

MessageQueue g_queue;

// Reads from the pipe
void np_reader()
{
    HANDLE hPipe;

    while (true)
    {
        security_attributes sa(GENERIC_WRITE | GENERIC_READ);

        hPipe = CreateNamedPipeW(
            pipename_r,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            BUFSIZE,
            BUFSIZE,
            NMPWAIT_USE_DEFAULT_WAIT,
            sa.get_sa());

        if (hPipe != INVALID_HANDLE_VALUE)
        {
            printf("Reader: waiting for someone to connect...\n");
            BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ?
                TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

            if (fConnected)
            {
                printf("Reader: client connected\n");
                std::thread worker([hPipe] {
                    char buffer[BUFSIZE];
                    DWORD dwRead;
                    while (true)
                    {
                        printf("Reader: reading data...\n");
                        //while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
                        BOOL readResult = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL);
                        if (readResult && dwRead > 0)
                        {
                            /* add terminating zero */
                            buffer[dwRead] = '\0';

                            /* do something with data in buffer */
                            printf("Reader: received '%s'\n", buffer);

                            for (int i = 0; i < (int)dwRead; ++i)
                            {
                                buffer[i] = toupper(buffer[i]);
                            }

                            g_queue.push(std::string(buffer));
                        }
                        else {
                            if (GetLastError() == ERROR_BROKEN_PIPE)
                            {
                                printf("Reader: client disconnected\n");
                            }
                            else
                            {
                                printf("Reader: ReadFile Error %d\n", GetLastError());
                            }
                            break;
                        }
                    }

                    FlushFileBuffers(hPipe);
                    DisconnectNamedPipe(hPipe);
                    CloseHandle(hPipe);

                });
                worker.detach();
            }
            else
            {
                printf("Reader: ConnectNamedPipe Error %d\n", GetLastError());
            }
        }
        else
        {
            printf("Reader: CreateNamedPipe Error %d\n", GetLastError());
        }
    }
}

// Writes into the pipe
void np_writer()
{
    HANDLE hPipe;

    while (true)
    {
        security_attributes sa(GENERIC_WRITE | GENERIC_READ);

        hPipe = CreateNamedPipeW(
            pipename_w,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            BUFSIZE,
            BUFSIZE,
            NMPWAIT_USE_DEFAULT_WAIT,
            sa.get_sa());

        if (hPipe != INVALID_HANDLE_VALUE)
        {
            printf("Writer: waiting for someone to connect...\n");
            BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ?
                TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

            if (fConnected)
            {
                printf("Writer: client connected\n");
                std::thread worker([hPipe] {
                    while (true)
                    {
                        auto msg = g_queue.pop();
                        printf("Writer: got data to send\n");

                        DWORD dwWritten;
                        DWORD numberOfBytesToWrite = min(msg.length(), BUFSIZE);

                        BOOL writeResult = WriteFile(hPipe,
                            msg.c_str(),
                            numberOfBytesToWrite,
                            &dwWritten,
                            NULL);

                        if (writeResult && dwWritten > 0) {
                            printf("Writer: message '%s' written\n", msg.c_str());
                        }
                        else {
                            if (GetLastError() == ERROR_BROKEN_PIPE)
                            {
                                printf("Writer: client disconnected\n");
                            }
                            else
                            {
                                printf("Writer: WriteFile Error %d\n", GetLastError());
                            }
                            break;
                        }
                    }
                    FlushFileBuffers(hPipe);
                    DisconnectNamedPipe(hPipe);
                    CloseHandle(hPipe);
                });
                worker.detach();
            }
            else
            {
                printf("Writer: ConnectNamedPipe Error %d\n", GetLastError());
            }
        }
        else
        {
            printf("Writer: CreateNamedPipe Error %d\n", GetLastError());
        }
    }
}

int main()
{
    std::thread reader(np_reader);
    std::thread writer(np_writer);

    reader.join();
    writer.join();
}


