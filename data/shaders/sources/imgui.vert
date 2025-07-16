#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragColor;

layout(push_constant) uniform PushConstants {
    vec2 scale;
    vec2 translate;
} pc;

void main() {
    fragUV = inUV;
    fragColor = inColor;
    vec2 pos = inPosition * pc.scale + pc.translate;
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}