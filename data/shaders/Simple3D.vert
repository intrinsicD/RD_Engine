#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

// Outputs to Fragment Shader
out vec3 v_WorldPosition;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main()
{
    v_WorldPosition = (u_Model * vec4(a_Position, 1.0)).xyz;
    // Normals must be transformed correctly for non-uniform scaling
    v_Normal = normalize(transpose(inverse(mat3(u_Model))) * a_Normal);
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * vec4(v_WorldPosition, 1.0);
}