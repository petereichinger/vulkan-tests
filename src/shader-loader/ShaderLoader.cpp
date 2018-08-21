//
// Created by Peter Eichinger on 20.08.18.
//

#include "ShaderLoader.h"
#include "FileHelpers.h"
#include <iostream>
#include <sstream>
bool ShaderLoader::m_glslangInitialized;

const TBuiltInResource DefaultTBuiltInResource = {
        /* .MaxLights = */ 32,
        /* .MaxClipPlanes = */ 6,
        /* .MaxTextureUnits = */ 32,
        /* .MaxTextureCoords = */ 32,
        /* .MaxVertexAttribs = */ 64,
        /* .MaxVertexUniformComponents = */ 4096,
        /* .MaxVaryingFloats = */ 64,
        /* .MaxVertexTextureImageUnits = */ 32,
        /* .MaxCombinedTextureImageUnits = */ 80,
        /* .MaxTextureImageUnits = */ 32,
        /* .MaxFragmentUniformComponents = */ 4096,
        /* .MaxDrawBuffers = */ 32,
        /* .MaxVertexUniformVectors = */ 128,
        /* .MaxVaryingVectors = */ 8,
        /* .MaxFragmentUniformVectors = */ 16,
        /* .MaxVertexOutputVectors = */ 16,
        /* .MaxFragmentInputVectors = */ 15,
        /* .MinProgramTexelOffset = */ -8,
        /* .MaxProgramTexelOffset = */ 7,
        /* .MaxClipDistances = */ 8,
        /* .MaxComputeWorkGroupCountX = */ 65535,
        /* .MaxComputeWorkGroupCountY = */ 65535,
        /* .MaxComputeWorkGroupCountZ = */ 65535,
        /* .MaxComputeWorkGroupSizeX = */ 1024,
        /* .MaxComputeWorkGroupSizeY = */ 1024,
        /* .MaxComputeWorkGroupSizeZ = */ 64,
        /* .MaxComputeUniformComponents = */ 1024,
        /* .MaxComputeTextureImageUnits = */ 16,
        /* .MaxComputeImageUniforms = */ 8,
        /* .MaxComputeAtomicCounters = */ 8,
        /* .MaxComputeAtomicCounterBuffers = */ 1,
        /* .MaxVaryingComponents = */ 60,
        /* .MaxVertexOutputComponents = */ 64,
        /* .MaxGeometryInputComponents = */ 64,
        /* .MaxGeometryOutputComponents = */ 128,
        /* .MaxFragmentInputComponents = */ 128,
        /* .MaxImageUnits = */ 8,
        /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /* .MaxCombinedShaderOutputResources = */ 8,
        /* .MaxImageSamples = */ 0,
        /* .MaxVertexImageUniforms = */ 0,
        /* .MaxTessControlImageUniforms = */ 0,
        /* .MaxTessEvaluationImageUniforms = */ 0,
        /* .MaxGeometryImageUniforms = */ 0,
        /* .MaxFragmentImageUniforms = */ 8,
        /* .MaxCombinedImageUniforms = */ 8,
        /* .MaxGeometryTextureImageUnits = */ 16,
        /* .MaxGeometryOutputVertices = */ 256,
        /* .MaxGeometryTotalOutputComponents = */ 1024,
        /* .MaxGeometryUniformComponents = */ 1024,
        /* .MaxGeometryVaryingComponents = */ 64,
        /* .MaxTessControlInputComponents = */ 128,
        /* .MaxTessControlOutputComponents = */ 128,
        /* .MaxTessControlTextureImageUnits = */ 16,
        /* .MaxTessControlUniformComponents = */ 1024,
        /* .MaxTessControlTotalOutputComponents = */ 4096,
        /* .MaxTessEvaluationInputComponents = */ 128,
        /* .MaxTessEvaluationOutputComponents = */ 128,
        /* .MaxTessEvaluationTextureImageUnits = */ 16,
        /* .MaxTessEvaluationUniformComponents = */ 1024,
        /* .MaxTessPatchComponents = */ 120,
        /* .MaxPatchVertices = */ 32,
        /* .MaxTessGenLevel = */ 64,
        /* .MaxViewports = */ 16,
        /* .MaxVertexAtomicCounters = */ 0,
        /* .MaxTessControlAtomicCounters = */ 0,
        /* .MaxTessEvaluationAtomicCounters = */ 0,
        /* .MaxGeometryAtomicCounters = */ 0,
        /* .MaxFragmentAtomicCounters = */ 8,
        /* .MaxCombinedAtomicCounters = */ 8,
        /* .MaxAtomicCounterBindings = */ 1,
        /* .MaxVertexAtomicCounterBuffers = */ 0,
        /* .MaxTessControlAtomicCounterBuffers = */ 0,
        /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
        /* .MaxGeometryAtomicCounterBuffers = */ 0,
        /* .MaxFragmentAtomicCounterBuffers = */ 1,
        /* .MaxCombinedAtomicCounterBuffers = */ 1,
        /* .MaxAtomicCounterBufferSize = */ 16384,
        /* .MaxTransformFeedbackBuffers = */ 4,
        /* .MaxTransformFeedbackInterleavedComponents = */ 64,
        /* .MaxCullDistances = */ 8,
        /* .MaxCombinedClipAndCullDistances = */ 8,
        /* .MaxSamples = */ 4,
        /* .limits = */ {
                                   /* .nonInductiveForLoops = */ 1,
                                   /* .whileLoops = */ 1,
                                   /* .doWhileLoops = */ 1,
                                   /* .generalUniformIndexing = */ 1,
                                   /* .generalAttributeMatrixVectorIndexing = */ 1,
                                   /* .generalVaryingIndexing = */ 1,
                                   /* .generalSamplerIndexing = */ 1,
                                   /* .generalVariableIndexing = */ 1,
                                   /* .generalConstantMatrixVectorIndexing = */ 1,
                           }
};

std::vector<unsigned int> ShaderLoader::loadShader(const std::string &fileName) {
    auto fileContentsVector = readFile(fileName);
    std::string contents(fileContentsVector.begin(), fileContentsVector.end());
    auto contentsC = contents.c_str();
    auto stage = determineShaderStage(fileName);

    glslang::TShader shader(stage);

    shader.setStrings(&contentsC, 1);

    TBuiltInResource Resources;
    Resources = DefaultTBuiltInResource;
    // Enable SPIR-V and Vulkan rules when parsing GLSL
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    if (!shader.parse(&Resources, 100, false, messages))
    {
        std::stringstream ss;
        ss << "Error in " << fileName << std::endl;
        ss << shader.getInfoLog() << std::endl;
        ss << shader.getInfoDebugLog()<< std::endl;
        throw std::runtime_error(ss.str());
    }

    glslang::TProgram Program;
    Program.addShader(&shader);

    if(!Program.link(messages))
    {
        std::stringstream ss;
        ss << "GLSL Linking Failed for: " << fileName << std::endl;
        ss << shader.getInfoLog() << std::endl;
        ss << shader.getInfoDebugLog() << std::endl;
        throw std::runtime_error(ss.str());
    }


    std::vector<unsigned int> SpirV;
    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;
    glslang::GlslangToSpv(*Program.getIntermediate(stage), SpirV, &logger, &spvOptions);

    return SpirV;
}

EShLanguage ShaderLoader::determineShaderStage(const std::string &fileName) {
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
