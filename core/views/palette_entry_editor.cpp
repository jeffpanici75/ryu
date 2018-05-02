//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <common/bytes.h>
#include "palette_entry_editor.h"

namespace ryu::core {

    palette_entry_editor::palette_entry_editor(
        const std::string& name,
        core::view_host* host) : core::view(core::view::types::control, name, host),
                                 _caret("palette-entry-editor-caret", host) {
    }

    palette_entry_editor::~palette_entry_editor() {
    }

    // 0123456789012345678
    // R:00 G:00 B:00 A:00
    void palette_entry_editor::caret_left() {
        if (_caret.left(1)) {
            _caret.column(19);
        } else {
            switch (_caret.column()) {
                case 1:
                    _caret.column(18);
                    break;
                case 16:
                    _caret.column(13);
                    break;
                case 11:
                    _caret.column(8);
                    break;
                case 6:
                    _caret.column(3);
                    break;
                default:
                    break;
            }
        }
    }

    void palette_entry_editor::bind_events() {
        action_provider().register_handler(
            core::input_action::find_by_name("palette_entry_editor_previous_palette"),
            [this](const core::event_data_t& data) {
                if (_locked)
                    return false;
                if (_index > 0)
                    --_index;
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("palette_entry_editor_next_palette"),
            [this](const core::event_data_t& data) {
                if (_locked)
                    return false;
                if (_index < 255)
                    ++_index;
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("palette_entry_editor_caret_left"),
            [this](const core::event_data_t& data) {
                caret_left();
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("palette_entry_editor_caret_right"),
            [this](const core::event_data_t& data) {
                caret_right();
                return true;
            });
        action_provider().register_handler(
            core::input_action::find_by_name("palette_entry_editor_input"),
            [this](const core::event_data_t& data) {
                if (data.c == core::ascii_escape)
                    return false;

                if (!isxdigit(data.c))
                    return false;

                auto nybble = static_cast<uint8_t>(data.c > '9' ?
                                                   (data.c & ~0x20) - 'A' + 10 :
                                                   data.c - '0');

                auto& palette_entry = palette()->get(_index);
                switch (_caret.column()) {
                    case 2: {
                        auto value = palette_entry.red();
                        value = set_upper_nybble(value, nybble);
                        palette_entry.red(value);
                        break;
                    }
                    case 3: {
                        auto value = palette_entry.red();
                        value = set_lower_nybble(value, nybble);
                        palette_entry.red(value);
                        break;
                    }
                    case 7: {
                        auto value = palette_entry.green();
                        value = set_upper_nybble(value, nybble);
                        palette_entry.green(value);
                        break;
                    }
                    case 8: {
                        auto value = palette_entry.green();
                        value = set_lower_nybble(value, nybble);
                        palette_entry.green(value);
                        break;
                    }
                    case 12: {
                        auto value = palette_entry.blue();
                        value = set_upper_nybble(value, nybble);
                        palette_entry.blue(value);
                        break;
                    }
                    case 13: {
                        auto value = palette_entry.blue();
                        value = set_lower_nybble(value, nybble);
                        palette_entry.blue(value);
                        break;
                    }
                    case 17: {
                        auto value = palette_entry.alpha();
                        value = set_upper_nybble(value, nybble);
                        palette_entry.alpha(value);
                        break;
                    }
                    case 18: {
                        auto value = palette_entry.alpha();
                        value = set_lower_nybble(value, nybble);
                        palette_entry.alpha(value);
                        break;
                    }
                    default:
                        break;
                }

                caret_right();

                return true;
            });
    }

    void palette_entry_editor::on_initialize() {
        tab_stop(true);
        define_actions();
        bind_events();

        _caret.index(1001);
        _caret.enabled(false);
        _caret.position(0, 2);
        _caret.page_size(1, 19);
        _caret.padding().left(84);
        _caret.padding().top(16);
        _caret.palette(palette());
        _caret.fg_color(fg_color());
        _caret.bg_color(bg_color());
        _caret.font_family(font_family());
        _caret.initialize();
        add_child(&_caret);
    }

    bool palette_entry_editor::locked() const {
        return _locked;
    }

    void palette_entry_editor::define_actions() {
        auto previous_palette_action = core::input_action::create_no_map(
            "palette_entry_editor_previous_palette",
            "Internal",
            "Change to the previous palette.");
        if (!previous_palette_action->has_bindings())
            previous_palette_action->bind_keys({core::key_down});

        auto next_palette_action = core::input_action::create_no_map(
            "palette_entry_editor_next_palette",
            "Internal",
            "Change to the next palette.");
        if (!next_palette_action->has_bindings())
            next_palette_action->bind_keys({core::key_up});

        auto move_left = core::input_action::create_no_map(
            "palette_entry_editor_caret_left",
            "Internal",
            "Move the caret left.");
        if (!move_left->has_bindings())
            move_left->bind_keys({core::key_left});

        auto move_right = core::input_action::create_no_map(
            "palette_entry_editor_caret_right",
            "Internal",
            "Move the caret right.");
        if (!move_right->has_bindings())
            move_right->bind_keys({core::key_right});

        auto text_input_action = core::input_action::create_no_map(
            "palette_entry_editor_input",
            "Internal",
            "Any hexadecimal input (non-mappable).");
        if (!text_input_action->has_bindings())
            text_input_action->bind_text_input();
    }

    palette_index palette_entry_editor::entry() {
        return _index;
    }

    void palette_entry_editor::locked(bool value) {
        _locked = value;
    }

    void palette_entry_editor::on_focus_changed() {
        _caret.enabled(focused());
    }

    void palette_entry_editor::entry(palette_index value) {
        _index = value;
    }

    void palette_entry_editor::text_color(palette_index value) {
        _text_color = value;
        _caret.fg_color(value);
    }

    void palette_entry_editor::on_draw(core::renderer& surface) {
        auto bounds = client_bounds();
        auto pal = *palette();
        auto fg = pal[fg_color()];
        auto bg = pal[bg_color()];
        auto entry = pal[_index];
        auto text_color = pal[_text_color];

        surface.push_blend_mode(SDL_BLENDMODE_BLEND);

        if (!enabled()) {
            fg = fg - 45;
            bg = bg - 45;
        } else if (!focused()) {
            fg = fg - 35;
            bg = bg - 35;
        }

        surface.set_color(entry);
        surface.fill_rect(bounds);

        surface.set_color(fg);
        surface.draw_rect(bounds);
        surface.set_font_color(font_face(), text_color);
        surface.draw_text_aligned(
            font_face(),
            fmt::format("{:02x}", _index),
            bounds,
            alignment::horizontal::center,
            alignment::vertical::middle);

        surface.draw_text(
            font_face(),
            bounds.left() + 84,
            bounds.top() + 16,
            fmt::format(
                "R:{:02x} G:{:02x} B:{:02x} A:{:02x}",
                entry.red(),
                entry.green(),
                entry.blue(),
                entry.alpha()),
            text_color);

        surface.pop_blend_mode();
    }

    void palette_entry_editor::caret_right(uint8_t overflow_column) {
        if (_caret.right(1)) {
            _caret.column(overflow_column);
        } else {
            auto column = _caret.column() + 1;
            if (column % 5 == 0) {
                if (_caret.right(3))
                    _caret.column(overflow_column);
            }
        }
    }

    void palette_entry_editor::on_resize(const core::rect& context_bounds) {
        switch (sizing()) {
            case sizing::fixed: {
                bounds().width(64);
                bounds().height(64);
                break;
            }
            case sizing::parent: {
                auto container = parent();
                bounds(container != nullptr ? container->bounds() : context_bounds);
                break;
            }
            default: {
                break;
            }
        }
    }

}