#version 450

// Входы: данные, которые мы получаем из вершинного буфера
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inUV;

// push-константы для данных, которые меняются чаще всего (например, модель-матрица)
layout(push_constant) uniform PushConstants {
    mat4 model;
} pushConstants;

// Дескрипторный сет 0: данные уровня сцены (например, матрицы вида и проекции)
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

// Выходы: данные, которые передаются во фрагментный шейдер
// Эти переменные интерполируются по всей поверхности треугольника
layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragPos; // Передаем позицию фрагмента в мировом пространстве

void main() {
    // Вычисляем финальную позицию вершины в пространстве отсечения
    gl_Position = ubo.proj * ubo.view * pushConstants.model * vec4(inPosition, 1.0);

    // Преобразуем нормали из локального пространства в мировое
    fragNormal = mat3(pushConstants.model) * inNormal;

    // Передаем UV-координаты
    fragUV = inUV;

    // Вычисляем и передаем позицию фрагмента в мировом пространстве
    // (понадобится для расчетов освещения)
    fragPos = (pushConstants.model * vec4(inPosition, 1.0)).xyz;
}