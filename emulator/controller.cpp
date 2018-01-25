//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <core/timer.h>
#include <core/engine.h>
#include <core/context.h>
#include "controller.h"

namespace ryu::emulator {

    controller::controller(const std::string& name) : core::state(name) {
    }

    controller::~controller() {
    }

    void controller::on_initialize() {
    }

    void controller::on_update(uint32_t dt) {
    }

    void controller::on_draw(core::renderer& surface) {
    }

    void controller::on_resize(const core::rect& bounds) {
    }

    bool controller::on_process_event(const SDL_Event* e) {
        return false;
    }

}