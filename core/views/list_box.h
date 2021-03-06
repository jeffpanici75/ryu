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

#include <core/view.h>

namespace ryu::core {

    class list_box  : public core::view {
    public:
        using on_clicked_callable = std::function<void ()>;

        list_box(
            const std::string& name,
            core::view_host* host);

        ~list_box() override;

        option_list& options();

        void row_color(palette_index value);

        void selection_color(palette_index value);

        void on_clicked(const on_clicked_callable& callable);

    protected:
        void bind_events();

        void define_actions();

        void on_initialize() override;

        void on_bounds_changed() override;

        void on_font_family_changed() override;

        void on_draw(core::renderer& surface) override;

    private:
        bool move_up();

        bool move_down();

        bool move_row_up();

        bool move_row_down();

        void calculate_visible_rows();

    private:
        size_t _row = 0;
        size_t _selection = 0;
        option_list _options {};
        palette_index _row_color;
        size_t _visible_rows = 0;
        size_t _selected_item = 0;
        palette_index _selection_color;
        on_clicked_callable _on_clicked {};
    };

};

