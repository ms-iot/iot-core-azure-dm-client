#pragma once
#include <string>
#include <chrono>

class Limpet
{
public:
    static Limpet& GetInstance();

    std::string GetSASToken(UINT deviceNumber, const std::chrono::system_clock::time_point& expiration);

private:
    Limpet();
    ~Limpet();

    void ThrowException(const char* message, UINT32 errorCode);
};