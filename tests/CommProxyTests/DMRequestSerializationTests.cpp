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
#include "CppUnitTest.h"

#include "Utils.h"
#include "Logger.h"
#include "DMRequest.h"

#include "Models\StartupApp.h"
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
            auto appInstallInfo = ref new AppInstallRequestData("abc", StartUpType::None, "def", deps, "ghi", "jkl");
            auto ireg = ref new AppInstallRequest(appInstallInfo);
            auto blob = ireg->Serialize();
            auto req = dynamic_cast<AppInstallRequest^>(AppInstallRequest::Deserialize(blob));
            Assert::AreEqual(req->data->AppxPath, appInstallInfo->AppxPath);
        }

        TEST_METHOD(TestIResponseSerialization)
        {
            ResponseStatus statuses[] = { ResponseStatus::Success, ResponseStatus::Failure};

            for (auto status : statuses)
            {
                auto iresponse = ref new StatusCodeResponse(status, DMMessageKind::AddStartupApp);
                auto blob = iresponse->Serialize();
                auto req = dynamic_cast<StatusCodeResponse^>(StatusCodeResponse::Deserialize(blob));
                Assert::IsTrue(req->Status == status);
            }
        }

        TEST_METHOD(TestIRequestSerializationThroughBlob)
        {
            auto deps = ref new Vector<String^>();
            auto appInstallInfo = ref new AppInstallRequestData("abc", StartUpType::None, "def", deps, "ghi" , "jkl");
            auto ireg = ref new AppInstallRequest(appInstallInfo);
            auto blob = ireg->Serialize();
            auto payload = blob->MakeIRequest();
            auto req = (AppInstallRequest^)payload;
            Assert::AreEqual(req->data->AppxPath, appInstallInfo->AppxPath);
        }

        TEST_METHOD(TestIResponseSerializationThroughBlob)
        {
            auto iresponse = ref new StatusCodeResponse(ResponseStatus::Success, DMMessageKind::AddStartupApp);
            auto blob = iresponse->Serialize();
            auto payload = blob->MakeIResponse();
            auto response = (StatusCodeResponse^)(payload);
            Assert::IsTrue(response->Status == ResponseStatus::Success);
        }

        Blob^ RoundTripThroughNativeHandle(Blob^ inputBlob)
        {
            Utils::AutoCloseHandle pipeHandleWrite = CreateNamedPipeW(
                PIPE_NAME L"-test",
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                PipeBufferSize,
                PipeBufferSize,
                NMPWAIT_USE_DEFAULT_WAIT,
                nullptr);

            Utils::AutoCloseHandle pipeHandleRead = CreateFileW(PIPE_NAME L"-test",
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,
                0,
                nullptr);

            inputBlob->WriteToNativeHandle(pipeHandleWrite.Get64());

            return Blob::ReadFromNativeHandle(pipeHandleRead.Get64());
        }

        TEST_METHOD(TestRequestRoundTripThroughNativeHandle)
        {
            auto deps = ref new Vector<String^>();
            auto appInstallInfo = ref new AppInstallRequestData("abc", StartUpType::None, "def", deps, "ghi", "jkl");
            auto req = ref new AppInstallRequest(appInstallInfo);
            auto blob = RoundTripThroughNativeHandle(req->Serialize());
            auto req2 = dynamic_cast<AppInstallRequest^>(AppInstallRequest::Deserialize(blob));
            Assert::AreEqual(req2->data->AppxPath, appInstallInfo->AppxPath);
        }

        TEST_METHOD(TestResponseRoundTripThroughNativeHandle)
        {
            ResponseStatus statuses[] = { ResponseStatus::Success, ResponseStatus::Failure };

            for (auto status : statuses)
            {
                auto app = ref new Platform::String(L"abc");
                auto response = ref new GetStartupForegroundAppResponse(status, app);
                auto blob = RoundTripThroughNativeHandle(response->Serialize());
                auto req = dynamic_cast<GetStartupForegroundAppResponse^>(GetStartupForegroundAppResponse::Deserialize(blob));
                Assert::IsTrue(req->Status == status);
                Assert::AreEqual(app, req->StartupForegroundApp);
            }
        }
    };
}
