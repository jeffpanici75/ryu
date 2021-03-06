//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <core/input_action.h>
#include "column_pick_list.h"

namespace ryu::core {

    column_pick_list::column_pick_list(
            const std::string& name,
            core::view_host* host) : core::view(types::control, name, host),
                                     _caret("column-pick-list-caret", host) {
    }

    column_pick_list::~column_pick_list() {
    }

    void column_pick_list::clear() {
        _headers.clear();
        _rows.clear();
    }

    bool column_pick_list::move_up() {
        auto result = true;
        if (_selected == 0) {
            result = move_row_up();
        } else {
            if (_selected > _rows.size() - 1)
                _selected = static_cast<uint32_t>(_rows.size() - 1);
            else
                _selected--;
        }
        _page = (_selected + _row) / _visible_rows;
        return result;
    }

    void column_pick_list::add_header(
            const std::string& title,
            uint8_t fg_color,
            uint8_t bg_color,
            uint32_t width,
            pick_list_header_t::formats format,
            pick_list_header_t::types type,
            halign_t halign,
            valign_t valign) {
        _headers.emplace_back(
            title,
            fg_color,
            bg_color,
            width,
            format,
            type,
            valign,
            halign);
    }

    bool column_pick_list::page_up() {
        auto result = false;
        if (_visible_rows > _row) {
            _row = 0;
            result = true;
        }
        else {
            _row -= _visible_rows;
        }
        _page = (_selected + _row) / _visible_rows;
        return result;
    }

    bool column_pick_list::move_top() {
        _row = 0;
        _selected = 0;
        return false;
    }

    bool column_pick_list::page_down() {
        auto result = false;
        auto next_start_row = _row + _visible_rows;
        if (next_start_row + _max_rows > _rows.size() - 1) {
            if (_visible_rows > _rows.size())
                _row = 0;
            else
                _row = static_cast<uint32_t>(_rows.size() - _visible_rows);
            result = true;
        } else {
            _row += _visible_rows;
        }
        _page = (_selected + _row) / _visible_rows;
        return result;
    }

    bool column_pick_list::move_down() {
        auto result = false;
        if (_selected < _visible_rows - 1
        &&  (_row + _selected) < _rows.size() - 1) {
            _selected++;
        }
        else
            result = move_row_down();
        _page = (_selected + _row) / _visible_rows;
        return result;
    }

    void column_pick_list::clear_rows() {
        _rows.clear();
        update_state();
        raise_selection_changed();
    }

    bool column_pick_list::move_row_up() {
        if (_row == 0) {
            return false;
        }
        _row--;
        return true;
    }

    bool column_pick_list::move_row_down() {
        if (_rows.size() < _visible_rows)
            return false;
        auto max_start = _rows.size() - _visible_rows;
        if (_row < max_start) {
            _row++;
            return false;
        }
        return true;
    }

    void column_pick_list::define_actions() {
        auto left_action = core::input_action::create_no_map(
            "column_pick_list_left_action",
            "Internal",
            "Move to the previous editable column.");
        if (!left_action->has_bindings())
            left_action->bind_keys({core::key_left});

        auto right_action = core::input_action::create_no_map(
            "column_pick_list_right_action",
            "Internal",
            "Move to the next editable column.");
        if (!right_action->has_bindings())
            right_action->bind_keys({core::key_right});

        auto up_action = core::input_action::create_no_map(
            "column_pick_list_up_action",
            "Internal",
            "Move to the previous pick list item.");
        if (!up_action->has_bindings())
            up_action->bind_keys({core::key_up});

        auto page_up_action = core::input_action::create_no_map(
            "column_pick_list_page_up_action",
            "Internal",
            "Move to the previous page of the pick list.");
        if (!page_up_action->has_bindings())
            page_up_action->bind_keys({core::key_page_up});

        auto down_action = core::input_action::create_no_map(
            "column_pick_list_down_action",
            "Internal",
            "Move to the next pick list item.");
        if (!down_action->has_bindings())
            down_action->bind_keys({core::key_down});

        auto page_down_action = core::input_action::create_no_map(
            "column_pick_list_page_down_action",
            "Internal",
            "Move to the next page of the pick list.");
        if (!page_down_action->has_bindings())
            page_down_action->bind_keys({core::key_page_down});

        auto select_action = core::input_action::create_no_map(
            "column_pick_list_select_action",
            "Internal",
            "Make current item the selected value and enter edit mode.");
        if (!select_action->has_bindings())
            select_action->bind_keys({core::key_return});

        auto backspace_action = core::input_action::create_no_map(
            "column_pick_list_backspace_action",
            "Internal",
            "Delete a character from the type ahead search.");
        if (!backspace_action->has_bindings())
            backspace_action->bind_keys({core::key_backspace});

        // XXX: this conflicts with delete being used in other places
        auto delete_action = core::input_action::create_no_map(
            "column_pick_list_delete_action",
            "Internal",
            "Reset the type ahead search.");
        if (!delete_action->has_bindings())
            delete_action->bind_keys({core::key_delete});

        auto text_input_action = core::input_action::create_no_map(
            "column_pick_list_text_input",
            "Internal",
            "Any ASCII text input (non-mappable).");
        if (!text_input_action->has_bindings())
            text_input_action->bind_text_input();
    }

    void column_pick_list::bind_events() {
        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_left_action"),
            [this](const event_data_t& data) {
                switch (_state) {
                    case states::none:
                    case states::select_row: {
                        return false;
                    }
                    case states::edit_row: {
                        auto& active_row = _rows[selected()];
                        if (active_row.active_column_index > 0) {
                            active_row.active_column_index--;
                            enable_caret_for_text_box();
                        }
                        return true;
                    }
                }
            });

        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_right_action"),
            [this](const event_data_t& data) {
                switch (_state) {
                    case states::none:
                    case states::select_row: {
                        return false;
                    }
                    case states::edit_row: {
                        auto& active_row = _rows[selected()];
                        if (active_row.active_column_index < active_row.editable_columns.size() - 1) {
                            active_row.active_column_index++;
                            enable_caret_for_text_box();
                        }
                        return true;
                    }
                }
            });

        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_up_action"),
            [this](const event_data_t& data) {
                leave_edit_mode_if_active();
                if (move_up())
                    raise_selection_changed();
                return true;
            });

        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_page_up_action"),
            [this](const event_data_t& data) {
                leave_edit_mode_if_active();
                page_up();
                return true;
            });

        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_down_action"),
            [this](const event_data_t& data) {
                leave_edit_mode_if_active();
                if (move_down())
                    raise_selection_changed();
                return true;
            });

        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_page_down_action"),
            [this](const event_data_t& data) {
                leave_edit_mode_if_active();
                page_down();
                return true;
            });

        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_select_action"),
            [this](const event_data_t& data) {
                switch (_state) {
                    case states::none:
                        return false;
                    case states::select_row: {
                        raise_activated();
                        reset_search(false);
                        if (has_editable_rows())
                            enter_edit_mode();
                        return true;
                    }
                    case states::edit_row: {
                        auto& active_row = _rows[selected()];
                        const auto column_index = active_row.editable_columns[active_row.active_column_index];
                        auto& active_column = active_row.columns[column_index];
                        const auto& header = _headers[column_index];

                        switch (header.type) {
                            case pick_list_header_t::types::check_box: {
                                auto value = std::get<bool>(active_column.value);
                                value = !value;
                                active_column.value = value;
                                active_column.changed = true;
                                return true;
                            }
                            case pick_list_header_t::types::button: {
                                raise_button_activated(
                                    selected(),
                                    static_cast<uint32_t>(column_index));
                                return true;
                            }
                            case pick_list_header_t::types::text_box:
                            default:
                                return false;
                        }
                    }
                }
            });

        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_backspace_action"),
            [this](const event_data_t& data) {
                switch (_state) {
                    case states::none:
                        return false;
                    case states::select_row: {
                        move_top();
                        _search = _search.substr(0, _search.length() - 1);
                        _caret.column(static_cast<uint8_t>(_search.length()));
                        _found = find_matching_text(_search);
                        return true;
                    }
                    case states::edit_row: {
                        auto& active_row = _rows[selected()];
                        const auto column_index = active_row.editable_columns[active_row.active_column_index];
                        auto& active_column = active_row.columns[column_index];

                        active_column.formatted_value = active_column.formatted_value.substr(
                            0,
                            active_column.formatted_value.length() - 1);
                        _caret.column(static_cast<uint8_t>(active_column.formatted_value.length()));
                        return true;
                    }
                }
            });

        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_delete_action"),
            [this](const event_data_t& data) {
                switch (_state) {
                    case states::none:
                        return false;
                    case states::select_row:
                        reset_search();
                        return true;
                    case states::edit_row:
                        return false;
                }
            });

        action_provider().register_handler(
            core::input_action::find_by_name("column_pick_list_text_input"),
            [this](const core::event_data_t& data) {
                if (data.c == core::ascii_tab
                ||  data.c == core::ascii_return
                ||  data.c == core::ascii_escape) {
                    return false;
                }

                switch (_state) {
                    case states::none: {
                        return false;
                    }
                    case states::select_row: {
                        if (_search.length() < 32) {
                            _search += data.c;
                            _caret.column(static_cast<uint8_t>(_search.length()));
                            _found = find_matching_text(_search);
                        }
                        return true;
                    }
                    case states::edit_row: {
                        auto& active_row = _rows[selected()];
                        const auto column_index = active_row.editable_columns[active_row.active_column_index];
                        auto& active_column = active_row.columns[column_index];

                        // XXX: this...is a problem
                        active_column.formatted_value += data.c;
                        _caret.column(static_cast<uint8_t>(active_column.formatted_value.length()));
                        return true;
                    }
                }
            });
    }

    void column_pick_list::update_state() {
        if (_rows.empty()) {
            _state = states::none;
        }
        else {
            if (_state == states::none)
                _state = states::select_row;
        }
    }

    void column_pick_list::on_initialize() {
        define_actions();
        bind_events();

        _caret.index(1001);
        _caret.enabled(false);
        _caret.position(0, 0);
        _caret.page_size(1, 128);
        _caret.palette(palette());
        _caret.fg_color(fg_color());
        _caret.bg_color(bg_color());
        _caret.font_family(font_family());
        _caret.initialize();
        add_child(&_caret);

        tab_stop(true);

        auto& minimum_size = min_size();
        minimum_size.dimensions(256, 256);
    }

    void column_pick_list::raise_activated() {
        if (_activated_callable != nullptr)
            _activated_callable(selected());
    }

    void column_pick_list::enter_edit_mode() {
        if (_state == states::edit_row)
            return;

        _state = states::edit_row;

        pick_list_row_t& active_row = _rows[selected()];
        active_row.active_column_index = 0;

        std::vector<size_t>& editable_columns = active_row.editable_columns;
        editable_columns.clear();
        size_t index = 0;
        for (const auto& header : _headers) {
            if (header.type != pick_list_header_t::types::value)
                editable_columns.push_back(index);
            ++index;
        }

        enable_caret_for_text_box();
    }

    void column_pick_list::on_focus_changed() {
    }

    void column_pick_list::on_bounds_changed() {
        calculate_visible_and_max_rows();
    }

    void column_pick_list::on_palette_changed() {
        _caret.palette(palette());
    }

    uint32_t column_pick_list::selected() const {
        return _row + _selected;
    }

    void column_pick_list::reset_search(bool move) {
        if (move)
            move_top();
        _search.clear();
        _caret.column(0);
        _found = false;
    }

    const row_list& column_pick_list::rows() const {
        return _rows;
    }

    void column_pick_list::on_font_family_changed() {
        calculate_visible_and_max_rows();
    }

    bool column_pick_list::has_editable_rows() const {
        for (const auto& header : _headers)
            if (header.type != pick_list_header_t::types::value)
                return true;
        return false;
    }

    void column_pick_list::raise_selection_changed() {
        if (_selection_changed_callable != nullptr) {
            if (_rows.empty())
                _selection_changed_callable(-1);
            else
                _selection_changed_callable(selected());
        }
    }

    void column_pick_list::remove_row(uint32_t index) {
        if (index < _rows.size()) {
            _rows.erase(_rows.begin() + index);
            calculate_visible_and_max_rows();
            if (move_up())
                raise_selection_changed();
        }
        update_state();
    }

    void column_pick_list::enable_caret_for_text_box() {
        auto& active_row = _rows[selected()];

        if (active_row.editable_columns.empty()) {
            _caret.enabled(false);
            return;
        }

        const auto column_index = active_row.editable_columns[active_row.active_column_index];
        const auto& header = _headers[column_index];

        if (header.type != pick_list_header_t::types::text_box) {
            _caret.enabled(false);
            return;
        }

        auto header_x_offset = 1;
        for (size_t i = 0; i < column_index; i++)
            header_x_offset += _headers[i].width + 3;

        auto& active_column = active_row.columns[column_index];

        _caret.padding().left(header_x_offset + 4);
        _caret.padding().top(((_selected + 1) * (_row_height + row_height_margin)) + 3);
        _caret.column(static_cast<uint8_t>(active_column.formatted_value.length()));
        _caret.enabled(true);
    }

    void column_pick_list::leave_edit_mode_if_active() {
        if (_state != states::edit_row)
            return;

        _state = states::select_row;

        pick_list_row_t& active_row = _rows[selected()];
        active_row.active_column_index = 0;

        std::vector<size_t>& editable_columns = active_row.editable_columns;
        editable_columns.clear();

        enable_caret_for_text_box();
    }

    void column_pick_list::row_color(palette_index value) {
        _row_color = value;
    }

    void column_pick_list::on_draw(core::renderer& surface) {
        auto pal = *palette();
        auto fg = adjust_color(pal[fg_color()]);
        auto& bg = pal[bg_color()];
        auto face = font_face();

        surface.set_color(bg);

        auto bounds = inner_bounds();
        surface.fill_rect(bounds);
        surface.push_clip_rect(inner_bounds());

        auto header_x = bounds.left();
        for (const auto& header : _headers) {
            core::rect header_rect {
                header_x + 1,
                bounds.top() + 1,
                header.width - 1,
                _row_height
            };
            surface.set_color(adjust_color(pal[header.bg_color]));
            surface.fill_rect(header_rect);
            surface.set_font_color(
                face,
                adjust_color(pal[header.fg_color]));
            header_rect.deflate(3, 2);
            surface.draw_text_aligned(
                face,
                header.title,
                header_rect,
                header.halign,
                header.valign);
            header_x += header.width + 2;
        }

        auto stop_row = std::min<uint32_t>(
            _row + _max_rows,
            static_cast<const uint32_t&>(_rows.size()));
        auto column_y = (bounds.top() + _row_height) + 6;
        for (auto row_index = _row;
                 row_index < stop_row;
                 row_index++) {
            auto& row = _rows[row_index];

            if (row_index == selected()) {
                core::rect selection_rect {
                    10,
                    column_y,
                    bounds.width(),
                    _row_height};

                if (_state == states::select_row) {
                    surface.draw_selection_rect(selection_rect, pal[_row_color]);
                } else if (_state == states::edit_row) {
                    surface.draw_selection_band(selection_rect, pal[_row_color]);
                }
            }

            auto index = 0;
            auto column_x = bounds.left() + 5;
            for (const auto& header : _headers) {
                auto clamped_width = column_x + header.width > bounds.right() ?
                                     bounds.right() - column_x :
                                     header.width;
                core::rect column_rect {
                    column_x,
                    column_y,
                    clamped_width,
                    _row_height
                };
                surface.push_clip_rect(column_rect);
                surface.set_font_color(
                    face,
                    adjust_color(pal[header.fg_color]));

                if (row.columns[index].changed) {
                    std::string formatted_value;
                    const auto& variant_value = row.columns[index].value;
                    auto variant_type = static_cast<pick_list_variant_types>(variant_value.index());

                    switch (variant_type) {
                        case pick_list_variant_types::empty:
                            formatted_value = "(none)";
                            break;
                        case pick_list_variant_types::boolean: {
                            auto flag = std::get<bool>(variant_value);
                            formatted_value = flag ? "X" : "";
                            break;
                        }
                        case pick_list_variant_types::string: {
                            formatted_value = std::get<std::string>(variant_value);
                            break;
                        }
                        case pick_list_variant_types::char_string: {
                            formatted_value = std::get<const char*>(variant_value);
                            break;
                        }
                        case pick_list_variant_types::u32: {
                            uint32_t value = std::get<uint32_t>(variant_value);
                            switch (header.format) {
                                case pick_list_header_t::formats::hex2: {
                                    formatted_value = fmt::format("${:02x}", value);
                                    break;
                                }
                                case pick_list_header_t::formats::hex4: {
                                    formatted_value = fmt::format("${:04x}", value);
                                    break;
                                }
                                case pick_list_header_t::formats::hex8: {
                                    formatted_value = fmt::format("${:08x}", value);
                                    break;
                                }
                                case pick_list_header_t::formats::binary2: {
                                    formatted_value = fmt::format("%{:02b}", value);
                                    break;
                                }
                                case pick_list_header_t::formats::binary4: {
                                    formatted_value = fmt::format("%{:04b}", value);
                                    break;
                                }
                                case pick_list_header_t::formats::binary8: {
                                    formatted_value = fmt::format("%{:08b}", value);
                                    break;
                                }
                                case pick_list_header_t::formats::binary16: {
                                    formatted_value = fmt::format("%{:016b}", value);
                                    break;
                                }
                                case pick_list_header_t::formats::binary32: {
                                    formatted_value = fmt::format("%{:032b}", value);
                                    break;
                                }
                                default: {
                                    formatted_value = std::to_string(value);
                                    break;
                                }
                            }
                            break;
                        }
                    }

                    row.columns[index].formatted_value = formatted_value;
                    row.columns[index].changed = false;
                }

                switch (header.type) {
                    case pick_list_header_t::types::value: {
                        surface.draw_text_aligned(
                            face,
                            row.columns[index].formatted_value,
                            column_rect,
                            header.halign,
                            header.valign);
                        break;
                    }
                    case pick_list_header_t::types::text_box: {
                        surface.set_color(adjust_color(pal[header.bg_color]));
                        surface.draw_rect(column_rect);

                        auto box_rect = column_rect;
                        box_rect.left(box_rect.left() + 4);
                        box_rect.top(box_rect.top() + 1);
                        box_rect.width(box_rect.width() - 2);
                        box_rect.height(box_rect.height() - 2);
                        surface.draw_text_aligned(
                            face,
                            row.columns[index].formatted_value,
                            box_rect,
                            header.halign,
                            header.valign);
                        break;
                    }
                    case pick_list_header_t::types::check_box: {
                        auto column_width = column_rect.width() / 6;
                        auto column_height = column_rect.height() / 2;
                        core::rect box_rect = {
                            column_rect.left() + ((column_rect.width() / 2) - column_width),
                            column_rect.top() + (column_height / 2),
                            column_width,
                            column_height
                        };

                        auto color = pal[header.bg_color];
                        adjust_color(color);
                        surface.set_color(color);
                        surface.draw_rect(box_rect);
                        auto flag = std::get<bool>(row.columns[index].value);
                        if (flag) {
                            box_rect.left(box_rect.left() + 4);
                            box_rect.top(box_rect.top() + 4);
                            box_rect.width(box_rect.width() - 8);
                            box_rect.height(box_rect.height() - 8);
                            surface.fill_rect(box_rect);
                        }
                        break;
                    }
                    case pick_list_header_t::types::button: {
                        auto value = row.columns[index].formatted_value;
                        if (!value.empty()) {
                            core::rect button_rect{
                                column_rect.left() + 8,
                                column_rect.top() + 2,
                                column_rect.width() - 24,
                                column_rect.height() - 2
                            };
                            auto color = pal[header.bg_color];
                            adjust_color(color);
                            surface.set_color(color);
                            surface.fill_rect(button_rect);
                            surface.set_color(fg);
                            surface.draw_rect(button_rect);

                            core::rect label_bounds{
                                button_rect.left(),
                                button_rect.top() + 2,
                                button_rect.width(),
                                button_rect.height() - 2
                            };
                            surface.set_font_color(font_face(), fg);
                            surface.draw_text_aligned(
                                font_face(),
                                value,
                                label_bounds,
                                halign_t::center,
                                valign_t::middle);
                        }
                        break;
                    }
                }

                if (header.type != pick_list_header_t::types::value) {
                    if (!row.editable_columns.empty()) {
                        auto column_index = row.editable_columns[row.active_column_index];
                        if (index == column_index) {
                            core::rect active_rect = {
                                column_rect.left(),
                                column_rect.top(),
                                column_rect.width() - 1,
                                column_rect.height() - 1
                            };
                            surface.draw_selection_band(active_rect, pal[header.fg_color]);
                        }
                    }
                }

                surface.pop_clip_rect();

                ++index;
                column_x += header.width + 2;
            }

            column_y += _row_height + row_height_margin;
        }

        core::rect footer_rect {
            bounds.left(),
            bounds.bottom() - _row_height,
            bounds.width(),
            _row_height
        };
        auto header_bg_color = pal[_headers[0].bg_color];
        auto header_fg_color = pal[_headers[0].fg_color];
        surface.set_color(adjust_color(header_bg_color));
        surface.set_font_color(face, adjust_color(header_fg_color));
        surface.fill_rect(footer_rect);

        footer_rect.deflate(4, 2);
        auto footer_string = fmt::format(
            "Row {:04d} | Page {:03d} of {:03d}",
            _row + 1,
            _page + 1,
            _max_page);
        auto footer_string_width = surface.measure_text(face, footer_string);
        surface.draw_text_aligned(
            face,
            footer_string,
            footer_rect,
            pick_list_header_t::halign_t::left,
            pick_list_header_t::valign_t::middle);

        if (_state == states::select_row) {
            if (!_search.empty()) {
                _caret.enabled(focused());

                core::rect type_ahead_rect{
                    bounds.left() + footer_string_width + 100,
                    bounds.bottom() - (_row_height - 2),
                    bounds.width() - (footer_string_width + 200),
                    _row_height - 4
                };
                surface.set_color(bg);
                surface.fill_rect(type_ahead_rect);

                _caret.padding().left(type_ahead_rect.left() - 8);
                _caret.padding().top(bounds.height() - _row_height);

                if (!_found)
                    surface.set_font_color(face, pal[_not_found_color]);
                else
                    surface.set_font_color(face, header_fg_color);
                surface.draw_text_aligned(
                    face,
                    _search,
                    type_ahead_rect,
                    alignment::horizontal::left,
                    alignment::vertical::middle);
                surface.set_font_color(face, fg);
            } else {
                _caret.enabled(false);
            }
        }

        surface.pop_clip_rect();
    }

    void column_pick_list::calculate_visible_and_max_rows() {
        _row_height = font_face()->line_height + 2;

        auto rows_rect = inner_bounds();
        rows_rect.height(rows_rect.height() - (_row_height * 2));

        _visible_rows = static_cast<uint32_t>(rows_rect.height() / (_row_height + row_height_margin));
        if (_visible_rows == 0)
            _visible_rows = 1;

        _max_rows = std::min<uint32_t>(
            _visible_rows,
            static_cast<uint32_t>(_rows.size()));

        _max_page = std::max<uint32_t>(
            1,
            static_cast<uint32_t>(_rows.size() / _visible_rows));
    }

    void column_pick_list::add_row(const pick_list_row_t& row) {
        _rows.push_back(row);

        update_state();

        // XXX: consider refactoring to support passing a full list so we only have
        //      to call this once.
        calculate_visible_and_max_rows();

        raise_selection_changed();
    }

    void column_pick_list::selection_color(palette_index value) {
        _selection_color = value;
    }

    void column_pick_list::not_found_color(palette_index value) {
        _not_found_color = value;
    }

    palette_entry column_pick_list::adjust_color(palette_entry entry) {
        if (!enabled() || !focused()) {
            return entry - 35;
        }
        return entry;
    }

    bool column_pick_list::find_matching_text(const std::string& text) {
        move_top();
        auto text_length = text.length();
        for (const auto& row : _rows) {
            for (const auto& column : row.columns) {
                if (column.formatted_value.substr(0, text_length) == text) {
                    return true;
                }
            }
            move_down();
        }
        return false;
    }

    void column_pick_list::raise_button_activated(uint32_t row, uint32_t column) {
        if (_button_activated_callable != nullptr)
            _button_activated_callable(row, column);
    }

    void column_pick_list::on_activated(const column_pick_list::activated_callable& callable) {
        _activated_callable = callable;
    }

    void column_pick_list::on_button_activated(const column_pick_list::button_activated_callable& callable) {
        _button_activated_callable = callable;
    }

    void column_pick_list::on_selection_changed(const column_pick_list::selection_changed_callable& callable) {
        _selection_changed_callable = callable;
    }

}