#pragma once

#include "Camera.h" // We need the base class definition
#include <memory>


namespace RDE {
    struct CameraComponent {
        std::shared_ptr<Camera> camera = nullptr;
        bool primary = true; // Is this the main camera to render from?
        bool fixed_aspect_ratio = false;

        CameraComponent() = default;

        CameraComponent(const CameraComponent &) = default;
    };
}