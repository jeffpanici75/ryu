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

#include <string>
#include "rect.h"
#include "engine.h"
#include "palette.h"
#include "renderer.h"
#include "state_stack.h"

namespace ryu::core {

    class context {
    public:
        explicit context(const std::string& name);

        virtual ~context();

        void resize();

        int id() const;

        void update(
                uint32_t dt,
                core::renderer& renderer,
                std::deque<SDL_Event>& events);

        uint8_t fg_color() const {
            return _fg_color;
        }

        uint8_t bg_color() const {
            return _bg_color;
        }

        void fg_color(uint8_t index) {
            _fg_color = index;
        }

        void bg_color(uint8_t index) {
            _bg_color = index;
        }

        inline int peek_state() const {
            return _stack.peek();
        }

        void pop_state(int to_id = -1);

        void remove_timer(timer* timer);

        inline core::rect bounds() const {
            return _bounds;
        }

        void engine(core::engine* value) {
            _engine = value;
        }

        void add_timer(core::timer* timer);

        void add_state(core::state* state);

        void palette(core::palette* palette);

        void remove_state(core::state* state);

        inline core::engine* engine() const {
            return _engine;
        }

        inline core::palette* palette() const {
            return _palette;
        }

        inline core::state* find_state(int id) {
            return _stack.find_state(id);
        }

        void initialize(const core::rect& bounds);

        inline void bounds(const core::rect& value) {
            _bounds = value;
            resize();
        }

        void erase_blackboard(const std::string& name);

        std::string blackboard(const std::string& name) const;

        void push_state(int id, const core::parameter_dict& params);

        void blackboard(const std::string& name, const std::string& value);

        void add_state(core::state* state, const state_transition_callable& callback);

    protected:
        virtual void on_initialize();

    private:
        int _id = 0;
        std::string _name {};
        uint8_t _fg_color = 0;
        uint8_t _bg_color = 0;
        timer_list _timers {};
        core::rect _bounds {};
        core::state_stack _stack {};
        core::blackboard _blackboard {};
        core::engine* _engine = nullptr;
        core::palette* _palette = nullptr;
    };

};

