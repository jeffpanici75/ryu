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

#include <utility>
#include <core/view.h>
#include <core/core_types.h>

namespace ryu::core {

    struct pick_list_header_t {
        using valign_t = core::alignment::vertical::types;
        using halign_t = core::alignment::horizontal::types;

        pick_list_header_t(
            std::string title,
            uint8_t fg,
            uint8_t bg,
            uint32_t width,
            valign_t valign = valign_t::middle,
            halign_t halign = halign_t::center) : title(std::move(title)),
                                                  fg_color(fg),
                                                  bg_color(bg),
                                                  width(width),
                                                  valign(valign),
                                                  halign(halign) {
        }

        std::string title;
        uint8_t fg_color;
        uint8_t bg_color;
        int32_t width;
        valign_t valign;
        halign_t halign;
    };

    struct pick_list_row_t {
        uint32_t key;
        std::vector<std::string> columns {};
    };

    using row_list = std::vector<pick_list_row_t>;
    using header_list = std::vector<pick_list_header_t>;

    class column_pick_list : public core::view {
    public:
        using activated_callable = std::function<void (uint32_t)>;
        using selection_changed_callable = std::function<void (int32_t)>;

        using valign_t = core::alignment::vertical::types;
        using halign_t = core::alignment::horizontal::types;

        column_pick_list(
            const std::string& name,
            core::view_host* host);

        void clear();

        void add_header(
            const std::string& title,
            uint8_t fg_color,
            uint8_t bg_color,
            uint32_t width,
            halign_t halign = halign_t::center,
            valign_t valign = valign_t::middle);

        void clear_rows();

        uint32_t selected() const;

        const row_list& rows() const;

        border::types border() const;

        void remove_row(uint32_t index);

        void border(border::types value);

        void add_row(const pick_list_row_t& row);

        void on_activated(const activated_callable& callable);

        void on_selection_changed(const selection_changed_callable& callable);

    protected:
        bool move_up();

        bool move_down();

        bool move_row_up();

        bool move_row_down();

        void raise_activated();

        void on_initialize() override;

        void raise_selection_changed();

        void on_draw(core::renderer& surface) override;

        void on_resize(const core::rect& context_bounds) override;

    private:
        uint32_t _row = 0;
        row_list _rows {};
        uint32_t _max_rows = 0;
        uint32_t _selected = 0;
        header_list _headers {};
        uint32_t _visible_rows = 0;
        activated_callable _activated_callable;
        border::types _border = border::types::none;
        selection_changed_callable _selection_changed_callable;
    };

};