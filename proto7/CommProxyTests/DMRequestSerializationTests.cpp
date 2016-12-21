#include "stdafx.h"
#include "CppUnitTest.h"

#include <Windows.h>
#include "..\SharedUtilities\Utils.h"
#include "..\SharedUtilities\Logger.h"
#include "..\SharedUtilities\DMRequest.h"
#include "..\SharedUtilities\SecurityAttributes.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CommProxyTests
{		
    void VerifyDMMessageEquality(DMMessage& one, DMMessage& two)
    {
        Assert::AreEqual(one.GetContext(), two.GetContext());
        Assert::AreEqual(one.GetDataCount(), two.GetDataCount());
        Assert::AreEqual(0, memcmp(one.GetData().data(), two.GetData().data(), one.GetDataCount()));
    }

	TEST_CLASS(DMRequestSerializationTests)
	{
    private:

        void ValidateDMMessageWriteRead(DMMessage& orig)
        {
            SecurityAttributes sa(GENERIC_WRITE | GENERIC_READ);
            Utils::AutoCloseHandle pipeHandleWrite;
            pipeHandleWrite = CreateNamedPipeW(
                PIPE_NAME L"-test",
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                PipeBufferSize,
                PipeBufferSize,
                NMPWAIT_USE_DEFAULT_WAIT,
                sa.GetSA());

            Utils::AutoCloseHandle pipeHandleRead;
            pipeHandleRead = CreateFileW(PIPE_NAME L"-test",
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            Assert::IsTrue(DMMessage::WriteToPipe(pipeHandleWrite.Get(), orig));
            DMMessage messageToReceive(DMStatus::Failed);
            Assert::IsTrue(DMMessage::ReadFromPipe(pipeHandleRead.Get(), messageToReceive));

            VerifyDMMessageEquality(orig, messageToReceive);
        }

    public:
		
        TEST_METHOD(TestEmptyUnknownMessageReadWrite)
        {
            DMMessage emptyMessage(DMCommand::Unknown);
            ValidateDMMessageWriteRead(emptyMessage);
        }

        TEST_METHOD(TestEmptyMessageReadWrite)
        {
            DMMessage message(DMCommand::CheckUpdates);
            ValidateDMMessageWriteRead(message);
        }

        TEST_METHOD(TestWstringMessageReadWrite)
        {
            std::wstring data(L"abcdefghijklmnop");
            DMMessage message(DMCommand::UninstallApp);
            message.SetData(data);

            Assert::AreEqual(data, message.GetDataW());

            ValidateDMMessageWriteRead(message);
        }

        TEST_METHOD(TestStringMessageReadWrite)
        {
            std::string data("abcdefghijklmnop");
            DMMessage message(DMCommand::UninstallApp);
            message.SetData(data.data(), data.size());

            Assert::AreEqual(data, message.GetData());

            ValidateDMMessageWriteRead(message);
        }

        TEST_METHOD(TestUint32MessageReadWrite)
        {
            uint32_t data = 0x12345678;
            auto dataAsBytes = (char*)&data;
            DMMessage message(DMCommand::UninstallApp);
            message.SetData(dataAsBytes, sizeof(uint32_t));

            auto messageDataAsBytes = message.GetData().data();
            Assert::AreEqual(0, memcmp(dataAsBytes, messageDataAsBytes, message.GetDataCount()));

            ValidateDMMessageWriteRead(message);
        }

    };
}