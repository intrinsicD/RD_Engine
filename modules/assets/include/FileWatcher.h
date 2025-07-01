#pragma once

#include <string>
#include <thread>
#include <atomic>

// Forward declare to avoid including the library's header here
namespace efsw {
    class FileWatcher;
}

// A simple thread-safe queue for demonstration purposes.
// In a real engine, you might use a more performant lock-free queue.
#include "internal/ThreadSafeQueue.h" // You would create this utility header

namespace RDE {
    class FileUpdateListener; // Forward declare our listener class
    class FileWatcher {
    public:
        FileWatcher();

        ~FileWatcher();

        // Disallow copying and moving to keep ownership simple
        FileWatcher(const FileWatcher &) = delete;

        FileWatcher &operator=(const FileWatcher &) = delete;

        /**
         * @brief Starts watching a directory for changes.
         * @param directory The path to the directory to watch.
         * @param event_queue A pointer to the thread-safe queue where file paths will be pushed.
         * This uses dependency injection, decoupling the watcher from the consumer.
         */
        void start(const std::string &directory, ThreadSafeQueue<std::string> *event_queue_modified,
                   ThreadSafeQueue<std::string> *event_queue_add = nullptr,
                   ThreadSafeQueue<std::string> *event_queue_delete = nullptr,
                   ThreadSafeQueue<std::string> *event_queue_moved = nullptr);

        /**
         * @brief Stops the file watcher thread and cleans up resources.
         */
        void stop();

    private:
        std::unique_ptr<efsw::FileWatcher> m_watcher;
        std::unique_ptr<FileUpdateListener> m_listener;
        std::thread m_thread;
        std::atomic<bool> m_is_running{false};
    };
}
