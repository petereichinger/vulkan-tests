//
// Created by Peter Eichinger on 20.08.18.
//

#pragma once
#include <vector>
#include <string>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <iostream>

class ShaderCompiler {
private:
    static bool m_glslangInitialized;
    EShLanguage determineShaderStage(const std::string& fileName);
public:
    ShaderCompiler() {
        if (!m_glslangInitialized) {
            glslang::InitializeProcess();
            m_glslangInitialized = true;
            std::cout << "INIT" << std::endl;
        }
    }

    std::vector<char> loadShader(const std::string& fileName);
};
