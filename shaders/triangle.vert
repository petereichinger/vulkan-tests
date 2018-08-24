#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 spherize;
} ubo;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;


layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 normalPos = ubo.model * inPosition;
    vec4 spherePos = ubo.model * vec4(1.1 * normalize(inPosition.xyz), 1.0f);
    gl_Position = ubo.proj * ubo.view * mix(normalPos, spherePos, ubo.spherize.x);
    fragColor = vec3(inColor);
    fragTexCoord = inTexCoord;
}