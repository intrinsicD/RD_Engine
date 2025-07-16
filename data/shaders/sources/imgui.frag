#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D sTexture;

void main() {
    outColor = fragColor * texture(sTexture, fragUV);
}