#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 1, binding = 0) uniform UniformBufferObject2 {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo2;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo2.proj * ubo2.view * ubo2.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}