#include "FileWatcher.h"
#include "Log.h" // Your logging utility

// Include the actual efsw header here in the .cpp
#include <efsw/efsw.hpp>

namespace RDE {
    // Implementation-detail listener class. Inherits from efsw's base listener.
    class FileUpdateListener : public efsw::FileWatchListener {
    public:
        // Constructor takes a pointer to the queue we will push events to.
        explicit FileUpdateListener(ThreadSafeQueue<std::string>* queue) : m_event_queue(queue) {}

        void handleFileAction(efsw::WatchID watchid, const std::string& dir,
                                const std::string& filename, efsw::Action action,
                                std::string old_filename) override {
            // We only care about certain actions.
            // For hot reloading, 'Modified' is the most important.
            // 'Add' can be useful for adding new assets while the engine is running.
            switch (action) {
                case efsw::Actions::Modified:
                case efsw::Actions::Add: {
                    if (m_event_queue) {
                        std::string full_path = dir + filename;
                        // Normalize path separators for consistency
                        std::replace(full_path.begin(), full_path.end(), '\\', '/');
                        m_event_queue->push(full_path);
                        RDE_CORE_TRACE("FileWatcher: Queued change for '{}'", full_path);
                    }
                    break;
                }
                case efsw::Actions::Delete:
                case efsw::Actions::Moved:
                    // We could handle these, but for hot reload, we'll ignore them for now.
                    RDE_CORE_TRACE("FileWatcher: Detected Delete/Move for '{}'. Ignoring.", filename);
                    break;
            }
        }

    private:
        ThreadSafeQueue<std::string>* m_event_queue;
    };

    // --- FileWatcher Member Implementations ---

    FileWatcher::FileWatcher() = default;

    FileWatcher::~FileWatcher() {
        stop();
    }

    void FileWatcher::start(const std::string& directory, ThreadSafeQueue<std::string>* event_queue) {
        if (m_is_running) {
            RDE_CORE_WARN("FileWatcher::start() called, but it is already running.");
            return;
        }

        // 1. Create the watcher and listener ON THE MAIN THREAD.
        // The listener must live as long as the watcher.
        // We pass ownership to the thread via capture.
        m_watcher = std::make_unique<efsw::FileWatcher>();
        auto listener = std::make_unique<FileUpdateListener>(event_queue);

        // 2. Set up the watch before starting the thread.
        efsw::WatchID watch_id = m_watcher->addWatch(directory, listener.get(), true /*recursive*/);
        if (watch_id < 0) {
            RDE_CORE_ERROR("FileWatcher: Failed to start watching directory '{}'.", directory);
            m_watcher.reset(); // Clean up on failure
            return;
        }

        m_is_running = true;

        // 3. Launch the thread.
        // We move the listener into the lambda to ensure its lifetime.
        m_thread = std::thread([this, listener = std::move(listener)]() {
            RDE_CORE_INFO("FileWatcher: Thread started. Now blocking on watch().");

            // This is the blocking call. It will only return when the
            // m_watcher object is destroyed from another thread.
            m_watcher->watch();

            RDE_CORE_INFO("FileWatcher: watch() unblocked. Thread exiting.");
        });
    }

    void FileWatcher::stop() {
        if (!m_is_running) {
            return;
        }

        RDE_CORE_INFO("FileWatcher: Stopping...");

        // 1. Destroy the efsw::FileWatcher object.
        // This is the key: The destructor of efsw::FileWatcher is thread-safe.
        // It will signal the internal efsw thread, which causes the blocking
        // `watch()` call in our own thread to unblock and return.
        m_watcher.reset();

        // 2. Our thread's lambda can now complete. We wait for it to exit cleanly.
        if (m_thread.joinable()) {
            m_thread.join();
        }

        m_is_running = false;
        RDE_CORE_INFO("FileWatcher: Stopped successfully.");
    }
}