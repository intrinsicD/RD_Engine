#include "graph/RenderGraph.h"
#include "graph/RenderPassBuilder.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h" // For the rendering function
#include <GLFW/glfw3.h>

namespace RDE {
    // This function adds a pass to the graph that will render ImGui data.
    void setup_imgui_pass(RenderGraph &rg, RGResourceHandle final_render_target) {
        ImDrawData* draw_data = ImGui::GetDrawData(); // <-- Query Just-in-Time
        if (!draw_data || draw_data->CmdListsCount == 0) {
            return; // No UI to draw, don't add the pass.
        }

        rg.add_pass("ImGui Pass",
                // 1. SETUP: Declare our dependencies.
                    [&](RGBuilder &builder) {
                        // This pass reads nothing (it generates its own geometry).
                        // It WRITES to whatever the final output of our 3D scene was.
                        // This creates the dependency: "3D Scene -> UI".
                        builder.write(final_render_target);
                    },
                // 2. EXECUTE: The actual drawing commands.
                    [=](ICommandBuffer &cmd, const RenderPacket &packet) {
                        // Here's the key: instead of calling the raw ImGui_ImplOpenGL3_RenderDrawData,
                        // we would ideally have a version that works with our ICommandBuffer abstraction.
                        // But since ImGui's backend is so specific, it's one of the few places
                        // where a small break in abstraction is pragmatic.

                        // The RenderGraph has already bound `final_render_target` for us.
                        // All we have to do is call the drawing function.
                        ImGui_ImplOpenGL3_RenderDrawData(draw_data);

                        ImGuiIO &io = ImGui::GetIO();
                        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                            GLFWwindow *backup_current_context = glfwGetCurrentContext();
                            ImGui::UpdatePlatformWindows();
                            ImGui::RenderPlatformWindowsDefault();
                            glfwMakeContextCurrent(backup_current_context);
                        }
                    }
        );
    }
}