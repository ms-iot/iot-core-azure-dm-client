#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <codecvt>

const int PipeBufferSize = 4096;
const int DataSizeInBytes = 128;
#define PIPE_NAME L"\\\\.\\pipe\\dm-client-pipe"
const wchar_t* PipeName = PIPE_NAME;
