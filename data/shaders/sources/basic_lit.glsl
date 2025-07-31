// =================================================================================================
// Basic Lit PBR Shader
// Implements a standard Cook-Torrance BRDF with a metallic/roughness workflow.
// =================================================================================================


// =====================================================
// VERTEX SHADER
// =====================================================
#ifdef VERTEX_SHADER

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

#endif


// =====================================================
// FRAGMENT SHADER
// =====================================================
#ifdef FRAGMENT_SHADER

#version 450 core

// -- Inputs from Vertex Shader --
layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inWorldNormal;
layout (location = 2) in vec2 inTexCoords;
#ifdef HAS_NORMAL_MAP
layout (location = 3) in mat3 inTBN; // Receive the TBN matrix
#endif

// -- UBO: Per-Frame Data (same as vertex shader) --
layout (set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

// -- UBO: Per-Material Data --
// Matches the 'parameters' block in our .mat file.
layout (set = 1, binding = 0) uniform MaterialData {
    vec4 baseColorFactor;
    float metalnessFactor;
    float roughnessFactor;
} material;

// -- Textures --
// These bindings must match the ones your renderer sets up for the material's descriptor set.
layout (set = 1, binding = 1) uniform sampler2D albedoMap;
layout (set = 1, binding = 2) uniform sampler2D normalMap;
layout (set = 1, binding = 3) uniform sampler2D metalRoughnessMap;

// -- Final Output --
layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// --- PBR Helper Functions (Cook-Torrance BRDF) ---

// Normal Distribution Function (Trowbridge-Reitz GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

// Geometry Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

// Smith's Method for combining geometry functions
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel-Schlick Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


void main() {
    // --- 1. Get Material Properties from Textures and UBOs ---
    #ifdef HAS_ALBEDO_MAP
    vec3 albedo = pow(texture(albedoMap, inTexCoords).rgb, vec3(2.2));
    #else
    vec3 albedo = vec3(1.0);// Default to white if no map
    #endif
    albedo *= material.baseColorFactor.rgb;

    // PBR map: R=AO, G=Roughness, B=Metalness
    float metalness;
    float roughness;

    #ifdef HAS_METALROUGHNESS_MAP
    // If a map is provided, sample from it.
    // The GLTF2 standard uses: R=unused, G=Roughness, B=Metalness.
    vec4 pbrSample = texture(metalRoughnessMap, inTexCoords);
    roughness = pbrSample.g;
    metalness = pbrSample.b;
    #else
    // If no map is provided, use the uniform factors as the definitive values.
    // We can just assign them directly.
    roughness = material.roughnessFactor;
    metalness = material.metalnessFactor;
    #endif

    // --- 2. Setup Geometric Vectors ---
    vec3 N;
    #ifdef HAS_NORMAL_MAP
    // Unpack normal from map and transform to world space (TBN matrix)
    vec3 tangentNormal = texture(normalMap, inTexCoords).xyz * 2.0 - 1.0;
    // ... create TBN matrix and transform tangentNormal ...
     N = normalize(inTBN * tangentNormal);
    #else
    N = normalize(inWorldNormal);// Use vertex normal if no map
    #endif
    vec3 V = normalize(ubo.camPos - inWorldPos);

    // For now, a single hardcoded directional light for testing.
    vec3 lightDir = normalize(vec3(0.5, 0.5, -1.0));
    vec3 L = -lightDir;
    vec3 lightColor = vec3(5.0, 5.0, 5.0);// A bright white light

    // Base reflectivity. For dielectrics, it's ~4% grey. For metals, it's the albedo color.
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metalness);

    // --- 3. PBR Lighting Calculation ---
    vec3 Lo = vec3(0.0);// Outgoing radiance

    // Cook-Torrance BRDF
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);

    if (NdotL > 0.0) {
        // Calculate the terms of the BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        // Specular BRDF
        vec3 kS = F;
        vec3 specular = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001);

        // Diffuse BRDF (energy-conserving)
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalness;// Pure metals have no diffuse reflection

        // Add to outgoing radiance
        Lo += (kD * albedo / PI + specular) * lightColor * NdotL;
    }

    // --- 4. Final Color ---
    // A simple ambient term to light the dark side of objects
    vec3 ambient = vec3(0.03) * albedo;
    vec3 color = ambient + Lo;

    // HDR to LDR mapping (simple Reinhard tone mapping) and Gamma Correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);
}

#endif