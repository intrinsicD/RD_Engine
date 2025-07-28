#pragma once

#include "ral/Device.h"
#include "ral/CommandBuffer.h"

namespace RAL {
    class BufferUploadManager {
    public:
        explicit BufferUploadManager(RAL::Device* device);
        ~BufferUploadManager();

        // --- Core Public API ---

        /**
         * @brief Creates a new, empty, device-local buffer. Does not upload any data.
         * @return Handle to the newly created GPU buffer.
         */
        BufferHandle create_buffer(size_t size, RAL::BufferUsage usage);

        /**
         * @brief Queues a data upload to an existing buffer. Does not create a buffer.
         * This is the core transfer operation.
         */
        void update_buffer(RAL::BufferHandle handle, const void* data, size_t size, size_t offset = 0);

        /**
         * @brief A convenience function that creates a new buffer and immediately queues an upload to it.
         * @return Handle to the newly created and populated GPU buffer.
         */
        BufferHandle create_and_upload_buffer(size_t size, const void* data, RAL::BufferUsage usage);

        /**
         * @brief The primary function for systems. Ensures a buffer exists with the correct size,
         * reallocating if necessary, and then queues an upload.
         * @param handle A reference to the buffer handle. Will be updated if reallocation occurs.
         */
        void update_or_create_buffer(
                RAL::BufferHandle& handle, // Passed by reference
                size_t new_size,
                const void* data,
                RAL::BufferUsage usage
        );

        /**
         * @brief Submits all queued copy commands to the GPU and waits for them to complete.
         */
        void flush();

    private:
        // --- Private Implementation ---

        // The fundamental, private upload-queuing logic.
        void queue_upload(RAL::BufferHandle destination, const void* data, size_t size, size_t offset);

        struct QueuedUpload {
            RAL::BufferHandle destination_buffer;
            uint64_t source_offset_in_staging;
            uint64_t destination_offset;
            uint64_t size;
        };

        RAL::Device* m_device;
        RAL::BufferHandle m_staging_buffer;
        void* m_staging_buffer_mapped_ptr;
        uint64_t m_current_staging_offset;
        std::vector<QueuedUpload> m_request_queue;
    };
}