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
#include <utility>
#include <hardware/machine.h>
#include <boost/filesystem/path.hpp>
#include "result.h"

namespace ryu::core {

    namespace fs = boost::filesystem;

    // N.B. ideas behind environments:
    //  - std::vector<boost::filesystem::path> for each environment assembly file
    //  - commands in command parser to switch between environments
    //  -       this does a environment::load (which does an assemble/execute)
    //  - command for newenv
    //  - command for switchenv
    //  - command for delenv
    //  - command for listenv
    //
    class project {
    public:
        static bool create(
                core::result& result,
                const fs::path& path,
                const std::string& name);

        static bool clone(
                core::result& result,
                const fs::path& source_path,
                const fs::path& target_path);

        explicit project(const std::string& name);

        std::string name() const;

        hardware::machine* machine();

        bool save(core::result& result);

        void machine(hardware::machine* machine);

        bool load(core::result& result, const fs::path& file_name);

    private:
        std::string _name;
        fs::path _file_name;
        hardware::machine* _machine = nullptr;
    };

};

