#include "stdafx.h"

using namespace concurrency;
using namespace Windows::ApplicationModel::AppService;

wchar_t* AppServiceResponseStatusString(AppServiceResponseStatus status)
{
    switch (status)
    {
    case AppServiceResponseStatus::Failure:
        return L"Failure: The service failed to acknowledge the message we sent it. It may have been terminated because the client was suspended.";

    case AppServiceResponseStatus::ResourceLimitsExceeded:
        return L"ResourceLimitsExceeded: The service exceeded the resources allocated to it and had to be terminated.";

    case AppServiceResponseStatus::Unknown:
    default:
        return L"Unknown: An unkown error occurred while we were trying to send a message to the service.";
    }
}

wchar_t* AppServiceConnectionStatusString(AppServiceConnectionStatus status)
{
    switch (status)
    {
    case AppServiceConnectionStatus::Success:
        return L"Success";
    case AppServiceConnectionStatus::AppNotInstalled:
        return L"AppNotInstalled";
    case AppServiceConnectionStatus::AppUnavailable:
        return L"AppUnavailable";
    case AppServiceConnectionStatus::AppServiceUnavailable:
        return L"AppServiceUnavailable";
    case AppServiceConnectionStatus::Unknown:
        return L"Unknown";
    case AppServiceConnectionStatus::RemoteSystemUnavailable:
        return L"RemoteSystemUnavailable";
    case AppServiceConnectionStatus::RemoteSystemNotSupportedByApp:
        return L"RemoteSystemNotSupportedByApp";
    case AppServiceConnectionStatus::NotAuthorized:
        return L"NotAuthorized";
    default:
        return L"??";
    }
}

void send_data_to_UWP_app(wchar_t* data, uint32_t length, Platform::String^ service_name, Platform::String^ pkg_family_name)
{
    Windows::ApplicationModel::AppService::AppServiceConnection^ newConnection = ref new Windows::ApplicationModel::AppService::AppServiceConnection();

    newConnection->AppServiceName = service_name;
    newConnection->PackageFamilyName = pkg_family_name;

    create_task(newConnection->OpenAsync()).then([&](task<Windows::ApplicationModel::AppService::AppServiceConnectionStatus> statusTask) {
        auto status = statusTask.get();
        if (status != Windows::ApplicationModel::AppService::AppServiceConnectionStatus::Success)
        {
            wprintf(L"Error '%s'\n", AppServiceConnectionStatusString(status));
            return task_from_result();
        }

        auto inputs = ref new Windows::Foundation::Collections::ValueSet();
        inputs->Insert(L"input", ref new Platform::String(data, length));

        return create_task(newConnection->SendMessageAsync(inputs)).then([](task<Windows::ApplicationModel::AppService::AppServiceResponse^> responseTask) {
            try
            {
                auto response = responseTask.get();
                auto status = response->Status;
                if (status == Windows::ApplicationModel::AppService::AppServiceResponseStatus::Success)
                {
                    auto resultText = response->Message->Lookup(L"ack");
                    wprintf(L"ack from UWP: %s\n", resultText->ToString()->Data());
                }
                else
                {
                    wprintf(L"Error: '%s'\n", AppServiceResponseStatusString(status));
                }
            }
            catch (...)
            {
                wprintf(L"Error: responseTask threw an exception\n");
            }
        });
    }).wait();
}