#pragma once
#include <string>
#include <vector>
#include <mutex>

class UpdateEngine
{
    struct PackageData
    {
        std::wstring id;
        std::wstring type;
        std::wstring version;
    };

public:
    void Scan();
    bool IsInstalled(const std::wstring& packageId, const std::wstring& packageVersion) const;
    static void Stage(const std::wstring& cabFullFileName);
    static void CommitStaged();

private:

    // Data members
    mutable std::mutex _mutex;

    std::vector<PackageData> _packageList;
};
