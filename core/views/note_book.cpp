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
#include "note_book.h"

namespace ryu::core {

    note_book::note_book(
            const std::string& name,
            core::view_host* host) : core::view(types::control, name, host) {
    }

    note_book::~note_book() {
    }

    void note_book::define_actions() {
        auto prev_tab_action = core::input_action::create_no_map(
                "notebook_previous_tab",
                "Internal",
                "Move to the previous note book tab.");
        if (!prev_tab_action->has_bindings())
            prev_tab_action->bind_keys({core::key_up});

        auto next_tab_action = core::input_action::create_no_map(
                "notebook_next_tab",
                "Internal",
                "Move to the next note book tab.");
        if (!next_tab_action->has_bindings())
            next_tab_action->bind_keys({core::key_down});
    }

    void note_book::bind_events() {
        action_provider().register_handler(
                core::input_action::find_by_name("notebook_previous_tab"),
                [this](const event_data_t& data) {
                    _index--;
                    if (_index < 0)
                        _index = 0;
                    return true;
                });
        action_provider().register_handler(
                core::input_action::find_by_name("notebook_next_tab"),
                [this](const event_data_t& data) {
                    if (_index + 1 < _tabs.size())
                        _index++;
                    return true;
                });
    }

    void note_book::on_initialize() {
        define_actions();
        bind_events();

        tab_stop(true);

        auto& minimum_size = min_size();
        minimum_size.dimensions(256, 256);
    }

    size_t note_book::active_tab() const {
        return _index;
    }

    void note_book::active_tab(size_t index) {
        _index = index;
    }

    void note_book::remove_tab(size_t index) {
        _tabs.erase(_tabs.begin() + index);
    }

    void note_book::on_font_family_changed() {
        calculate_tab_and_page_sizes();
    }

    void note_book::calculate_tab_and_page_sizes() {
        // XXX: need to implement this logic and pull out of on_draw
    }

    void note_book::on_draw(core::renderer& surface) {
        // XXX: rework how the tab buttons are included so we can
        //      take advantage of the layout_engine

        auto& rect = bounds();
        auto& pads = padding();

        auto pal = *palette();
        auto& fg = pal[fg_color()];
        auto& bg = pal[bg_color()];

        if (!enabled() || !focused())
            fg = fg - 35;

        surface.set_color(fg);

        size_t index = 0;
        auto y = rect.top() + pads.top();
        auto x = rect.left() + pads.left();
        for (const auto& tab : _tabs) {
            core::rect tab_rect {x, y, 250, 32};

            if (index == _index) {
                surface.set_font_color(font_face(), bg);
                surface.fill_rect(tab_rect);
            } else {
                surface.set_font_color(font_face(), fg);
                surface.draw_rect(tab_rect);
            }

            auto child = get_child_at(index);
            if (child != nullptr)
                child->visible(index == _index);

            surface.draw_text_aligned(
                    font_face(),
                    tab,
                    tab_rect,
                    alignment::horizontal::center,
                    alignment::vertical::middle);

            y += 31;
            index++;
        }

        auto content_rect = bounds();
        content_rect.pos(rect.left() + 249, rect.top())
                    .size(rect.width() - 251, rect.height());
        surface.draw_rect(content_rect);
    }

    void note_book::add_tab(const std::string& title, core::view* content) {
        _tabs.push_back(title);
        if (content != nullptr) {
            content->visible(false);
            add_child(content);
        }
    }

}