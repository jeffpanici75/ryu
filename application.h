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

#include <ide/context.h>
#include <core/engine.h>
#include <emulator/context.h>

namespace ryu {

    class application {
    public:
        static const int display_width = 1920;
        static const int display_height = 1000;

        application();

        bool init();

        bool shutdown();

        int run(int argc, char** argv);

    private:
        void configure_ide();

        void configure_emulator();

        core::rect ide_bounds(const core::rect& bounds);

        core::rect emulator_bounds(const core::rect& bounds);

    private:
        core::engine _engine;
        ide::context _ide_context;
        emulator::context _emulator_context;
    };

};

