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
    BufferUploadManager::BufferUploadManager(RAL::Device *device)
            : m_device(device), m_current_staging_offset(0) {
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

    // --- Private Helper ---
    // This is the single place where data is copied to the staging buffer and a command is enqueued.
    void
    BufferUploadManager::queue_upload(RAL::BufferHandle destination, const void *data, size_t size, size_t offset) {
        assert(destination.is_valid() && "Destination buffer for upload is invalid.");

        size_t aligned_offset = align_up(m_current_staging_offset, 4);

        if (aligned_offset + size > STAGING_BUFFER_SIZE) {
            RDE_CORE_WARN("UploadManager staging buffer full. Flushing mid-frame.");
            flush();
            aligned_offset = align_up(m_current_staging_offset, 4);
        }
        assert(aligned_offset + size <= STAGING_BUFFER_SIZE &&
               "Single upload request exceeds total staging buffer size!");

        uint8_t *destination_in_staging = static_cast<uint8_t *>(m_staging_buffer_mapped_ptr) + aligned_offset;
        memcpy(destination_in_staging, data, size);

        m_request_queue.push_back({
                                          .destination_buffer = destination,
                                          .source_offset_in_staging = aligned_offset,
                                          .destination_offset = offset,
                                          .size = size
                                  });

        m_current_staging_offset = aligned_offset + size;
    }


    // --- Public API Implementation ---

    RAL::BufferHandle BufferUploadManager::create_buffer(size_t size, RAL::BufferUsage usage) {
        RAL::BufferDescription dest_desc = {};
        dest_desc.size = size;
        // The buffer must be a transfer destination to receive the staged data.
        dest_desc.usage = usage | RAL::BufferUsage::TransferDst;
        dest_desc.memoryUsage = RAL::MemoryUsage::DeviceLocal;
        return m_device->create_buffer(dest_desc);
    }

    void BufferUploadManager::update_buffer(RAL::BufferHandle handle, const void *data, size_t size, size_t offset) {
        queue_upload(handle, data, size, offset);
    }

    RAL::BufferHandle
    BufferUploadManager::create_and_upload_buffer(size_t size, const void *data, RAL::BufferUsage usage) {
        // Compose the two fundamental operations.
        RAL::BufferHandle new_handle = create_buffer(size, usage);
        queue_upload(new_handle, data, size, 0);
        return new_handle;
    }

    void BufferUploadManager::update_or_create_buffer(
            RAL::BufferHandle &handle,
            size_t new_size,
            const void *data,
            RAL::BufferUsage usage
    ) {
        // Case 1: The buffer doesn't exist yet. Create and upload.
        if (!handle.is_valid()) {
            handle = create_and_upload_buffer(new_size, data, usage);
            return;
        }

        // Get info about the existing buffer. This requires the Device to provide it.
        // Assuming you have `get<T>` on your ResourceDatabase
        auto &existing_desc = m_device->get_resources_database().get<RAL::BufferDescription>(handle);

        // Case 2: The buffer exists, but its size is wrong. Reallocate.
        if (existing_desc.size != new_size) {
            // Queue the old handle for safe, deferred destruction.
            m_device->destroy_buffer(handle); // the destroy functions are already delayed in the Device

            // Create a new buffer and update the user's handle reference.
            handle = create_and_upload_buffer(new_size, data, usage);
            return;
        }

        // Case 3: The buffer exists and has the correct size. Do an in-place update.
        queue_upload(handle, data, new_size, 0);
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
        RAL::CommandBuffer *cmd = m_device->get_command_buffer();
        cmd->begin();

        // Record all the queued copy commands into this single command buffer.
        for (const auto &request: m_request_queue) {
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