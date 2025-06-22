#version 450 core

// These 'location' values must match the layout order in our C++ BufferLayout
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;

// This uniform is set once per scene by Renderer2D::BeginScene
uniform mat4 u_ViewProjection;

// These are per-vertex outputs that will be interpolated
// and sent to the fragment shader.
out vec4 v_Color;
out vec2 v_TexCoord;
// 'flat' means do not interpolate this value. The value from the first
// vertex of a triangle is used for the whole triangle. This is correct
// for an index that should be the same for all vertices of a quad.
flat out float v_TexIndex;
out float v_TilingFactor;

void main()
{
    // Pass attributes through to the fragment shader
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TexIndex = a_TexIndex;
    v_TilingFactor = a_TilingFactor;

    // Calculate the final clip-space position of the vertex
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}