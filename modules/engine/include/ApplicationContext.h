#pragma once

namespace RDE {
    class IWindow;
    class IRenderer;
    class JobSystem;
    class AssetManager;
    class InputManager;
    class LayerStack;

    struct ApplicationContext {
        IWindow *window = nullptr;
        IRenderer *renderer = nullptr;
        JobSystem *job_system = nullptr;
        AssetManager *asset_manager = nullptr;
        InputManager *input_manager = nullptr;
        LayerStack *layer_stack = nullptr;
    };
}
