#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform CameraUBO {
    mat4 u_view;
    mat4 u_projection;
};

uniform mat4 u_model;

// Outputs to Fragment Shader
out vec3 v_WorldPosition;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main()
{
    v_WorldPosition = (u_model * vec4(a_Position, 1.0)).xyz;
    // Normals must be transformed correctly for non-uniform scaling
    v_Normal = normalize(transpose(inverse(mat3(u_Model))) * a_Normal);
    v_TexCoord = a_TexCoord;
    gl_Position = u_projection * u_view * vec4(v_WorldPosition, 1.0);
}