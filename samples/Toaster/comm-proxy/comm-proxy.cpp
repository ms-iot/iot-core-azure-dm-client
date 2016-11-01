#include "stdafx.h"
#include "app-service-client.h"

// TODO: rework to avoid any fixed-size buffers
#define BUFSIZE 4096

char buffer_to_named_pipe[BUFSIZE];
char buffer_from_named_pipe[BUFSIZE];
wchar_t wbuffer[BUFSIZE];

HANDLE create_r_pipe();
HANDLE create_w_pipe();

void send_data_to_named_pipe(HANDLE hPipe, char* request, uint32_t request_len);
uint32_t receive_data_from_named_pipe(HANDLE hPipe, char* response, uint32_t max_response_len);

class closer
{
    std::function<void(void)> _closing_action;
public:
    closer(std::function<void(void)> closing_action) : _closing_action(closing_action) {}
    ~closer()
    {
        _closing_action();
    }
};

int main(Platform::Array<Platform::String^>^ args)
{
    if (args->Length != 3)
    {
        printf("Syntax: comm-proxy <service name> <package family name>\n");
        return 1;
    }

    HANDLE r_pipe = create_r_pipe();
    closer close_r_pipe([&] {
        CloseHandle(r_pipe);
    });

    HANDLE w_pipe = create_w_pipe();
    closer close_w_pipe([&] {
        CloseHandle(w_pipe);
    });

    auto service_name = args[1];
    auto pkg_family_name = args[2];

    std::thread reader([=] {
        while (true)
        {
            uint32_t read_from_admin = receive_data_from_named_pipe(w_pipe, buffer_from_named_pipe, BUFSIZE);

            printf("received %d bytes from named pipe\n", read_from_admin);

            if (read_from_admin > 0)
            {
                int chars_converted = MultiByteToWideChar(CP_UTF8, 0, buffer_from_named_pipe, read_from_admin, wbuffer, BUFSIZE);
                send_data_to_UWP_app(wbuffer, chars_converted, service_name, pkg_family_name);
            }
            else
            {
                break;
            }
        }
    });

    closer join_reader_thread([&] { reader.join(); });

    auto stdinHandle = GetStdHandle(STD_INPUT_HANDLE);

    while (true)
    {
        DWORD read_from_stdin;
        BOOL bSuccess = ReadFile(stdinHandle, buffer_to_named_pipe, BUFSIZE, &read_from_stdin, NULL);

        if (!bSuccess || read_from_stdin == 0)
        {
            printf("end of stream! bSuccess=%d, dwRead=%d\n", bSuccess, read_from_stdin);
            break;
        }
        else
        {
            buffer_to_named_pipe[read_from_stdin] = '\0';
            printf("sending to named pipe: %s\n", buffer_to_named_pipe);

            send_data_to_named_pipe(r_pipe, buffer_to_named_pipe, read_from_stdin);
        }
    }
}

