#include "stdafx.h"
#include "CppUnitTest.h"

#include "Utils.h"
#include "Logger.h"
#include "DMRequest.h"
#include "SecurityAttributes.h"

#include "Models\AppInstall.h"
#include "Models\CheckForUpdates.h"
#include "Models\StatusCodeResponse.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Microsoft::Devices::Management::Message;
using namespace concurrency;

namespace CommProxyTests
{
    TEST_CLASS(DMRequestSerializationTests)
    {
        TEST_METHOD(TestIRequestSerialization)
        {
            auto deps = ref new Vector<String^>();
            auto appInstallInfo = ref new AppInstallInfo("abc", "def", deps);
            IRequest^ ireg = ref new AppInstallRequest(appInstallInfo);
            auto blob = ireg->Serialize();
            AppInstallRequest^ req = dynamic_cast<AppInstallRequest^>(AppInstallRequest::Deserialize(blob));
            Assert::AreEqual(req->AppInstallInfo->AppxPath, appInstallInfo->AppxPath);
        }

        TEST_METHOD(TestIResponseSerialization)
        {
            ResponseStatus statuses[] = { ResponseStatus::Success, ResponseStatus::Failure};

            for (auto status : statuses)
            {
                IResponse^ iresponse = ref new AppInstallResponse(status);

                auto blob = iresponse->Serialize();

                auto req = dynamic_cast<AppInstallResponse^>(AppInstallResponse::Deserialize(blob));
                Assert::IsTrue(req->Status == status);
            }
        }

        TEST_METHOD(TestIRequestSerializationThroughBlob)
        {
            auto deps = ref new Vector<String^>();
            auto appInstallInfo = ref new AppInstallInfo("abc", "def", deps);
            IRequest^ ireg = ref new AppInstallRequest(appInstallInfo);
            auto blob = ireg->Serialize();
            auto payload = blob->MakeIRequest();
            AppInstallRequest^ req = (AppInstallRequest^)payload;
            Assert::AreEqual(req->AppInstallInfo->AppxPath, appInstallInfo->AppxPath);
        }

        TEST_METHOD(TestIResponseSerializationThroughBlob)
        {
            IResponse^ iresponse = ref new AppInstallResponse(ResponseStatus::Success);

            auto blob = iresponse->Serialize();

            auto payload = blob->MakeIResponse();

            AppInstallResponse^ response = (AppInstallResponse^)(payload);
            Assert::IsTrue(response->Status == ResponseStatus::Success);
        }

        Blob^ RoundTripThroughNativeHandle(Blob^ inputBlob)
        {
            Utils::AutoCloseHandle pipeHandleWrite;
            pipeHandleWrite = CreateNamedPipeW(
                PIPE_NAME L"-test",
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                PipeBufferSize,
                PipeBufferSize,
                NMPWAIT_USE_DEFAULT_WAIT,
                nullptr);

            Utils::AutoCloseHandle pipeHandleRead;
            pipeHandleRead = CreateFileW(PIPE_NAME L"-test",
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,
                0,
                nullptr);

            inputBlob->WriteToNativeHandle(pipeHandleWrite.Get());

            return Blob::ReadFromNativeHandle(pipeHandleRead.Get());
        }

        TEST_METHOD(TestRequestRoundTripThroughNativeHandle)
        {
            auto deps = ref new Vector<String^>();
            auto appInstallInfo = ref new AppInstallInfo("abc", "def", deps);
            IRequest^ req = ref new AppInstallRequest(appInstallInfo);
            auto blob = RoundTripThroughNativeHandle(req->Serialize());
            auto req2 = dynamic_cast<AppInstallRequest^>(AppInstallRequest::Deserialize(blob));
            Assert::AreEqual(req2->AppInstallInfo->AppxPath, appInstallInfo->AppxPath);
        }

        TEST_METHOD(TestResponseRoundTripThroughNativeHandle)
        {
            ResponseStatus statuses[] = { ResponseStatus::Success, ResponseStatus::Failure };

            for (auto status : statuses)
            {
                auto response = ref new AppInstallResponse(status);
                auto blob = RoundTripThroughNativeHandle(response->Serialize());
                auto req = dynamic_cast<AppInstallResponse^>(AppInstallResponse::Deserialize(blob));
                Assert::IsTrue(req->Status == status);
            }
        }
    };
}