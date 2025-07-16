#include "assets/FileWatcher.h"
#include "core/Log.h" // Your logging utility

// Include the actual efsw header here in the .cpp
#include <efsw/efsw.hpp>

namespace RDE {
    // Implementation-detail listener class. Inherits from efsw's base listener.
    class FileUpdateListener : public efsw::FileWatchListener {
    public:
        // Constructor takes a pointer to the queue we will push events to.
        explicit FileUpdateListener(ThreadSafeQueue<std::string> *queue_mod = nullptr,
                                    ThreadSafeQueue<std::string> *queue_add = nullptr,
                                    ThreadSafeQueue<std::string> *queue_delete = nullptr,
                                    ThreadSafeQueue<std::string> *queue_moved = nullptr
        ) : m_event_queue_modified(queue_mod),
            m_event_queue_add(queue_add),
            m_event_queue_delete(queue_delete),
            m_event_queue_moved(queue_moved) {
        }

        void handleFileAction(efsw::WatchID watchid, const std::string &dir,
                              const std::string &filename, efsw::Action action,
                              std::string old_filename) override {
            // We only care about certain actions.
            // For hot reloading, 'Modified' is the most important.
            // 'Add' can be useful for adding new assets while the engine is running.
            switch (action) {
                case efsw::Actions::Modified: {
                    if (m_event_queue_modified) {
                        std::string full_path = dir + filename;
                        // Normalize path separators for consistency
                        std::replace(full_path.begin(), full_path.end(), '\\', '/');
                        m_event_queue_modified->push(full_path);
                        RDE_CORE_TRACE("FileWatcher: Queued mod change for '{}'", full_path);
                    }
                    break;
                }
                case efsw::Actions::Add: {
                    if (m_event_queue_add) {
                        std::string full_path = dir + filename;
                        // Normalize path separators for consistency
                        std::replace(full_path.begin(), full_path.end(), '\\', '/');
                        m_event_queue_add->push(full_path);
                        RDE_CORE_TRACE("FileWatcher: Queued add change for '{}'", full_path);
                    }
                    break;
                }
                case efsw::Actions::Delete: {
                    if (m_event_queue_delete) {
                        std::string full_path = dir + filename;
                        // Normalize path separators for consistency
                        std::replace(full_path.begin(), full_path.end(), '\\', '/');
                        m_event_queue_delete->push(full_path);
                        RDE_CORE_TRACE("FileWatcher: Queued delete change for '{}'", full_path);
                    }
                    break;
                }
                case efsw::Actions::Moved: {
                    if (m_event_queue_moved) {
                        std::string full_path = dir + filename;
                        // Normalize path separators for consistency
                        std::replace(full_path.begin(), full_path.end(), '\\', '/');
                        m_event_queue_moved->push(full_path);
                        RDE_CORE_TRACE("FileWatcher: Queued change moved for '{}'", full_path);
                    }
                    break;
                }
                break;
            }
            //TODO above, i still get two notifications... fix this somehow...
        }

    private:
        ThreadSafeQueue<std::string> *m_event_queue_modified;
        ThreadSafeQueue<std::string> *m_event_queue_add;
        ThreadSafeQueue<std::string> *m_event_queue_delete;
        ThreadSafeQueue<std::string> *m_event_queue_moved;
    };

    // --- FileWatcher Member Implementations ---

    FileWatcher::FileWatcher() = default;

    FileWatcher::~FileWatcher() {
        stop();
    }

    void FileWatcher::start(const std::string &directory,
                            ThreadSafeQueue<std::string> *queue_mod,
                            ThreadSafeQueue<std::string> *queue_add,
                            ThreadSafeQueue<std::string> *queue_delete,
                            ThreadSafeQueue<std::string> *queue_moved) {
        if (m_is_running) {
            RDE_CORE_WARN("FileWatcher::start() called, but it is already running.");
            return;
        }

        // 1. Create the watcher and listener ON THE MAIN THREAD.
        // The listener must live as long as the watcher.
        // We pass ownership to the thread via capture.
        m_watcher = std::make_unique<efsw::FileWatcher>();
        m_listener = std::make_unique<FileUpdateListener>(queue_mod, queue_add, queue_delete, queue_moved);

        // 2. Set up the watch before starting the thread.
        efsw::WatchID watch_id = m_watcher->addWatch(directory, m_listener.get(), true /*recursive*/);
        if (watch_id < 0) {
            RDE_CORE_ERROR("FileWatcher: Failed to start watching directory '{}'.", directory);
            m_watcher.reset(); // Clean up on failure
            m_listener.reset();
            return;
        }

        m_is_running = true;

        // 3. Launch the thread.
        // We move the listener into the lambda to ensure its lifetime.
        m_thread = std::thread([this]() {
            RDE_CORE_TRACE("FileWatcher: Thread started. Now blocking on watch().");

            // This is the blocking call. It will only return when the
            // m_watcher object is destroyed from another thread.
            m_watcher->watch();

            RDE_CORE_TRACE("FileWatcher: watch() unblocked. Thread exiting.");
        });
    }

    void FileWatcher::stop() {
        if (!m_is_running) {
            return;
        }

        RDE_CORE_TRACE("FileWatcher: Stopping...");

        // 1. Destroy the efsw::FileWatcher object.
        // This is the key: The destructor of efsw::FileWatcher is thread-safe.
        // It will signal the internal efsw thread, which causes the blocking
        // `watch()` call in our own thread to unblock and return.
        m_watcher.reset();

        // 2. Our thread's lambda can now complete. We wait for it to exit cleanly.
        if (m_thread.joinable()) {
            m_thread.join();
        }

        m_listener.reset();
        m_is_running = false;
        RDE_CORE_TRACE("FileWatcher: Stopped successfully.");
    }
}
