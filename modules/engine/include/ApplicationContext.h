#pragma once

namespace RDE {
    class IWindow;
    class IRenderer;
    class JobSystem;
    class AssetManager;
    class InputManager;
    class LayerStack;
    class IRenderPipeline;
    class RenderPipelineManager;

    struct ApplicationContext {
        IWindow *window = nullptr;
        IRenderer *renderer = nullptr;
        JobSystem *job_system = nullptr;
        AssetManager *asset_manager = nullptr;
        InputManager *input_manager = nullptr;
        RenderPipelineManager *render_pipeline_manager = nullptr; // Optional, for advanced rendering setups
        LayerStack *layer_stack = nullptr;
    };
}
