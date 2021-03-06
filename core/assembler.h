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

#include <hardware/component.h>
#include "evaluator.h"
#include "assembler_parser.h"
#include "assembly_listing.h"

namespace ryu::core {

    namespace fs = boost::filesystem;

    class assembler {
    public:
        assembler();

        virtual ~assembler() = default;

        bool assemble(
                core::result& result,
                const parser_input_t& input);

        bool load_target(
                core::result& result,
                const std::string& name);

        bool assemble_stream(
                core::result& result,
                const parser_input_t& input);

        byte_list write_data(
                directive_t::data_sizes size,
                uint32_t value);

        void align(uint8_t size);

        hardware::component* target();

        core::assembly_listing& listing();

        uint32_t location_counter() const;

        core::symbol_table* symbol_table();

        void location_counter(uint32_t value);

        bool load_binary_to_location_counter(
                core::result& result,
                byte_list& data_bytes,
                std::iostream& stream,
                uint32_t end_address);

        bool read_location_counter_to_binary(
                core::result& result,
                std::iostream& stream,
                uint32_t end_address);

        void symbol_table(core::symbol_table* value);

        byte_list write_data(const std::string& value);

        byte_list read_data(directive_t::data_sizes size);

        void increment_location_counter(directive_t::data_sizes size);

    private:
        core::evaluator _evaluator;
        uint32_t _location_counter = 0;
        core::assembler_parser _parser {};
        core::assembly_listing _listing {};
        hardware::component* _target = nullptr;
        core::symbol_table* _symbol_table = nullptr;
    };

};

