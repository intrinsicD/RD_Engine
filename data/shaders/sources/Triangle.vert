#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

layout(push_constant) uniform PC {
    mat4 u_model; // model matrix only; no descriptor sets needed
} pc;

layout(location = 0) out vec3 v_Color;

void main()
{
    v_Color = a_Color;
    gl_Position = pc.u_model * vec4(a_Position, 1.0);
}