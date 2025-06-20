#pragma once

namespace RDE{
    // This is a tag component to mark an entity as the primary camera in the scene.
    // It is used to identify which camera should be used for rendering.
    struct PrimaryCameraTag {
        // This struct is intentionally empty; it serves as a marker.
        // The presence of this component on an entity indicates that it is the primary camera.
    };
}