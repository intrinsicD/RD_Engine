#version 450 core
layout(location = 0) in vec3 a_Position;

layout (std140, binding = 0) uniform CameraUBO {
    mat4 u_view;
    mat4 u_projection;
};

uniform mat4 u_model;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(a_Position, 1.0);
}