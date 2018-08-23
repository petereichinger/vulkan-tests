//
// Created by Peter Eichinger on 22.08.18.
//

#include "ShaderUpdater.h"
#include <tuple>

bool ShaderUpdater::registerShader(const std::string &fileName) {
    m_watcher.registerFile(fileName);
    auto find = m_shaders.find(fileName);

    if (find != m_shaders.end()) {
        return false;
    }

    bool success;
    std::vector<uint32_t> code;
    std::tie(success, code) = m_loader.loadShader(fileName);
    m_shaders[fileName] = code;
    return success;
}

void ShaderUpdater::unregisterShader(const std::string &fileName) {
    m_watcher.unregisterFile(fileName);
    auto find = m_shaders.find(fileName);

    if (find == m_shaders.end()) {
        return;
    }

    m_shaders.erase(fileName);
}

bool ShaderUpdater::checkForModifications() {
    bool modified = false;
    m_watcher.checkForModifications();

    for(auto& shader : m_shaders) {
        if (m_watcher.wasModified(shader.first)) {
            bool success;
            std::vector<uint32_t> code;
            std::tie(success,code) = m_loader.loadShader(shader.first);
            if (success) {
                shader.second = code;
                modified = true;
            }
        }
    }
    return modified;
}

const std::vector<uint32_t> &ShaderUpdater::getShaderCode(const std::string &fileName) {

    auto find = m_shaders.find(fileName);

    if (find == m_shaders.end()) {
        std::cerr << "Unregistered shader: " << fileName << std::endl;
    }

    return find->second;
}
