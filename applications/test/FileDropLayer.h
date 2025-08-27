#pragma once

#include "core/ILayer.h"

#include <vector>
#include <string>

namespace RDE{
    class FileDropLayer : public ILayer {
    public:
        FileDropLayer() = default;

        ~FileDropLayer() override = default;

        void on_attach() override;

        void on_detach() override;

        void on_update(float delta_time) override;

        void on_render_gui() override;

        void on_event(Event &e) override;

        const char * get_name() const override { return "FileDropLayer"; }
    private:
        void on_file_drop(const std::vector<std::string> &paths);


    };
}