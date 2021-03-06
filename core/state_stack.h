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

#include <vector>
#include "core_types.h"
#include "renderer.h"
#include "input_binding.h"

namespace ryu::core {

    class state_stack {
    public:
        struct pending_state {
            enum action {
                none = 1,
                push,
                pop
            };
        };

        void draw(
                uint32_t dt,
                pending_event_list& events,
                core::renderer& renderer);

        void clear();

        int peek() const;

        bool empty() const;

        void pop(int to_id = -1);

        inline core::state* active() {
            return _active;
        }

        core::state* find_state(int id);

        void apply_pending_transition();

        void add_state(core::state* state);

        void remove_state(core::state* state);

        void push(int id, const core::parameter_dict& params);

        void add_state(core::state* state, const state_transition_callable& callback);

    private:
        void update_active_state();

    private:
        int _pending_id = -1;
        state_dict _states {};
        std::vector<int> _stack {};
        core::state* _active = nullptr;
        core::parameter_dict _pending_params {};
        pending_state::action _pending_action = pending_state::action::none;
    };

};

