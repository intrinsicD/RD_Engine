 # Example code for a hot-reload system in an asset management context:

    class Engine {
        // ...
        AssetManager m_asset_manager;
        FileWatcher m_file_watcher;
        ThreadSafeQueue<std::string> m_hot_reload_queue;
        // ...
        
    public:
    
        void Initialize() {
            // ...
            m_file_watcher.start("./assets", &m_hot_reload_queue);
        }
    
        void Update() {
            // ...
            HotReloadSystem_Update();
            // ...
        }
    
        void HotReloadSystem_Update() {
            std::optional<std::string> changed_uri;
            while((changed_uri = m_hot_reload_queue.try_pop()).has_value()) {
                m_asset_manager.force_load_from(changed_uri.value());
            }
        }
        
        void Shutdown() {
            m_file_watcher.stop();
            // ...
        }
    };