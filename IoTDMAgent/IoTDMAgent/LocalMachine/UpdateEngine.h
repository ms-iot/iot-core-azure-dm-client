#pragma once
#include <string>
#include <vector>

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
    bool IsInstalled(const std::wstring& id, const std::wstring& version) const;
    static void Stage(const std::wstring& cabFullFileName);
    static void CommitStaged();

private:
    std::vector<PackageData> _packageList;
};
