#version 450 core

layout(location = 0) out vec4 FragColor;

in vec3 v_WorldPosition;
in vec3 v_Normal;
in vec2 v_TexCoord;

// Uniforms provided by the renderer
uniform sampler2D u_TextureSampler;
uniform vec4 u_Color;
uniform bool u_UseTexture;

struct Light
{
    vec3 Direction;
    vec3 Color;
};
uniform Light u_Light;

void main()
{
    vec3 normal = normalize(v_Normal);
    float diffuse_intensity = max(dot(normal, -normalize(u_Light.Direction)), 0.0);
    vec3 diffuse_color = u_Light.Color * diffuse_intensity;

    // Determine base color from texture or uniform
    vec4 base_color = u_UseTexture ? texture(u_TextureSampler, v_TexCoord) : u_Color;

    // Final color is a mix of ambient, diffuse, and base color
    // We'll use a simple ambient term to prevent shadows from being pure black.
    vec3 ambient_color = vec3(0.1);
    vec3 final_color = (ambient_color + diffuse_color) * base_color.rgb;

    FragColor = vec4(final_color, base_color.a);
}