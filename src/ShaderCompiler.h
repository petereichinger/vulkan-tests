//
// Created by Peter Eichinger on 20.08.18.
//

#pragma once
#include <vector>
#include <string>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

class ShaderCompiler {

    EShLanguage determineShaderStage(const std::string& fileName);
public:
    ShaderCompiler();

    std::vector<char> loadShader(const std::string& fileName);
};
