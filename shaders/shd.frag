#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define PI 3.14159265359

// Входные данные из вершинного шейдера
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragPos;

// Uniform-буферы
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} ubo;

// PBR-текстуры
layout(set = 1, binding = 1) uniform sampler2D albedoTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D normalTexture;
layout(set = 1, binding = 4) uniform sampler2D occlusionTexture;
layout(set = 1, binding = 5) uniform sampler2D emissiveTexture;

// Выходной цвет
layout(location = 0) out vec4 out_color;

// PBR-функции
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalTexture, fragUV).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(fragPos);
    vec3 Q2 = dFdy(fragPos);
    vec2 st1 = dFdx(fragUV);
    vec2 st2 = dFdy(fragUV);

    vec3 N = normalize(fragNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(Q1 * st2.s - Q2 * st1.s);
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// Функции PBR
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 albedo = pow(texture(albedoTexture, fragUV).rgb, vec3(2.2));

    // Если текстура заглушка, используем стандартные значения
    vec3 normal = (textureSize(normalTexture, 0).x == 1 && textureSize(normalTexture, 0).y == 1) ? normalize(fragNormal) : getNormalFromMap();
    float metallic = (textureSize(metallicRoughnessTexture, 0).x == 1 && textureSize(metallicRoughnessTexture, 0).y == 1) ? 0.0 : texture(metallicRoughnessTexture, fragUV).r;
    float roughness = (textureSize(metallicRoughnessTexture, 0).x == 1 && textureSize(metallicRoughnessTexture, 0).y == 1) ? 1.0 : texture(metallicRoughnessTexture, fragUV).g;
    float ao = (textureSize(occlusionTexture, 0).x == 1 && textureSize(occlusionTexture, 0).y == 1) ? 1.0 : texture(occlusionTexture, fragUV).r;
    vec3 emissive = (textureSize(emissiveTexture, 0).x == 1 && textureSize(emissiveTexture, 0).y == 1) ? vec3(0.0) : texture(emissiveTexture, fragUV).rgb;

    // Векторы для PBR-освещения
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(ubo.cameraPos - fragPos);

    // F0 - базовый Френель
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Без источников света пока
    vec3 Lo = vec3(0.0);

    // Общий цвет
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo + emissive;

    // Гамма-коррекция
    out_color = vec4(pow(color, vec3(1.0/2.2)), 1.0);
}
