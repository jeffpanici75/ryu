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
#include "caret.h"

namespace ryu::core {

    class pick_list : public core::view {
    public:
        using selection_changed_callable = std::function<void (int32_t)>;

        pick_list(
            const std::string& name,
            core::view_host* host);

        ~pick_list() override;

        int width() const;

        int height() const;

        int length() const;

        void reset_search();

        void width(int value);

        void height(int value);

        option_list& options();

        void length(int value);

        uint32_t visible_rows() const;

        void selected_key(uint32_t key);

        void visible_rows(uint32_t value);

        std::string value() const override;

        void row_color(palette_index value);

        void selection_color(palette_index value);

        void not_found_color(palette_index value);

        void value(const std::string& text) override;

        void on_selection_changed(const selection_changed_callable& callable);

    protected:
        bool page_up();

        bool move_up();

        void move_top();

        bool move_down();

        bool page_down();

        bool move_row_up();

        bool move_row_down();

        void on_initialize() override;

        void raise_selection_changed();

        void on_focus_changed() override;

        void on_palette_changed() override;

        void on_font_family_changed() override;

        void on_draw(core::renderer& surface) override;

    private:
        void bind_events();

        void define_actions();

        void update_minimum_size();

        bool find_matching_text(const std::string& text);

    private:
        bool _found;
        int _width = 0;
        int _height = 0;
        int _length = 32;
        uint32_t _row = 0;
        core::caret _caret;
        std::string _search;
        uint32_t _selection = 0;
        option_list _options {};
        palette_index _row_color;
        uint32_t _selected_item = 0;
        uint32_t _visible_rows = 10;
        palette_index _selection_color;
        palette_index _not_found_color;
        selection_changed_callable _selection_changed_callable;
    };

};