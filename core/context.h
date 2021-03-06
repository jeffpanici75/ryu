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
#include "input_action_provider.h"

namespace ryu::core {

    class environment;

    class context {
    public:
        explicit context(const std::string& name);

        virtual ~context();

        void update(
                uint32_t dt,
                pending_event_list& pending_events,
                core::renderer& renderer);

        void resize();

        void add_state(
                core::result& result,
                core::state* state,
                const state_transition_callable& callback);

        void add_state(
                core::result& result,
                core::state* state);

        void push_state(
                int id,
                const core::parameter_dict& params);

        bool initialize(
                core::result& result,
                const core::rect& bounds);

        uint32_t id() const;

        uint8_t fg_color() const {
            return _fg_color;
        }

        uint8_t bg_color() const {
            return _bg_color;
        }

        core::palette& palette() {
            return _palette;
        }

        core::preferences* prefs() {
            return _engine->prefs();
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

        const core::font_t* font_face();

        core::font_family* font_family();

        inline std::string name() const {
            return _name;
        }

        void engine(core::engine* value) {
            _engine = value;
        }

        void draw(core::renderer& renderer);

        inline core::engine* engine() const {
            return _engine;
        }

        void remove_state(core::state* state);

        inline core::state* find_state(int id) {
            return _stack.find_state(id);
        }

        inline core::environment* environment() {
            return _environment.get();
        }

        inline const core::rect& bounds() const {
            return _bounds;
        }

        void font_face(const core::font_t* value);

        void font_family(core::font_family* value);

        inline void bounds(const core::rect& value) {
            _bounds = value;
            resize();
        }

    protected:
        virtual void on_draw(core::renderer& surface);

        core::input_action_provider& action_provider();

        virtual bool on_initialize(core::result& result);

    private:
        uint32_t _id = 0;
        std::string _name {};
        uint8_t _fg_color = 0;
        uint8_t _bg_color = 0;
        core::rect _bounds {};
        core::palette _palette {};
        core::state_stack _stack {};
        core::engine* _engine = nullptr;
        const core::font_t* _font = nullptr;
        core::font_family* _family = nullptr;
        core::input_action_provider _action_provider;
        std::unique_ptr<core::environment> _environment;
    };

};