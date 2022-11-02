#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform GlobalUniformObject {
    mat4 view;
    mat4 proj;
} u_global_ubo;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;
layout (location = 0) out vec3 v_color;

void main() {
    // gl_Position = vec4(in_position, 1.0);
    gl_Position = u_global_ubo.proj * u_global_ubo.view * vec4(in_position, 1.0);
    v_color = in_color;
}
