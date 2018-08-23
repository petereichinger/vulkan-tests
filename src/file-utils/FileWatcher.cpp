//
// Created by Peter Eichinger on 22.08.18.
//

#include "FileWatcher.h"
#include "FileHelpers.h"
#include <iostream>

void FileWatcher::registerFile(const std::string &fileName) {
    auto find = m_fileWriteTimes.find(fileName);

    if (find != m_fileWriteTimes.end()) {
        std::cerr<< "Already registered '" << fileName << "'" << std::endl;
        return;
    }

    long time = getFileWriteTime(fileName);

    m_fileWriteTimes[fileName] = std::make_pair(time, false);
}

void FileWatcher::unregisterFile(const std::string &fileName) {
    auto find = m_fileWriteTimes.find(fileName);

    if (find == m_fileWriteTimes.end()) {
        std::cerr<< "Not registered '" << fileName << "'" << std::endl;
        return;
    }

    m_fileWriteTimes.erase(fileName);
}

void FileWatcher::checkForModifications() {
    for(auto& writeTime : m_fileWriteTimes) {
        long newTime = getFileWriteTime(writeTime.first);

        writeTime.second = std::make_pair(newTime, newTime > writeTime.second.first);
    }
}

bool FileWatcher::wasModified(const std::string &fileName) {
    return m_fileWriteTimes[fileName].second;
}
