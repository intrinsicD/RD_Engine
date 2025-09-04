#version 450 core

// -- Vertex Attributes (from VBOs) --
// These locations must match your RAL::VertexInputAttribute descriptions.
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
#ifdef HAS_NORMAL_MAP
layout (location = 3) in vec4 inTangent;// The tangent from the mesh
#endif

// -- UBO: Per-Frame Data --
// Contains data that is constant for an entire frame, like camera matrices.
layout (set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

// -- Push Constants: Per-Draw Data --
// For small, frequently changing data like the model matrix of an object.
layout (push_constant) uniform PushConstants {
    mat4 model;
} pc;

// -- Outputs to Fragment Shader --
layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outWorldNormal;
layout (location = 2) out vec2 outTexCoords;
#ifdef HAS_NORMAL_MAP
layout (location = 3) out mat3 outTBN;// We will pass the full TBN matrix
#endif

void main() {
    // Calculate position in world space and pass to fragment shader
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    outWorldPos = worldPos.xyz;

    // Pass texture coordinates through
    outTexCoords = inTexCoords;

    // Transform normal into world space. Use inverse transpose for non-uniform scaling.
    // This is the mathematically correct way to transform normals.
    outWorldNormal = normalize(transpose(inverse(mat3(pc.model))) * inNormal);

    #ifdef HAS_NORMAL_MAP
    // Transform the tangent to world space
    vec3 T = normalize(mat3(pc.model) * inTangent.xyz);

    // The normal N is already in world space (outWorldNormal)
    vec3 N = outWorldNormal;

    // Re-orthogonalize T with respect to N to prevent floating point inaccuracies
    // This ensures the tangent is perfectly perpendicular to the normal.
    T = normalize(T - dot(T, N) * N);

    // Calculate the bitangent B using the cross product.
    // The inTangent.w component stores the handedness, which corrects the direction.
    vec3 B = cross(N, T) * inTangent.w;

    // Create and pass the TBN matrix to the fragment shader.
    // This matrix will transform from tangent space to world space.
    outTBN = mat3(T, B, N);
    #endif

    // Final clip space position
    gl_Position = ubo.proj * ubo.view * worldPos;
}