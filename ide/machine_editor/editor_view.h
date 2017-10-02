//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#pragma once

#include <core/result.h>
#include <core/project.h>
#include <core/views/label.h>
#include <core/views/panel.h>
#include <hardware/machine.h>
#include <core/views/button.h>
#include <core/views/textbox.h>
#include <core/views/pick_list.h>

namespace ryu::ide::machine_editor {

    class editor_view : public core::view {
    public:
        explicit editor_view(const std::string& name);

        void initialize();

        core::project* project();

        hardware::machine* machine();

        void project(core::project* value);

        void machine(hardware::machine* value);

    protected:
        struct metrics_t {
            const int left_padding = 10;
            const int right_padding = 10;
        };

        void on_focus_changed() override;

        void on_draw(core::renderer& surface) override;

        bool on_process_event(const SDL_Event* e) override;

    private:
        void update_values();

    private:
        metrics_t _metrics;
        core::label _header;
        core::label _footer;
        core::panel _row1_panel;
        core::panel _row2_panel;
        core::label _name_label;
        core::button _map_button;
        core::button _add_button;
        core::panel _button_panel;
        core::label _display_label;
        core::button _delete_button;
        core::textbox _name_textbox;
        core::label _address_space_label;
        core::project* _project = nullptr;
        core::pick_list _display_pick_list;
        core::textbox _address_space_textbox;
        hardware::machine* _machine = nullptr;
    };

};

