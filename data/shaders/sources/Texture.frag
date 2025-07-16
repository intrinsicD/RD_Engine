#version 450 core

// The final color output for this fragment
layout(location = 0) out vec4 o_Color;

// Data received from the vertex shader (interpolated across the triangle)
in vec4 v_Color;
in vec2 v_TexCoord;
flat in float v_TexIndex; // Received without interpolation
in float v_TilingFactor;

// This is an array of texture samplers. We can access up to 32 textures.
uniform sampler2D u_Textures[32];

void main()
{
    // Determine which texture to sample from.
    // We must cast the incoming float index to an integer to use it as an array index.
    int index = int(v_TexIndex);

    // Sample the color from the correct texture in the array, using the interpolated
    // texture coordinates, multiplied by the tiling factor.
    vec4 sampledColor = texture(u_Textures[index], v_TexCoord * v_TilingFactor);
    vec4 texColor = vec4(sampledColor.r, sampledColor.r, sampledColor.r, 1.0);

    // The final pixel color is the sampled texture color multiplied (tinted) by the
    // interpolated vertex color. This allows us to have both a texture and a tint.
    o_Color = v_Color * texColor;
}