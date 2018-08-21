//
// Created by Peter Eichinger on 20.08.18.
//

#include "ShaderCompiler.h"
#include "FileHelpers.h"
#include <iostream>

bool ShaderCompiler::m_glslangInitialized;

std::vector<char> ShaderCompiler::loadShader(const std::string &fileName) {
    auto fileContentsVector = readFile(fileName);
    std::string contents(fileContentsVector.begin(), fileContentsVector.end());
    auto contentsC = contents.c_str();
    auto stage = determineShaderStage(fileName);

    glslang::TShader shader(stage);

    shader.setStrings(&contentsC, 1);

    return std::vector<char>();
}

EShLanguage ShaderCompiler::determineShaderStage(const std::string &fileName) {
    auto lastDot = fileName.rfind('.');

    if (lastDot == std::string::npos) {
        throw std::runtime_error("invalid filename " + fileName);
    }

    auto extension = fileName.substr(lastDot);

    if (extension == ".vert") return EShLangVertex;
    if (extension == ".frag") return EShLangFragment;
    if (extension == ".geom") return EShLangGeometry;
    if (extension == ".comp") return EShLangCompute;
    if (extension == ".tesc") return EShLangTessControl;
    if (extension == ".tese") return EShLangTessEvaluation;

    throw std::runtime_error("Invalid extension for shader " + fileName);
}
