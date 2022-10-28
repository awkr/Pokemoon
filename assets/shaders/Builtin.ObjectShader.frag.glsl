#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 v_position;
layout (location = 0) out vec4 out_color;

void main() {
    // out_color = vec4(1.0);
    out_color = vec4(v_position.r, v_position.g, v_position.b, 1.0);
}
