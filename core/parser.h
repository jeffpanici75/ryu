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

#include <map>
#include <string>
#include <fmt/format.h>
#include <boost/variant.hpp>
#include "result.h"
#include "core_types.h"
#include "parser_types.h"

namespace ryu::core {

    class symbol_table;

    class parser {
    public:
        parser() = default;

        const core::result& result() const;

        core::symbol_table* symbol_table();

        void symbol_table(core::symbol_table* value);

        ast_node_shared_ptr parse_expression(const parser_input_t& input);

        virtual ast_node_shared_ptr parse(const parser_input_t& input) = 0;

    protected: // shunting yard
        bool has_operand();

        bool has_operator();

        operator_t* pop_operator();

        operator_t* peek_operator();

        ast_node_shared_ptr pop_operand();

        ast_node_shared_ptr peek_operand();

        void push_operator(operator_t* op);

        void push_operand(const ast_node_shared_ptr& node);

    protected: // core
        void error(
                const std::string& code,
                const std::string& message);

        char* set_token();

        void clear_stacks();

        void pop_position();

        void push_position();

        void increment_line();

        char* current_token();

        void register_operator(
                const std::string& key,
                const core::operator_t& op);

        bool is_failed() const {
            return _result.is_failed();
        }

        char* move_to_next_token();

        void consume_white_space();

        size_t forget_top_position();

        inline uint32_t line() const {
            return _line;
        }

        void consume_tokens(int count);

        inline uint32_t column() const {
            return _column;
        }

        void reset(const parser_input_t& input);

        ast_node_shared_ptr create_ast_node(ast_node_t::tokens type);

    protected: // parsers
        operator_t* parse_operator();

        ast_node_shared_ptr parse_number();

        ast_node_shared_ptr parse_comment();

        ast_node_shared_ptr parse_expression();

        ast_node_shared_ptr parse_identifier();

        ast_node_shared_ptr parse_null_literal();

        ast_node_shared_ptr parse_uninitialized();

        ast_node_shared_ptr parse_string_literal();

        ast_node_shared_ptr parse_boolean_literal();

        ast_node_shared_ptr parse_character_literal();

        ast_node_shared_ptr parse_location_counter_literal();

    private:
        bool operator_stack_has(operator_t* op);

        bool match_literal(const std::string& literal);

        std::vector<operator_t*> find_matching_operators(
                std::vector<operator_t*> candidates,
                char token,
                size_t index);

    private:
        const std::vector<std::function<ast_node_shared_ptr ()>> _terminals = {
                [&] () {return parse_number();},
                [&] () {return parse_comment();},
                [&] () {return parse_null_literal();},
                [&] () {return parse_boolean_literal();},
                [&] () {return parse_identifier();},
                [&] () {return parse_string_literal();},
                [&] () {return parse_character_literal();},
                [&] () {return parse_uninitialized();},
                [&] () {return parse_location_counter_literal();},
        };

        static operator_dict _operators;

        size_t _index {};
        uint32_t _line {};
        uint32_t _column {};
        core::result _result;
        char* _token = nullptr;
        parser_input_t _input {};
        std::vector<operator_t*> _operator_stack;
        std::stack<scanner_pos_t> _position_stack;
        core::symbol_table* _symbol_table = nullptr;
        std::stack<ast_node_shared_ptr> _operand_stack;
    };

};

