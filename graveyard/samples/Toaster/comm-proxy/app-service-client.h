#pragma once
#include <stdint.h>

void send_data_to_UWP_app(wchar_t* data, uint32_t length, Platform::String^ service_name, Platform::String^ pkg_family_name);

