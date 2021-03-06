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
#include <hardware/registry.h>
#include <emulator/emulator_context.h>
#include "ide_types.h"
#include "ide_context.h"

namespace ryu::ide {

    std::map<core::state_transition_command, uint32_t> ide_context::s_state_transitions;

    ide_context::ide_context(const std::string& name) : core::context(name),
                                                        _hex_editor_state("hex editor"),
                                                        _console_state("console"),
                                                        _tile_editor_state("tile editor"),
                                                        _machine_list_state("machine list"),
                                                        _actor_editor_state("actor editor"),
                                                        _sprite_editor_state("sprite editor"),
                                                        _source_editor_state("text editor"),
                                                        _module_editor_state("module editor"),
                                                        _sample_editor_state("sample editor"),
                                                        _palette_editor_state("palette editor"),
                                                        _machine_editor_state("machine editor"),
                                                        _project_editor_state("project editor"),
                                                        _component_editor_state("component editor"),
                                                        _background_editor_state("background editor"),
                                                        _preferences_editor_state("preferences editor") {
    }

    void ide_context::define_actions() {
        auto toggle_context = core::input_action::create(
            "toggle_ide_context",
            "IDE",
            "Collapse, expand, and split the IDE context window.");
        if (!toggle_context->has_bindings()) {
            toggle_context->bind_keys({core::mod_alt, core::key_f1});
        }

        auto reload_state = core::input_action::create(
            "reload_state",
            "IDE",
            "Execute load on active state.");
        if (!reload_state->has_bindings()) {
            reload_state->bind_keys({core::mod_ctrl, core::key_r});
        }
    }

    void ide_context::bind_events() {
        action_provider().register_handler(
            core::input_action::find_by_name("reload_state"),
            [this](const core::event_data_t& data) {
                auto active_state = find_state(peek_state());
                if (active_state != nullptr) {
                    core::result result;
                    if (!active_state->load(result)) {
                        // XXX: add in dialog/error pop up when ready
                    } else {
                        active_state->activate({});
                    }
                }
                return true;
            });

        action_provider().register_handler(
            core::input_action::find_by_name("toggle_ide_context"),
            [this](const core::event_data_t& data) {
                auto emulator_context = dynamic_cast<emulator::emulator_context*>(engine()->find_context("emulator"));
                switch (_size) {
                    case core::context_window::split:
                        size(core::context_window::expanded);
                        emulator_context->size(core::context_window::collapsed);
                        break;
                    case core::context_window::expanded:
                    case core::context_window::collapsed:
                        size(core::context_window::split);
                        emulator_context->size(core::context_window::split);
                        break;
                    default:
                        break;
                }
                return true;
            });
    }

    void ide_context::configure_states(core::result& result) {
        add_state(
            result,
            &_console_state,
            std::bind(&ide_context::execute_transition_command, this, std::placeholders::_1, std::placeholders::_2));
        add_state(
            result,
            &_machine_list_state,
            std::bind(&ide_context::execute_transition_command, this, std::placeholders::_1, std::placeholders::_2));
        add_state(result, &_source_editor_state);
        add_state(
            result,
            &_machine_editor_state,
            std::bind(&ide_context::execute_transition_command, this, std::placeholders::_1, std::placeholders::_2));
        add_state(result, &_source_editor_state);
        add_state(result, &_hex_editor_state);
        add_state(result, &_tile_editor_state);
        add_state(result, &_actor_editor_state);
        add_state(result, &_module_editor_state);
        add_state(result, &_sample_editor_state);
        add_state(result, &_sprite_editor_state);
        add_state(result, &_project_editor_state);
        add_state(result, &_palette_editor_state);
        add_state(result, &_component_editor_state);
        add_state(result, &_background_editor_state);
        add_state(result, &_preferences_editor_state);
        push_state(_console_state.id(), {});
    }

    void ide_context::configure_palette() {
        auto& pal = palette();

        auto& black = pal[ide::colors::black];
        black.red(0x00);
        black.green(0x00);
        black.blue(0x00);
        black.alpha(0xff);

        auto& white = pal[ide::colors::white];
        white.red(0xff);
        white.green(0xff);
        white.blue(0xff);
        white.alpha(0xff);

        auto& grey = pal[ide::colors::grey];
        grey.red(0x78);
        grey.green(0x78);
        grey.blue(0x78);
        grey.alpha(0xff);

        auto& blue = pal[ide::colors::blue];
        blue.red(0x40);
        blue.green(0x31);
        blue.blue(0x8c);
        blue.alpha(0xff);

        auto& red = pal[ide::colors::red];
        red.red(0x88);
        red.green(0x33);
        red.blue(0x32);
        red.alpha(0xff);

        auto& cyan = pal[ide::colors::cyan];
        cyan.red(0x67);
        cyan.green(0xb6);
        cyan.blue(0xbd);
        cyan.alpha(0xff);

        auto& purple = pal[ide::colors::purple];
        purple.red(0x8b);
        purple.green(0x3f);
        purple.blue(0x97);
        purple.alpha(0xff);

        auto& green = pal[ide::colors::green];
        green.red(0x57);
        green.green(0xa0);
        green.blue(0x49);
        green.alpha(0xff);

        auto& yellow = pal[ide::colors::yellow];
        yellow.red(0xbf);
        yellow.green(0xce);
        yellow.blue(0x73);
        yellow.alpha(0xff);

        auto& orange = pal[ide::colors::orange];
        orange.red(0x8b);
        orange.green(0x55);
        orange.blue(0x29);
        orange.alpha(0xff);

        auto& brown = pal[ide::colors::brown];
        brown.red(0x57);
        brown.green(0x42);
        brown.blue(0x00);
        brown.alpha(0xff);

        auto& pink = pal[ide::colors::pink];
        pink.red(0xb8);
        pink.green(0x69);
        pink.blue(0x62);
        pink.alpha(0xff);

        auto& dark_grey = pal[ide::colors::dark_grey];
        dark_grey.red(0x50);
        dark_grey.green(0x50);
        dark_grey.blue(0x50);
        dark_grey.alpha(0xff);

        auto& light_green = pal[ide::colors::light_green];
        light_green.red(0x95);
        light_green.green(0xe0);
        light_green.blue(0x88);
        light_green.alpha(0xff);

        auto& light_blue = pal[ide::colors::light_blue];
        light_blue.red(0x78);
        light_blue.green(0x69);
        light_blue.blue(0xc4);
        light_blue.alpha(0xff);

        auto& light_grey = pal[ide::colors::light_grey];
        light_grey.red(0x9f);
        light_grey.green(0x9f);
        light_grey.blue(0x9f);
        light_grey.alpha(0xff);

        auto& transparent = pal[ide::colors::transparent];
        transparent.red(0x00);
        transparent.green(0x00);
        transparent.blue(0x00);
        transparent.alpha(0x00);
    }

    void ide_context::on_draw(core::renderer& surface) {
        if (size() == core::context_window::sizes::expanded)
            return;

        const int16_t tab_width = 32;
        const int16_t tab_slope = 16;
        const int16_t tab_height = 128;
        const int16_t offset_height = 64;

        auto rect = bounds();
        auto middle = static_cast<int16_t>((rect.height() / 2) - (tab_height + offset_height));
        core::vertex_list tab_vertices = {
            {static_cast<int16_t>(rect.right() + tab_width), static_cast<int16_t>(middle)},
            {static_cast<int16_t>(rect.right()),             static_cast<int16_t>(middle - tab_slope)},
            {static_cast<int16_t>(rect.right()),             static_cast<int16_t>(middle + tab_height + tab_slope)},
            {static_cast<int16_t>(rect.right() + tab_width), static_cast<int16_t>(middle + tab_height)},
            {static_cast<int16_t>(rect.right() + tab_width), static_cast<int16_t>(middle)},
        };

        auto& pal = palette();
        surface.set_color(pal[colors::fill_color]);
        surface.fill_polygon(tab_vertices);
        surface.set_color(pal[colors::black]);
        surface.draw_line(
                rect.right(),
                middle - tab_slope,
                rect.right() + tab_width,
                middle);
        surface.draw_line(
                rect.right() + tab_width,
                middle + tab_height,
                rect.right() + tab_width,
                middle);
        surface.draw_line(
                rect.right() + tab_width,
                middle + tab_height,
                rect.right(),
                middle + tab_height + tab_slope);
    }

    bool ide_context::on_initialize(core::result& result) {
        s_state_transitions = {
            {core::system_commands::edit_source, _source_editor_state.id()},
            {core::system_commands::edit_memory, _hex_editor_state.id()},
            {core::system_commands::list_machine, _machine_list_state.id()},
            {core::system_commands::edit_tiles, _tile_editor_state.id()},
            {core::system_commands::edit_sprites, _sprite_editor_state.id()},
            {core::system_commands::edit_backgrounds, _background_editor_state.id()},
            {core::system_commands::edit_music, _module_editor_state.id()},
            {core::system_commands::edit_sounds, _sample_editor_state.id()},
            {core::system_commands::edit_palette, _palette_editor_state.id()},
            {core::system_commands::edit_actor, _actor_editor_state.id()},
            {core::system_commands::edit_component, _component_editor_state.id()},
            {core::system_commands::edit_machine, _machine_editor_state.id()},
            {core::system_commands::edit_project, _project_editor_state.id()},
        };

        define_actions();
        bind_events();
        configure_palette();
        configure_states(result);
        parent_resize(bounds());
        return true;
    }

    core::context_window::sizes ide_context::size() const {
        return _size;
    }

    void ide_context::size(core::context_window::sizes value) {
        if (value != _size) {
            _size = value;
            engine()->raise_resize();
        }
    }

    void ide_context::parent_resize(const core::rect& parent_bounds) {
        switch (_size) {
            case core::context_window::split:
                bounds({
                    0,
                    0,
                    (parent_bounds.width() / 2) - 1,
                    parent_bounds.height()
                });
                break;
            case core::context_window::expanded:
                bounds({
                    0,
                    0,
                    parent_bounds.width() - 16,
                    parent_bounds.height()
                });
                break;
            case core::context_window::collapsed:
                bounds({
                    0,
                    0,
                    16,
                    parent_bounds.height()
                });
                break;
        }
    }

    bool ide_context::execute_transition_command(
            core::state_transition_command command,
            const core::parameter_dict& params) {
        auto it = s_state_transitions.find(command);
        if (it == s_state_transitions.end())
            return false;
        push_state(it->second, params);
        return true;
    }

}