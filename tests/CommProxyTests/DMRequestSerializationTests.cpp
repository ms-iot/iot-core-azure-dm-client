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
#include "Models\WifiConfiguration.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Microsoft::Devices::Management::Message;
using namespace concurrency;

namespace CommProxyTests
{
    TEST_CLASS(DMRequestSerializationTests)
    {
        template<class T>
        void TestBlob(T blob, DMMessageKind kind)
        {
            Assert::IsTrue(blob != nullptr);
            Assert::IsTrue(blob->Tag == kind);
        }

        template<class T>
        void TestBlobAndConfig(T blob, DMMessageKind kind, uint32 size)
        {
            TestBlob(blob, kind);
            Assert::IsTrue(blob->Configuration != nullptr);
            Assert::IsTrue(blob->Configuration->Profiles != nullptr);
            Assert::IsTrue(blob->Configuration->Profiles->Size == size);
        }

        TEST_METHOD(TestWifiConfigRequestResponseSerializeDeserialize_CommProxy)
        {
            auto abc = ref new WifiProfileConfiguration(); { abc->Name = "abc"; abc->Path = "ade"; abc->Uninstall = false; abc->Xml = "<afg/>"; }
            auto lmn = ref new WifiProfileConfiguration(); { lmn->Name = "lmn"; lmn->Path = "lop"; lmn->Uninstall = false; lmn->Xml = "<lqr/>"; }
            auto rem = ref new WifiProfileConfiguration(); { rem->Name = "remove"; rem->Path = "wzz"; rem->Uninstall = true; rem->Xml = "<zzz/>"; }

            // validate GetWifiConfigurationRequest serial/deserial round trip
            {
                auto wifiConfigRequest = ref new GetWifiConfigurationRequest();
                auto blob = wifiConfigRequest->Serialize();
                auto wifiConfigRequestRehydrated = 
                    dynamic_cast<GetWifiConfigurationRequest^>(GetWifiConfigurationRequest::Deserialize(blob));

                TestBlob(wifiConfigRequestRehydrated, DMMessageKind::GetWifiConfiguration);
            }

            // validate SetWifiConfigurationRequest serial/deserial round trip with Apply='no'
            {
                auto config = ref new WifiConfiguration();
                config->Profiles->Append(abc);
                config->ApplyFromDeviceTwin = L"no";

                auto wifiConfigRequest = ref new SetWifiConfigurationRequest(config);
                auto blob = wifiConfigRequest->Serialize();
                auto wifiConfigRequestRehydrated = 
                    dynamic_cast<SetWifiConfigurationRequest^>(SetWifiConfigurationRequest::Deserialize(blob));

                TestBlobAndConfig(wifiConfigRequestRehydrated, DMMessageKind::SetWifiConfiguration, 0);
                Assert::AreEqual(wifiConfigRequestRehydrated->Configuration->ApplyFromDeviceTwin, "no");
            }

            // validate SetWifiConfigurationRequest serial/deserial round trip with Apply='yes'
            {
                auto config = ref new WifiConfiguration();
                config->Profiles->Append(abc);
                config->Profiles->Append(lmn);
                config->Profiles->Append(rem);
                config->ApplyFromDeviceTwin = L"yes";

                auto wifiConfigRequest = ref new SetWifiConfigurationRequest(config);
                auto blob = wifiConfigRequest->Serialize();
                auto wifiConfigRequestRehydrated =
                    dynamic_cast<SetWifiConfigurationRequest^>(SetWifiConfigurationRequest::Deserialize(blob));

                TestBlobAndConfig(wifiConfigRequestRehydrated, DMMessageKind::SetWifiConfiguration, 3);
                Assert::AreEqual(wifiConfigRequestRehydrated->Configuration->ApplyFromDeviceTwin, "yes");

                auto profiles = wifiConfigRequestRehydrated->Configuration->Profiles;
                std::for_each(begin(profiles), end(profiles), [abc, lmn, rem](WifiProfileConfiguration^ profile) {
                    WifiProfileConfiguration^ compare = nullptr;
                    if (profile->Name == "abc") compare = abc;
                    else if (profile->Name == "lmn") compare = lmn;
                    else if (profile->Name == "remove") compare = rem;
                    else Assert::Fail(L"unknown wifi profile name");

                    Assert::AreEqual(profile->Name, compare->Name);
                    Assert::AreEqual(profile->Uninstall, compare->Uninstall);
                    if (profile->Uninstall)
                    {
                        Assert::IsTrue(profile->Path == nullptr);
                        Assert::IsTrue(profile->Xml == nullptr);
                    }
                    else
                    {
                        Assert::AreEqual(profile->Path, compare->Path);
                        Assert::AreEqual(profile->Xml, compare->Xml);
                    }
                });
            }

            // validate GetWifiConfigurationResponse serial/deserial round trip w/ Report='no'
            {
                //
                // When Report=no, no profiles are serialized
                //
                auto config = ref new WifiConfiguration();
                config->Profiles->Append(abc);
                config->ReportToDeviceTwin = L"no";

                auto wifiConfigResponse = ref new GetWifiConfigurationResponse(ResponseStatus::Success, config);
                auto blob = wifiConfigResponse->Serialize();
                auto wifiConfigResponseRehydrated = 
                    dynamic_cast<GetWifiConfigurationResponse^>(GetWifiConfigurationResponse::Deserialize(blob));

                TestBlobAndConfig(wifiConfigResponseRehydrated, DMMessageKind::GetWifiConfiguration, 0);
                Assert::AreEqual(wifiConfigResponseRehydrated->Configuration->ReportToDeviceTwin, "no");
            }

            // validate GetWifiConfigurationResponse serial/deserial round trip
            {
                //
                // GetWifiConfiguration only returns name, reflecting which profiles are installed ... none of 
                // the other details are serialized.
                //
                auto config = ref new WifiConfiguration();
                config->Profiles->Append(abc);
                config->ReportToDeviceTwin = L"yes";

                auto wifiConfigResponse = ref new GetWifiConfigurationResponse(ResponseStatus::Success, config);
                auto blob = wifiConfigResponse->Serialize();
                auto wifiConfigResponseRehydrated =
                    dynamic_cast<GetWifiConfigurationResponse^>(GetWifiConfigurationResponse::Deserialize(blob));

                TestBlobAndConfig(wifiConfigResponseRehydrated, DMMessageKind::GetWifiConfiguration, 1);
                auto foo = wifiConfigResponseRehydrated->Configuration->Profiles->First();
                Assert::AreEqual(foo->Current->Name, abc->Name);
                Assert::AreEqual(foo->Current->Uninstall, false);
                Assert::IsTrue(foo->Current->Path == nullptr);
                Assert::IsTrue(foo->Current->Xml == nullptr);
            }
        }

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
            std::wstring pipeName;
            pipeName += PipeName;
            pipeName += L"-test";

            Utils::AutoCloseHandle pipeHandleWrite = CreateNamedPipeW(
                pipeName.c_str(),
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                PipeBufferSize,
                PipeBufferSize,
                NMPWAIT_USE_DEFAULT_WAIT,
                nullptr);

            Utils::AutoCloseHandle pipeHandleRead = CreateFileW(
                pipeName.c_str(),
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
