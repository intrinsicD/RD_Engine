//assets/AssetConcepts.h
#pragma once

// These are empty structs that act as high-level, compile-time tags.
// They represent the "idea" of an asset, not its data storage.

namespace RDE::AssetConcepts {
    struct Texture {};
    struct Mesh {};
    struct Material {};
    struct ShaderProgram {};
    // etc.
}