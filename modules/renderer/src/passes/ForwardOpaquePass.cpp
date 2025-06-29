#include "graph/RenderGraph.h"
#include "graph/RenderPassBuilder.h"
#include "RenderPacket.h"
#include "../../include/ral/CommandBuffer.h"

namespace RDE {

    void setup_forward_opaque_pass(RenderGraph &rg, const RenderPacket &packet, RGResourceHandle render_target,
                                   RGResourceHandle depth_target) {
        rg.add_pass("Forward Opaque",
                    [&](RGBuilder &builder) {
                        // This pass writes to the final color and depth buffers.
                        builder.write(render_target);
                        builder.write(depth_target);
                    },
                    [=](ICommandBuffer &cmd, const RenderPacket &pkt) {
                        // In a real pass, you'd set up camera UBOs, light data, etc. here.

                        for (const auto &render_object: pkt.opaque_objects) {
                            // Here you would get the GpuPipelineHandle from the material
                            // and bind it. For now, we assume a single pipeline.
                            // cmd.bind_pipeline(material.pipeline_handle);
                            // cmd.push_constants(&render_object.transform, ...);
                            cmd.draw_indexed(render_object.mesh_id.index_count);
                        }
                    }
        );
    }
}