#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragPos;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 1, binding = 1) uniform sampler2D albedoTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D normalTexture;
layout(set = 1, binding = 4) uniform sampler2D occlusionTexture;
layout(set = 1, binding = 5) uniform sampler2D emissiveTexture;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 albedoColor;
} pushConstants;

layout(location = 0) out vec4 out_color;

void main() {
    vec4 texColor = texture(albedoTexture, fragUV);
    out_color = texColor * pushConstants.albedoColor;
}
