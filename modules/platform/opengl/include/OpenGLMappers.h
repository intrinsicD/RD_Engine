//opengl/OpenGLMappers.h
#pragma once

#include "ral/Common.h"
#include "ral/Resources.h"

#include <stdexcept>
#include <glad/gl.h>

namespace RDE {
    inline GLenum ToOpenGLFormat(RAL::Format format) {
        switch (format) {
            // 8-bit UNORM
            case RAL::Format::R8_UNORM:
                return GL_R8;
            case RAL::Format::R8G8_UNORM:
                return GL_RG8;
            case RAL::Format::R8G8B8A8_UNORM:
                return GL_RGBA8;
            case RAL::Format::B8G8R8A8_UNORM:
                return GL_RGBA8; // Use swizzle if true BGRA needed

                // 8-bit sRGB (approx for single / dual channel where no direct sRGB internal format exists)
            case RAL::Format::R8_SRGB:
                return GL_R8;              // No single-channel sRGB internal format
            case RAL::Format::R8G8_SRGB:
                return GL_RG8;             // No dual-channel sRGB internal format
            case RAL::Format::R8G8B8A8_SRGB:
                return GL_SRGB8_ALPHA8;
            case RAL::Format::B8G8R8A8_SRGB:
                return GL_SRGB8_ALPHA8;    // Use swizzle if BGRA order required

                // 16-bit float
            case RAL::Format::R16_SFLOAT:
                return GL_R16F;
            case RAL::Format::R16G16_SFLOAT:
                return GL_RG16F;
            case RAL::Format::R16G16B16A16_SFLOAT:
                return GL_RGBA16F;

                // 32-bit float
            case RAL::Format::R32_SFLOAT:
                return GL_R32F;
            case RAL::Format::R32G32_SFLOAT:
                return GL_RG32F;
            case RAL::Format::R32G32B32_SFLOAT:
                return GL_RGB32F;
            case RAL::Format::R32G32B32A32_SFLOAT:
                return GL_RGBA32F;

                // 32-bit unsigned int
            case RAL::Format::R32_UINT:
                return GL_R32UI;
            case RAL::Format::R32G32_UINT:
                return GL_RG32UI;
            case RAL::Format::R32G32B32_UINT:
                return GL_RGB32UI;
            case RAL::Format::R32G32B32A32_UINT:
                return GL_RGBA32UI;

                // Depth / Depth-Stencil
            case RAL::Format::D32_SFLOAT:
                return GL_DEPTH_COMPONENT32F;
            case RAL::Format::D24_UNORM_S8_UINT:
                return GL_DEPTH24_STENCIL8;
            case RAL::Format::D32_SFLOAT_S8_UINT:
                return GL_DEPTH32F_STENCIL8;

                // Block Compression (BCn / S3TC / BPTC)
            case RAL::Format::BC1_RGB_UNORM:
                return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            case RAL::Format::BC3_UNORM:
                return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            case RAL::Format::BC7_UNORM:
                return GL_COMPRESSED_RGBA_BPTC_UNORM;

            case RAL::Format::UNKNOWN:
            default:
                throw std::runtime_error("Unsupported or unknown RAL::Format in ToOpenGLFormat");
        }
    }

    // --- PRIMITIVE TOPOLOGY ---
    inline GLenum ToOpenGLPrimitive(RAL::PrimitiveTopology t) {
        switch (t) {
            case RAL::PrimitiveTopology::PointList:      return GL_POINTS;
            case RAL::PrimitiveTopology::LineList:       return GL_LINES;
            case RAL::PrimitiveTopology::LineStrip:      return GL_LINE_STRIP;
            case RAL::PrimitiveTopology::TriangleList:   return GL_TRIANGLES;
            case RAL::PrimitiveTopology::TriangleStrip:  return GL_TRIANGLE_STRIP;
            default: throw std::runtime_error("Unknown PrimitiveTopology");
        }
    }

    // --- COMPARE OP (Depth/Stencil func) ---
    inline GLenum ToOpenGLCompare(RAL::CompareOp op) {
        switch (op) {
            case RAL::CompareOp::Never:         return GL_NEVER;
            case RAL::CompareOp::Less:          return GL_LESS;
            case RAL::CompareOp::Equal:         return GL_EQUAL;
            case RAL::CompareOp::LessOrEqual:   return GL_LEQUAL;
            case RAL::CompareOp::Greater:       return GL_GREATER;
            case RAL::CompareOp::NotEqual:      return GL_NOTEQUAL;
            case RAL::CompareOp::GreaterOrEqual:return GL_GEQUAL;
            case RAL::CompareOp::Always:        return GL_ALWAYS;
            default: throw std::runtime_error("Unknown CompareOp");
        }
    }

    // --- BLEND FACTOR ---
    inline GLenum ToOpenGLBlendFactor(RAL::BlendFactor f) {
        switch (f) {
            case RAL::BlendFactor::Zero:              return GL_ZERO;
            case RAL::BlendFactor::One:               return GL_ONE;
            case RAL::BlendFactor::SrcColor:          return GL_SRC_COLOR;
            case RAL::BlendFactor::OneMinusSrcColor:  return GL_ONE_MINUS_SRC_COLOR;
            case RAL::BlendFactor::DstColor:          return GL_DST_COLOR;
            case RAL::BlendFactor::OneMinusDstColor:  return GL_ONE_MINUS_DST_COLOR;
            case RAL::BlendFactor::SrcAlpha:          return GL_SRC_ALPHA;
            case RAL::BlendFactor::OneMinusSrcAlpha:  return GL_ONE_MINUS_SRC_ALPHA;
            case RAL::BlendFactor::DstAlpha:          return GL_DST_ALPHA;
            case RAL::BlendFactor::OneMinusDstAlpha:  return GL_ONE_MINUS_DST_ALPHA;
            default: throw std::runtime_error("Unknown BlendFactor");
        }
    }

    // --- BLEND OP ---
    inline GLenum ToOpenGLBlendOp(RAL::BlendOp op) {
        switch (op) {
            case RAL::BlendOp::Add:              return GL_FUNC_ADD;
            case RAL::BlendOp::Subtract:         return GL_FUNC_SUBTRACT;
            case RAL::BlendOp::ReverseSubtract:  return GL_FUNC_REVERSE_SUBTRACT;
            case RAL::BlendOp::Min:              return GL_MIN;
            case RAL::BlendOp::Max:              return GL_MAX;
            default: throw std::runtime_error("Unknown BlendOp");
        }
    }

    // --- RASTER STATE ---
    inline GLenum ToOpenGLCullMode(RAL::CullMode c) {
        switch (c) {
            case RAL::CullMode::None:         return 0;            // Indicates disable cull face
            case RAL::CullMode::Front:        return GL_FRONT;
            case RAL::CullMode::Back:         return GL_BACK;
            case RAL::CullMode::FrontAndBack: return GL_FRONT_AND_BACK;
            default: throw std::runtime_error("Unknown CullMode");
        }
    }

    inline GLenum ToOpenGLFrontFace(RAL::FrontFace f) {
        switch (f) {
            case RAL::FrontFace::Clockwise:        return GL_CW;
            case RAL::FrontFace::CounterClockwise: return GL_CCW;
            default: throw std::runtime_error("Unknown FrontFace");
        }
    }

    inline GLenum ToOpenGLPolygonMode(RAL::PolygonMode m) {
        switch (m) {
            case RAL::PolygonMode::Fill: return GL_FILL;
            case RAL::PolygonMode::Line: return GL_LINE;
            case RAL::PolygonMode::Point:return GL_POINT;
            default: throw std::runtime_error("Unknown PolygonMode");
        }
    }

    // --- SAMPLER / TEXTURE ---
    inline GLenum ToOpenGLFilter(RAL::Filter f) {
        switch (f) {
            case RAL::Filter::Nearest: return GL_NEAREST;
            case RAL::Filter::Linear:  return GL_LINEAR;
            default: throw std::runtime_error("Unknown Filter");
        }
    }

    inline GLenum ToOpenGLWrap(RAL::SamplerAddressMode m) {
        switch (m) {
            case RAL::SamplerAddressMode::Repeat:          return GL_REPEAT;
            case RAL::SamplerAddressMode::MirroredRepeat:  return GL_MIRRORED_REPEAT;
            case RAL::SamplerAddressMode::ClampToEdge:     return GL_CLAMP_TO_EDGE;
            case RAL::SamplerAddressMode::ClampToBorder:   return GL_CLAMP_TO_BORDER;
            default: throw std::runtime_error("Unknown SamplerAddressMode");
        }
    }

    // --- SHADER STAGE (single stage) ---
    inline GLenum ToOpenGLShaderStage(RAL::ShaderStage s) {
        switch (s) {
            case RAL::ShaderStage::Vertex:                  return GL_VERTEX_SHADER;
            case RAL::ShaderStage::Fragment:                return GL_FRAGMENT_SHADER;
            case RAL::ShaderStage::Geometry:                return GL_GEOMETRY_SHADER;
            case RAL::ShaderStage::TessellationControl:     return GL_TESS_CONTROL_SHADER;
            case RAL::ShaderStage::TessellationEvaluation:  return GL_TESS_EVALUATION_SHADER;
            case RAL::ShaderStage::Compute:                 return GL_COMPUTE_SHADER;
                // Mesh/task (requires extensions/core version); placeholders:
            case RAL::ShaderStage::Task:                    return 0; // GL_TASK_SHADER_NV / GL_MESH_SHADER_EXT if available
            case RAL::ShaderStage::Mesh:                    return 0; // GL_MESH_SHADER_NV / GL_MESH_SHADER_EXT if available
            case RAL::ShaderStage::RayTracing:              return 0; // No direct single stage enum in core GL
            case RAL::ShaderStage::None:                    return 0;
            default: throw std::runtime_error("Unknown ShaderStage");
        }
    }

    // --- MEMORY BARRIER / ACCESS TRANSLATION ---
    inline bool HasAccess(RAL::AccessFlags value, RAL::AccessFlags bit) {
        return (static_cast<uint64_t>(value) & static_cast<uint64_t>(bit)) != 0;
    }

    inline GLbitfield ToOpenGLMemoryBarrierMask(RAL::AccessFlags access) {
        GLbitfield mask = 0;
        if (HasAccess(access, RAL::AccessFlags::ShaderRead))
            mask |= GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
        if (HasAccess(access, RAL::AccessFlags::ShaderWrite))
            mask |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT;
        if (HasAccess(access, RAL::AccessFlags::ColorAttachmentRead) ||
            HasAccess(access, RAL::AccessFlags::ColorAttachmentWrite) ||
            HasAccess(access, RAL::AccessFlags::DepthStencilAttachmentRead) ||
            HasAccess(access, RAL::AccessFlags::DepthStencilAttachmentWrite))
            mask |= GL_FRAMEBUFFER_BARRIER_BIT;
        if (HasAccess(access, RAL::AccessFlags::TransferRead) ||
            HasAccess(access, RAL::AccessFlags::TransferWrite))
            mask |= GL_PIXEL_BUFFER_BARRIER_BIT;
        // HostRead / HostWrite have no direct GL memory barrier bits; CPU sync handled differently.
        return mask;
    }

    // (Optional) Combine stages + access if you want a single call site helper.
    inline GLbitfield ToOpenGLBarrierMask(RAL::PipelineStageFlags /*stages*/, RAL::AccessFlags access) {
        // Stages not directly used in classic GL barrier; access drives glMemoryBarrier mask.
        return ToOpenGLMemoryBarrierMask(access);
    }

    // --- IMAGE LAYOUT NOTE ---
    // OpenGL does not expose explicit image layouts; transitions are no-ops.
    inline void ApplyImageLayoutTransition(const RAL::ResourceBarrier::TextureTransition& /*transition*/) {
        // Intentional no-op (track state in engine if needed for validation)
    }

    GLenum ToOpenGLIndexType(RAL::IndexType t) {
        switch (t) {
            case RAL::IndexType::UINT16: return GL_UNSIGNED_SHORT;
            case RAL::IndexType::UINT32: return GL_UNSIGNED_INT;
            default: return GL_UNSIGNED_INT;
        }
    }


}