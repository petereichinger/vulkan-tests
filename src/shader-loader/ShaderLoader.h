//
// Created by Peter Eichinger on 20.08.18.
//

#pragma once
#include <vector>
#include <string>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <iostream>
#include <tuple>

class ShaderLoader {
private:
    static bool m_glslangInitialized;
    EShLanguage determineShaderStage(const std::string& fileName);
public:
    ShaderLoader() {
        if (!m_glslangInitialized) {
            glslang::InitializeProcess();
            m_glslangInitialized = true;
        }
    }

    std::tuple<bool, std::vector<unsigned int>> loadShader(const std::string &fileName);
};
