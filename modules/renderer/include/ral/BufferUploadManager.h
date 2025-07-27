//ral/BufferUploadManager.h
#pragma once

#include "ral/Device.h"
#include "ral/CommandBuffer.h"

namespace RAL {
    class BufferUploadManager {
    public:
        // The manager takes ownership of its dependencies.
        explicit BufferUploadManager(RAL::Device* device);

        ~BufferUploadManager();

        // The ONE public function for creating and uploading a buffer.
        // This is the core of the API. It's a single, clear request.
        // It returns the handle to the FINAL destination buffer immediately.
        RAL::BufferHandle create_and_upload_buffer(
                size_t size,
                const void* data,
                RAL::BufferUsage usage
        );

        // This is called ONCE per frame by the main engine loop to process all
        // queued uploads in a single, efficient batch.
        void flush();

    private:
        struct QueuedUpload {
            RAL::BufferHandle destination_buffer;
            uint64_t source_offset_in_staging; // Where the data is in our big staging buffer
            uint64_t destination_offset;
            uint64_t size;
        };

        // Internal resources owned by the manager
        RAL::Device* m_device;
        RAL::BufferHandle m_staging_buffer; // A single, large, reusable staging buffer
        void* m_staging_buffer_mapped_ptr;  // It's persistently mapped for performance
        uint64_t m_current_staging_offset;

        std::vector<QueuedUpload> m_request_queue;
    };
}