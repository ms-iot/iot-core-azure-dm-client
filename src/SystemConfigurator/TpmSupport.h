#pragma once

#include <string>

std::string GetServiceUrl(int logicalId);
std::string GetSASToken(int logicalId);
void ClearTPM();

