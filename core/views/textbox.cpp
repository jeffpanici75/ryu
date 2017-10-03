//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <sstream>
#include "textbox.h"

namespace ryu::core {

    textbox::textbox(const std::string& name) : core::view(types::control, name),
                                                _caret("textbox-caret") {
    }

    void textbox::clear() {
        _document.clear();
        caret_home();
    }

    void textbox::caret_end() {
        auto line_end = _document.find_line_end(0);
        _caret.column(static_cast<uint8_t>(line_end));
    }

    void textbox::caret_home() {
        _caret.column(0);
    }

    std::string textbox::value() {
        std::stringstream stream;
        _document.write_line(stream, 0, 0, _page_width);
        return stream.str();
    }

    void textbox::on_focus_changed() {
        _caret.enabled(focused());
    }

    bool textbox::caret_left(uint8_t columns) {
        return _caret.left(columns);
    }

    bool textbox::caret_right(uint8_t columns) {
        return _caret.right(columns);
    }

    void textbox::value(const std::string& value) {
        std::stringstream stream(value);
        _document.load(stream);
    }

    void textbox::on_draw(core::renderer& surface) {
        auto bounds = client_bounds();

        auto pal = *palette();
        auto fg = pal[fg_color()];
        auto& bg = pal[bg_color()];

        if (!enabled() || !focused()) {
            fg = fg - 35;
        }

        surface.set_color(bg);
        surface.fill_rect(bounds);

        surface.set_color(fg);
        surface.set_font_color(font_face(), fg);

        std::stringstream stream;
        _document.write_line(stream, 0, 0, _page_width);

        surface.draw_text_aligned(
            font_face(),
            stream.str(),
            bounds,
            alignment::horizontal::left,
            alignment::vertical::middle);
        surface.draw_line(
            bounds.left(),
            bounds.bottom(),
            bounds.right() + 5,
            bounds.bottom());
    }

    void textbox::size(uint8_t rows, uint8_t columns) {
        _document.page_size(rows, columns);
        _caret.page_size(rows, columns);
        _page_width = columns;
        on_resize({});
    }

    bool textbox::on_process_event(const SDL_Event* e) {
        if (e->type == SDL_TEXTINPUT) {
            const char* c = &e->text.text[0];
            while (*c != '\0') {
                if (_on_key_down != nullptr) {
                    if (!_on_key_down(*c)) {
                        c++;
                        continue;
                    }
                }

                _document.put(
                        0,
                        _caret.column(),
                        core::element_t {static_cast<uint8_t>(*c), _document.default_attr()});

                if (caret_right())
                    break;

                c++;
            }
        } else if (e->type == SDL_KEYDOWN) {
            switch (e->key.keysym.sym) {
                case SDLK_HOME: {
                    caret_home();
                    break;
                }
                case SDLK_END: {
                    caret_end();
                    break;
                }
                case SDLK_RIGHT: {
                    caret_right();
                    break;
                }
                case SDLK_LEFT: {
                    caret_left();
                    break;
                }
                case SDLK_DELETE: {
                    _document.shift_left(0, _caret.column());
                    return true;
                }
                case SDLK_BACKSPACE: {
                    if (_caret.column() == 0) {
                        _document.delete_line(0);
                    } else {
                        caret_left();
                        _document.shift_left(0, _caret.column());
                    }
                    return true;
                }
            }

            if (_on_key_down != nullptr)
                return _on_key_down(e->key.keysym.sym);
        }
        return false;
    }

    void textbox::initialize(uint8_t rows, uint8_t columns) {
        _document.initialize(rows, columns);
        _document.clear();

        _caret.font_family(font_family());
        _caret.fg_color(fg_color());
        _caret.palette(palette());
        _caret.initialize(0, 0);
        _caret.enabled(false);
        add_child(&_caret);

        padding({5, 5, 5, 5});
        size(rows, columns);
    }

    void textbox::on_resize(const core::rect& context_bounds) {
        bounds().size(font_face()->width * (_page_width + 1), font_face()->line_height + 10);
    }

    void textbox::on_key_down(const textbox::on_key_down_callable& callable) {
        _on_key_down = callable;
    }

}