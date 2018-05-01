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
#include "pick_list.h"

namespace ryu::core {

    pick_list::pick_list(
            const std::string& name,
            core::view_host* host) : core::view(types::control, name, host),
                                     _caret("text-box-caret", host) {
    }

    pick_list::~pick_list() {
    }

    bool pick_list::page_up() {
        if (_visibile_items > _row) {
            _row = 0;
            return true;
        }
        else
            _row -= _visibile_items;
        return false;
    }

    bool pick_list::move_up() {
        _selected_item--;
        if (_selected_item < 0) {
            _selected_item = 0;
            return move_row_up();
        }
        return false;
    }

    void pick_list::move_top() {
        _row = 0;
        _selected_item = 0;
    }

    bool pick_list::move_down() {
        _selected_item++;
        if (_selected_item > _visibile_items - 1) {
            _selected_item = _visibile_items - 1;
            return move_row_down();
        }
        return false;
    }

    bool pick_list::page_down() {
        auto next_stop_row = _row + _visibile_items;
        if (next_stop_row > _options.size() - 1) {
            _row = static_cast<int>(_options.size() - _visibile_items);
            return true;
        }
        else
            _row += _visibile_items;
        return false;
    }

    int pick_list::width() const {
        return _width;
    }

    int pick_list::height() const {
        return _height;
    }

    void pick_list::define_actions() {
        auto up_action = core::input_action::create_no_map(
            "pick_list_up_action",
            "Internal",
            "Move to the previous pick list item.");
        if (!up_action->has_bindings())
            up_action->bind_keys({core::key_up});

        auto page_up_action = core::input_action::create_no_map(
            "pick_list_page_up_action",
            "Internal",
            "Move to the previous item page.");
        if (!page_up_action->has_bindings())
            page_up_action->bind_keys({core::key_page_up});

        auto down_action = core::input_action::create_no_map(
            "pick_list_down_action",
            "Internal",
            "Move to the next pick list item.");
        if (!down_action->has_bindings())
            down_action->bind_keys({core::key_down});

        auto page_down_action = core::input_action::create_no_map(
            "pick_list_page_down_action",
            "Internal",
            "Move to the next item page.");
        if (!page_down_action->has_bindings())
            page_down_action->bind_keys({core::key_page_down});

        auto select_action = core::input_action::create_no_map(
            "pick_list_select_action",
            "Internal",
            "Make current item the selected value.");
        if (!select_action->has_bindings())
            select_action->bind_keys({core::key_return});

        auto backspace_action = core::input_action::create_no_map(
            "pick_list_backspace_action",
            "Internal",
            "Delete a character from the typeahead search.");
        if (!backspace_action->has_bindings()) {
            backspace_action->bind_keys({core::key_backspace});
        }

        auto delete_action = core::input_action::create_no_map(
            "pick_list_delete_action",
            "Internal",
            "Reset the typeahead search.");
        if (!delete_action->has_bindings()) {
            delete_action->bind_keys({core::key_delete});
        }

        auto text_input_action = core::input_action::create_no_map(
            "pick_list_text_input",
            "Internal",
            "Any ASCII text input (non-mappable).");
        if (!text_input_action->has_bindings()) {
            text_input_action->bind_text_input();
        }
    }

    void pick_list::bind_events() {
        action_provider().register_handler(
            core::input_action::find_by_name("pick_list_up_action"),
            [this](const event_data_t& data) {
                move_up();
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("pick_list_page_up_action"),
            [this](const event_data_t& data) {
                page_up();
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("pick_list_down_action"),
            [this](const event_data_t& data) {
                move_down();
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("pick_list_page_down_action"),
            [this](const event_data_t& data) {
                page_down();
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("pick_list_select_action"),
            [this](const event_data_t& data) {
                _search.clear();
                _caret.column(0);
                _selection = _row + _selected_item;
                value(_options[_selection].text);
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("pick_list_backspace_action"),
            [this](const event_data_t& data) {
                move_top();
                _search = _search.substr(0, _search.length() - 1);
                _caret.column(static_cast<uint8_t>(_search.length()));
                _found = find_matching_text(_search);
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("pick_list_delete_action"),
            [this](const event_data_t& data) {
                move_top();
                _search.clear();
                _caret.column(0);
                _found = false;
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("pick_list_text_input"),
            [this](const core::event_data_t& data) {
                if (data.c == core::ascii_tab
                ||  data.c == core::ascii_return
                ||  data.c == core::ascii_escape) {
                    return false;
                }

                if (_search.length() < 32) {
                    _search += data.c;
                    _caret.column(static_cast<uint8_t>(_search.length()));
                    _found = find_matching_text(_search);
                }

                return true;
            });
    }

    bool pick_list::move_row_up() {
        _row--;
        if (_row < 0) {
            _row = 0;
            return true;
        }
        return false;
    }

    int pick_list::length() const {
        return _length;
    }

    void pick_list::on_initialize() {
        tab_stop(true);
        define_actions();
        bind_events();

        _caret.index(1001);
        _caret.enabled(false);
        _caret.position(0, 0);
        _caret.page_size(1, 128);
        _caret.fg_color(fg_color());
        _caret.bg_color(bg_color());
        _caret.font_family(font_family());
        _caret.initialize();
        add_child(&_caret);

        padding({5, 5, 5, 5});
    }

    bool pick_list::move_row_down() {
        if (_options.size() < _visibile_items)
            return false;
        _row++;
        auto max = _options.size() - _visibile_items;
        if (_row > max) {
            _row = static_cast<int>(max);
            return true;
        }
        return false;
    }

    void pick_list::width(int value) {
        _width = value;
    }

    void pick_list::height(int value) {
        _height = value;
    }

    void pick_list::length(int value) {
        _length = value;
        requires_layout();
    }

    option_list& pick_list::options() {
        return _options;
    }

    void pick_list::on_focus_changed() {
        _caret.enabled(focused());
        if (focused()) {
            index(1000);
        } else {
            index(0);
        }
    }

    std::string pick_list::value() const {
        return view::value();
    }

    int pick_list::visible_items() const {
        return _visibile_items;
    }

    border::types pick_list::border() const {
        return _border;
    }

    void pick_list::visible_items(int value) {
        _visibile_items = value;
    }

    void pick_list::selected_key(uint32_t key) {
        auto it = std::find_if(
            _options.begin(),
            _options.end(),
            [&key](const pick_list_option_t& o) { return o.key == key; });
        if (it != _options.end()) {
            view::value((*it).text);
            _selection = static_cast<int>(std::distance(_options.begin(), it));
            find_matching_text(value());
        }
    }

    void pick_list::border(border::types value) {
        _border = value;
    }

    void pick_list::value(const std::string& text) {
        view::value(text);

        if (!text.empty()) {
            auto it = std::find_if(
                _options.begin(),
                _options.end(),
                [&text](const pick_list_option_t& o) { return o.text == text; });
            if (it != _options.end()) {
                _selection = static_cast<int>(std::distance(_options.begin(), it));
                find_matching_text(value());
            }
        }
    }

    void pick_list::row_color(palette_index value) {
        _row_color = value;
    }

    void pick_list::on_draw(core::renderer& surface) {
        auto bounds = client_bounds();

        auto pal = *palette();
        auto fg = pal[fg_color()];
        auto bg = pal[bg_color()];

        if (!enabled() || !focused()) {
            fg = fg - 35;
        }

        surface.set_color(bg);
        surface.fill_rect(bounds);

        surface.set_font_color(font_face(), fg);
        surface.set_color(fg);

        if (_search.empty()) {
            surface.draw_text_aligned(
                font_face(),
                value(),
                bounds,
                alignment::horizontal::left,
                alignment::vertical::middle);
        } else {
            // XXX: this shouldn't be hard coded
            if (!_found)
                surface.set_font_color(font_face(), pal[_not_found_color]);
            surface.draw_text_aligned(
                font_face(),
                _search,
                bounds,
                alignment::horizontal::left,
                alignment::vertical::middle);
            surface.set_font_color(font_face(), fg);
        }

        if (_border == border::types::solid) {
            surface.set_color(fg);
            surface.draw_rect(bounds);
        } else {
            surface.draw_line(
                bounds.left(),
                bounds.bottom(),
                bounds.right() + 5,
                bounds.bottom());
        }

        if (!focused())
            return;

        auto height = _height > 0 ?
                      _height :
                      font_face()->line_height * (_visibile_items + 1);
        auto width = _width > 0 ? _width : bounds.width();
        core::rect box {bounds.left(), bounds.bottom(), width + 6, height};
        surface.set_color(bg);
        surface.fill_rect(box);
        surface.set_color(fg);
        surface.draw_rect(box);

        auto y = box.top() + 4;
        auto start = _row;
        auto stop = start + _visibile_items;
        if (stop > _options.size())
            stop = static_cast<int>(_options.size());
        for (auto row = start; row < stop; ++row) {
            core::rect line = {
                bounds.left() + 4,
                y,
                bounds.width() - 2,
                font_face()->line_height};
            if (row == _selection) {
                surface.draw_selection_rect(line, pal[_selection_color]);
            } else if (row == _row + _selected_item) {
                surface.draw_selection_rect(line, pal[_row_color]);
            }
            surface.draw_text_aligned(
                font_face(),
                _options[row].text,
                line,
                alignment::horizontal::left,
                alignment::vertical::middle);
            y += font_face()->line_height + 1;
        }
    }

    void pick_list::selection_color(palette_index value) {
        _selection_color = value;
    }

    void pick_list::not_found_color(palette_index value) {
        _not_found_color = value;
    }

    void pick_list::on_resize(const core::rect& context_bounds) {
        bounds().size(
            font_face()->width * (_length + 1),
            font_face()->line_height + 10);
    }

    bool pick_list::find_matching_text(const std::string& text) {
        move_top();
        auto text_length = text.length();
        for (const auto& option : _options) {
            if (option.text.substr(0, text_length) == text) {
                return true;
            }
            move_down();
        }
        return false;
    }

}