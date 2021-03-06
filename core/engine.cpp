//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//

#include <SDL.h>
#include <algorithm>
#include <fmt/format.h>
#include <SDL_FontCache.h>
#include <common/string_support.h>
#include "state.h"
#include "engine.h"
#include "context.h"
#include "font_book.h"
#include "joysticks.h"
#include "timer_pool.h"
#include "preferences.h"
#include "input_action.h"

namespace ryu::core {

    using event_list = std::vector<SDL_Event>;

    std::vector<rect> engine::displays() {
        std::vector<rect> displays;

        auto display_count = SDL_GetNumVideoDisplays();
        for (auto i = 0; i < display_count; i++) {
            SDL_Rect bounds{};
            SDL_GetDisplayBounds(i, &bounds);
            displays.emplace_back(bounds);
        }

        return displays;
    }

    void engine::quit() {
        _quit = true;
    }

    bool engine::shutdown(
            core::result& result,
            core::preferences& prefs) {
        prefs.window_position(_window_rect);
        if (_font_family != nullptr) {
            prefs.engine_font(std::make_pair(
                    _font_family->name(),
                    _font_family->size()));
        }

        joysticks::instance()->shutdown();

        if (_renderer != nullptr)
            SDL_DestroyRenderer(_renderer);

        if (_window != nullptr)
            SDL_DestroyWindow(_window);

        SDL_Quit();

        return !result.is_failed();
    }

    bool engine::initialize(
            core::result& result,
            core::preferences* prefs) {
        _prefs = prefs;

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            result.add_message(
                    "R002",
                    "SDL_Init failed.",
                    fmt::format("SDL Error: {0}", SDL_GetError()),
                    true);
            return false;
        }

        _window_rect = _prefs->window_position();
        _clip_rect = {0, 0, _window_rect.width(), _window_rect.height()};

        _window = SDL_CreateWindow(
                "Ryu: The Arcade Construction Kit",
                _window_rect.left(),
                _window_rect.top(),
                _window_rect.width(),
                _window_rect.height(),
                SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
        if (_window == nullptr) {
            result.add_message(
                    "R003",
                    "SDL_CreateWindow failed.",
                    fmt::format("SDL Error: {0}", SDL_GetError()),
                    true);
            return false;
        }

        int x, y;
        SDL_GetWindowPosition(_window, &x, &y);
        _window_rect.left(x);
        _window_rect.top(y);

        SDL_SetWindowMinimumSize(_window, 1280, 1024);
        SDL_SetWindowOpacity(_window, 1.0f);
        _renderer = SDL_CreateRenderer(
                _window,
                -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (_renderer == nullptr) {
            result.add_message(
                    "R004",
                    "SDL_CreateRenderer failed.",
                    fmt::format("SDL Error: {0}", SDL_GetError()),
                    true);
            return false;
        }

        _surface.sdl_renderer(_renderer);
        _surface.set_clip_rect(_clip_rect);

        font_book::instance()->renderer(_renderer);
        font_book::instance()->load(result, _prefs->font_book_path());

        const auto& engine_font = _prefs->engine_font();
        auto family = font_book::instance()->find_font_family(
                engine_font.first,
                engine_font.second);
        if (family != nullptr) {
            font_family(family);
            auto face = family->find_style(font::styles::normal);
            if (face != nullptr) {
                font_face(face);
            }
        }

        joysticks::instance()->initialize();

        define_actions();
        bind_events();

        return true;
    }

    void engine::raise_move() {
        if (_move_callback != nullptr) {
            _move_callback(_window_rect);
        }
    }

    void engine::focus(int id) {
        _focused_context = id;
    }

    void engine::define_actions() {
        auto quit_action = input_action::create_no_map(
            "ryu_quit",
            "Ryu",
            "Quit the Ryu application.",
            input_action::type::system);
        if (!quit_action->has_bindings())
            quit_action->bind_quit();

        auto minimized_action = input_action::create_no_map(
            "ryu_minimized",
            "Ryu",
            "Minimize the Ryu application window.",
            input_action::type::window);
        if (!minimized_action->has_bindings())
            minimized_action->bind_minimized();

        auto maximized_action = input_action::create_no_map(
            "ryu_maximized",
            "Ryu",
            "Maximize the Ryu application window.",
            input_action::type::window);
        if (!maximized_action->has_bindings())
            maximized_action->bind_maximized();

        auto move_action = input_action::create_no_map(
            "ryu_moved",
            "Ryu",
            "The Ryu application window was moved.",
            input_action::type::window,
            input_action::flag::bubble);
        if (!move_action->has_bindings())
           move_action->bind_move();

        auto resize_action = input_action::create_no_map(
            "ryu_resized",
            "Ryu",
            "The Ryu application window was resized.",
            input_action::type::window,
            input_action::flag::bubble);
        if (!resize_action->has_bindings())
           resize_action->bind_resize();

        auto restore_action = input_action::create_no_map(
            "ryu_restore",
            "Ryu",
            "The Ryu application window was restored from minimized or maximized.",
            input_action::type::window);
        if (!restore_action->has_bindings())
            restore_action->bind_restore();
    }

    void engine::bind_events() {
        _action_provider.register_handler(
                input_action::find_by_name("ryu_quit"),
                [this](const event_data_t& data) {
                    quit();
                    return true;
                });
        _action_provider.register_handler(
                input_action::find_by_name("ryu_minimized"),
                [](const event_data_t& data) {
                    return true;
                });
        _action_provider.register_handler(
                input_action::find_by_name("ryu_maximized"),
                [](const event_data_t& data) {
                    return true;
                });
        _action_provider.register_handler(
                input_action::find_by_name("ryu_moved"),
                [this](const event_data_t& data) {
                    _window_rect.left(data.x);
                    _window_rect.top(data.y);
                    raise_move();
                    return true;
                });
        _action_provider.register_handler(
                input_action::find_by_name("ryu_resized"),
                [this](const event_data_t& data) {
                    _window_rect.width(data.width);
                    _clip_rect.width(data.width);
                    _window_rect.height(data.height);
                    _clip_rect.height(data.height);
                    raise_resize();
                    return true;
                });
        _action_provider.register_handler(
                input_action::find_by_name("ryu_restore"),
                [](const event_data_t& data) {
                    // XXX:
                    return true;
                });
    }

    void engine::raise_resize() {
        _surface.set_clip_rect(_clip_rect);
        if (_resize_callback != nullptr)
            _resize_callback(_clip_rect);
    }

    core::preferences* engine::prefs() {
        return _prefs;
    }

    bool engine::run(core::result& result) {
        uint16_t average_fps = 0;
        uint16_t frame_count = 0;

        auto last_time = SDL_GetTicks();
        auto last_fps_time = last_time;

        pending_event_list pending_events {};
        event_list events {};

        while (!_quit) {
            auto current_time = SDL_GetTicks();
            auto dt = current_time - last_time;

            timer_pool::instance()->update();

            auto fps_dt = last_time - last_fps_time;
            if (fps_dt >= 1000) {
                average_fps = frame_count;
                frame_count = 0;
                last_fps_time = last_time;
            }

            _surface.set_color({0x00, 0x00, 0x00, 0xff});
            _surface.clear();

            SDL_Event sdl_event{};
            while (true) {
                if (!SDL_PollEvent(&sdl_event))
                    break;
                events.push_back(sdl_event);
            }

            // XXX: need to fix joystick repeat functionality
            //          right now the binding code has to be called per some unit of
            //          time to ensure it works properly.
            if (!events.empty()) {
                action_type_flags types = input_action::type::none;
                for (const auto& e : events) {
                    switch (e.type) {
                        case SDL_QUIT:
                            types |= input_action::type::system;
                            break;
                        case SDL_WINDOWEVENT:
                            types |= input_action::type::window;
                            break;
                        case SDL_KEYUP:
                        case SDL_KEYDOWN:
                            types |= input_action::type::keyboard;
                            break;
                        case SDL_JOYBUTTONUP:
                        case SDL_JOYHATMOTION:
                        case SDL_JOYBUTTONDOWN:
                        case SDL_JOYBALLMOTION:
                            types |= input_action::type::joystick;
                            break;
                        case SDL_MOUSEWHEEL:
                        case SDL_MOUSEMOTION:
                        case SDL_MOUSEBUTTONUP:
                        case SDL_MOUSEBUTTONDOWN:
                            types |= input_action::type::mouse;
                            break;
                        default:
                            break;
                    }
                }

                auto filtered_actions = input_action::filtered_catalog(types);
                for (const auto& e : events) {
                    for (auto action : filtered_actions) {
                        event_data_t data{};
                        if (action->process(&e, data)) {
                            pending_events.push_back(pending_event_t{action, data});
                        }
                    }
                }
            }

            for (auto& it : _contexts)
                it.second->update(dt, pending_events, _surface);

            _action_provider.process(pending_events);

            // N.B. this is an override/overlay draw so contexts
            //      can "bleed" into other contexts.
            for (auto& it : _contexts)
                it.second->draw(_surface);

            if (_font != nullptr) {
                auto fps = fmt::format("FPS: {}", (int) average_fps);
                auto fps_width = _surface.measure_text(_font, fps);
                _surface.draw_text(
                        _font,
                        _window_rect.width() - (fps_width + 5),
                        _window_rect.height() - (_font->line_height + 4),
                        fps,
                        {0xff, 0xff, 0xff, 0xff});
            }

            _surface.present();

            pending_events.clear();
            events.clear();

            ++frame_count;

            auto frame_duration = SDL_GetTicks() - current_time;
            if (frame_duration < 16) {
                SDL_Delay(16 - frame_duration);
            }

            last_time = current_time;
        }

        return !result.is_failed();
    }

    const core::font_t* engine::font_face() {
        return _font;
    }

    core::font_family* engine::font_family() {
        return _font_family;
    }

    core::rect engine::window_position() const {
        return _window_rect;
    }

    void engine::add_context(core::context* context) {
        if (context == nullptr)
            return;
        context->engine(this);
        _contexts.insert(std::make_pair(context->id(), context));
    }

    void engine::font_face(const core::font_t* value) {
        _font = value;
    }

    void engine::font_family(core::font_family* value) {
        _font_family = value;
    }

    void engine::remove_context(core::context* context) {
        if (context == nullptr)
            return;
        context->engine(nullptr);
        _contexts.erase(context->id());
    }

    void engine::window_position(const core::rect& value) {
        SDL_SetWindowPosition(_window, value.left(), value.top());
        SDL_SetWindowSize(_window, value.width(), value.height());
    }

    void engine::on_move(const engine::move_callable& callback) {
        _move_callback = callback;
    }

    core::context* engine::find_context(const std::string& name) {
        for (auto it = _contexts.begin();
             it != _contexts.end();
             ++it) {
            if (it->second->name() == name)
                return it->second;
        }
        return nullptr;
    }

    void engine::on_resize(const engine::resize_callable& callback) {
        _resize_callback = callback;
    }

}