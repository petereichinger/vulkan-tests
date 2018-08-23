//
// Created by Peter Eichinger on 22.08.18.
//

#pragma once
#include <map>
#include <string>
#include <tuple>

class FileWatcher {

private:
    std::map<std::string, std::pair<long, bool>> m_fileWriteTimes;

public:
    void registerFile(const std::string& fileName);
    void unregisterFile(const std::string& fileName);
    void checkForModifications();

    bool wasModified(const std::string& fileName);
};


