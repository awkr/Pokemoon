#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_position;
layout (location = 0) out vec3 v_position;

void main() {
    gl_Position = vec4(in_position, 1.0);
    v_position = in_position;
}
