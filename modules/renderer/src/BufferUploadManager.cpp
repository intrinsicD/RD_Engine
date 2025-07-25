#include "ral/BufferUploadManager.h"
#include "ral/CommandBuffer.h" // Needed for the flush() implementation
#include "core/Log.h"          // For logging errors/warnings
#include <cassert>             // For assertions on critical errors

namespace RAL {

    // A reasonable default size for a reusable staging buffer.
    // Can be tuned based on application needs.
    constexpr size_t STAGING_BUFFER_SIZE = 64 * 1024 * 1024; // 64 MB

    // Helper for memory alignment, as GPU copies can have alignment requirements.
    static size_t align_up(size_t size, size_t alignment) {
        return (size + alignment - 1) & ~(alignment - 1);
    }

    // --- Constructor: Sets up the manager's long-lived resources ---
    BufferUploadManager::BufferUploadManager(RAL::Device* device)
            : m_device(device), m_current_staging_offset(0)
    {
        RDE_CORE_INFO("Initializing BufferUploadManager...");

        // Create the single, large, reusable staging buffer.
        RAL::BufferDescription staging_desc = {};
        staging_desc.size = STAGING_BUFFER_SIZE;
        staging_desc.usage = RAL::BufferUsage::TransferSrc;
        staging_desc.memoryUsage = RAL::MemoryUsage::HostVisibleCoherent;

        m_staging_buffer = m_device->create_buffer(staging_desc);

        // Persistently map the buffer for its entire lifetime.
        // This avoids the expensive map/unmap overhead on every upload.
        m_staging_buffer_mapped_ptr = m_device->map_buffer(m_staging_buffer);

        if (!m_staging_buffer_mapped_ptr) {
            RDE_CORE_CRITICAL("Failed to map persistent staging buffer for UploadManager!");
            // This is a fatal error, the application cannot proceed.
        }
    }

    // --- Destructor: Cleans up the long-lived resources ---
    BufferUploadManager::~BufferUploadManager() {
        RDE_CORE_INFO("Shutting down BufferUploadManager...");

        // Ensure any pending work is flushed before destroying resources.
        // In a well-behaved app, the queue should be empty here.
        flush();

        if (m_staging_buffer_mapped_ptr) {
            m_device->unmap_buffer(m_staging_buffer);
        }
        if (m_staging_buffer.is_valid()) {
            m_device->destroy_buffer(m_staging_buffer);
        }
    }

    // --- The main public API function ---
    RAL::BufferHandle BufferUploadManager::create_and_upload_buffer(
            size_t size,
            const void* data,
            RAL::BufferUsage usage
    ) {
        // 1. Create the final, GPU-local destination buffer.
        // The user gets this handle back immediately.
        RAL::BufferDescription dest_desc = {};
        dest_desc.size = size;
        dest_desc.usage = usage | RAL::BufferUsage::TransferDst; // Must be a transfer destination
        dest_desc.memoryUsage = RAL::MemoryUsage::DeviceLocal;
        RAL::BufferHandle destination_buffer = m_device->create_buffer(dest_desc);

        // 2. Check if there's enough space in the staging buffer.
        // For simplicity, we require a 4-byte alignment for our copy source.
        size_t aligned_offset = align_up(m_current_staging_offset, 4);

        if (aligned_offset + size > STAGING_BUFFER_SIZE) {
            // Not enough space. Flush all pending uploads to make room.
            RDE_CORE_WARN("UploadManager staging buffer full. Flushing mid-frame.");
            flush();
            // After flushing, the offset is reset. Recalculate.
            aligned_offset = align_up(m_current_staging_offset, 4);
        }

        // This is a hard error: a single upload request is larger than our entire staging buffer.
        assert(aligned_offset + size <= STAGING_BUFFER_SIZE && "Single upload request exceeds total staging buffer size!");

        // 3. Copy the user's data into our persistently-mapped staging buffer.
        uint8_t* destination_in_staging = static_cast<uint8_t*>(m_staging_buffer_mapped_ptr) + aligned_offset;
        memcpy(destination_in_staging, data, size);

        // 4. Enqueue the copy operation. We don't do any GPU work yet.
        m_request_queue.push_back({
                                          .destination_buffer = destination_buffer,
                                          .source_offset_in_staging = aligned_offset,
                                          .destination_offset = 0,
                                          .size = size
                                  });

        // 5. "Bump" the offset for the next upload.
        m_current_staging_offset = aligned_offset + size;

        // 6. Return the handle to the final destination buffer.
        return destination_buffer;
    }

    // --- Processes all queued uploads for the frame ---
    void BufferUploadManager::flush() {
        if (m_request_queue.empty()) {
            // Nothing to do.
            return;
        }

        // Get a temporary command buffer to record our copy commands.
        // Using `get_command_buffer` is fine here because we will submit and wait on it,
        // ensuring it's free before the main render loop needs it.
        RAL::CommandBuffer* cmd = m_device->get_command_buffer();
        cmd->begin();

        // Record all the queued copy commands into this single command buffer.
        for (const auto& request : m_request_queue) {
            cmd->copy_buffer(
                    m_staging_buffer,           // Source is always our big staging buffer
                    request.destination_buffer,
                    request.size,
                    request.source_offset_in_staging, // With the correct offset
                    request.destination_offset
            );
        }

        cmd->end();

        // Submit the work and synchronously wait for the GPU to finish all copies.
        m_device->submit_and_wait({cmd});

        RDE_CORE_TRACE("UploadManager flushed {} requests.", m_request_queue.size());

        // Now that the GPU is done, we can safely reuse the entire staging buffer.
        m_request_queue.clear();
        m_current_staging_offset = 0;
    }
}