//
// Created by Peter Eichinger on 22.08.18.
//

#pragma once
#include <vector>
#include <map>
#include <string>
#include "file-utils/FileWatcher.h"

#include "ShaderLoader.h"

class ShaderUpdater {
private:
    ShaderLoader m_loader;
    FileWatcher m_watcher;
    std::map<std::string, std::vector<uint32_t>> m_shaders;
public:

    bool registerShader(const std::string &fileName);
    void unregisterShader(const std::string& fileName);

    bool checkForModifications();

    const std::vector<uint32_t>& getShaderCode(const std::string& fileName);
};


