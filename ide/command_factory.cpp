//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <fmt/format.h>
#include <core/evaluator.h>
#include <hardware/machine.h>
#include <hardware/registry.h>
#include <boost/filesystem.hpp>
#include <core/command_parser.h>
#include <common/string_support.h>
#include "command_factory.h"

namespace ryu::ide {

    bool command_factory::execute(
            core::result& result,
            const std::string& line) {
        using namespace boost::filesystem;

        core::command_parser parser;
        parser.symbol_table(&_symbol_table);

        auto root = parser.parse(line);
        if (parser.result().is_failed()) {
            for (auto& msg : parser.result().messages())
                result.add_message(msg.code(), msg.message(), msg.is_error());
            result.fail();
            return false;
        }

        auto command = boost::get<core::command_t>(root->value);
        core::evaluator evaluator;
        evaluator.symbol_table(&_symbol_table);

        switch (command.type) {
            case core::command_t::quit: {
                result.add_message("C001", "Goodbye!");
                break;
            }
            case core::command_t::clear: {
                result.add_message("C004", "Clear screen buffer");
                break;
            }
            case core::command_t::evaluate: {
                for (const auto node : root->children) {
                    auto param = evaluator.evaluate(result, node);
                    switch (param.which()) {
                        case 1: {
                            std::string ascii;
                            uint32_t value = boost::get<uint32_t>(param);
                            auto* p = (char*) (&value) + 3;
                            for (auto i = 0; i < 4; i++) {
                                char c = *p--;
                                if (c == 0)
                                    ascii += ".";
                                else
                                    ascii += c;
                            }
                            result.add_message(
                                    "C003",
                                    fmt::format("${0:08x} {0:>10} \"{1:>4}\" %{0:032b}", value, ascii));
                            break;
                        }
                        case 2: {
                            auto lit = boost::get<std::string>(param);
                            auto dump = hex_dump(
                                    static_cast<const void*>(lit.c_str()),
                                    lit.length());
                            result.add_message("C003", dump);
                            break;
                        }
                    }
                }
                break;
            }
//            case command::change_directory: {
//                if (command.params.empty()) {
//                    result.add_message("C007", "path parameter is required", true);
//                    return false;
//                }
//                string_literal_t lit = boost::get<string_literal_t>(command.params[0]);
//                if (!is_directory(lit.value)) {
//                    result.add_message("C007", fmt::format("invalid path: {}", lit.value), true);
//                    return false;
//                }
//                current_path(lit.value);
//                auto cwd = current_path();
//                result.add_message("C007", fmt::format("working path is now: {}", cwd.string()));
//                break;
//            }
//            case command::dir: {
//                auto cwd = current_path();
//                if (is_directory(cwd)) {
//                    result.add_message("C005", fmt::format("{0}:", cwd.string()));
//                    std::vector<directory_entry> entries;
//                    copy(directory_iterator(cwd), directory_iterator(), back_inserter(entries));
//                    for (auto& entry : entries) {
//                        result.add_message("C005", fmt::format("  {0}", entry.path().filename().string()));
//                    }
//                }
//                break;
//            }
//            case command::remove_file: {
//                if (command.params.empty()) {
//                    result.add_message("C008", "path parameter is required", true);
//                    return false;
//                }
//                string_literal_t lit = boost::get<string_literal_t>(command.params[0]);
//                if (!is_directory(lit.value) && !is_regular_file(lit.value)) {
//                    result.add_message("C008", fmt::format("invalid path: {}", lit.value), true);
//                    return false;
//                }
//                if (remove(lit.value)) {
//                    result.add_message("C008", fmt::format("{} removed", lit.value));
//                } else {
//                    result.add_message("C008", fmt::format("remove of {} failed", lit.value), true);
//                }
//                break;
//            }
            case core::command_t::print_working_directory: {
                auto cwd = current_path();
                if (is_directory(cwd)) {
                    result.add_message("C006", cwd.string());
                }
                break;
            }
            case core::command_t::read_text: {
                if (root->children.empty()) {
                    result.add_message("C021", "path parameter is required", true);
                    return false;
                }
                auto lit = boost::get<std::string>(root->children[1]->value);
                if (!is_regular_file(lit)) {
                    result.add_message("C021", fmt::format("invalid path: {}", lit), true);
                    return false;
                }
                result.add_message("C021", lit);
                break;
            }
            case core::command_t::write_text: {
                std::string lit {"(default)"};
                if (!root->children.empty()) {
                    lit = boost::get<std::string>(root->children[1]->value);
                    if (!is_regular_file(lit)) {
                        result.add_message("C022", fmt::format("invalid path: {}", lit), true);
                        return false;
                    }
                }
                result.add_message("C022", lit);
                break;
            }
            case core::command_t::memory_editor: {
                result.add_data("C024", {});
                break;
            }
            case core::command_t::text_editor: {
                result.add_data("C002", {});
                break;
            }
//            case command::open_editor: {
//                if (command.params.size() < 2) {
//                    result.add_message(
//                            "C030",
//                            "open editor requires {italic}name{} and {italic}type{} parameters",
//                            true);
//                    return false;
//                }
//                const auto& name = command.params[0];
//                const auto& type = command.params[1];
//
//                core::parameter_dict params;
//                if (name.which() == 2) {
//                    string_literal_t lit = boost::get<string_literal_t>(name);
//                    params["name"] = lit.value;
//                } else {
//                    result.add_message("C030", "{italic}name{} must be a string literal", true);
//                    result.fail();
//                }
//
//                if (type.which() == 1) {
//                    identifier_t symbol = boost::get<identifier_t>(type);
//                    params["type"] = symbol.value;
//                } else {
//                    result.add_message("C030", "{italic}type{} must be a identifier", true);
//                    result.fail();
//                }
//
//                if (result.is_failed())
//                    return false;
//
//                result.add_data("C030", params);
//                break;
//            }
            case core::command_t::machines_list: {
                auto machines = hardware::registry::instance()->machines();
                result.add_message("C028", "{rev}{bold}  ID Name                             Type ");
                for (auto machine : machines) {
                    result.add_message(
                            "C028",
                            fmt::format(" {:>3d} {:<32s} {:<4s}",
                                        machine->id(),
                                        fmt::format("\"{}\"", machine->name()),
                                        "MACH"));
                }
                result.add_message("C028", fmt::format("{} registered machines", machines.size()));
                break;
            }
//            case command::machine_editor: {
//                if (command.params.empty()) {
//                    result.add_message("C023", "machine name required", true);
//                    return false;
//                }
//                const auto& param = command.params[0];
//                if (param.which() == 2) {
//                    string_literal_t lit = boost::get<string_literal_t>(param);
//                    result.add_data("C023", {{"name", lit.value}});
//                } else {
//                    result.add_message("C023", "machine name must be a string literal", true);
//                    return false;
//                }
//                break;
//            }
//            case command::use_machine: {
//                if (command.params.empty()) {
//                    result.add_message("C026", "machine name required", true);
//                    return false;
//                }
//                const auto& param = command.params[0];
//                if (param.which() == 2) {
//                    string_literal_t lit = boost::get<string_literal_t>(param);
//                    result.add_data("C026", {{"name", lit.value}});
//                } else {
//                    result.add_message("C026", "machine name must be a string literal", true);
//                    return false;
//                }
//                break;
//            }
//            case command::del_machine: {
//                if (command.params.empty()) {
//                    result.add_message("C027", "machine name required", true);
//                    return false;
//                }
//                const auto& param = command.params[0];
//                if (param.which() == 2) {
//                    string_literal_t lit = boost::get<string_literal_t>(param);
//                    result.add_data("C027", {{"name", lit.value}});
//                } else {
//                    result.add_message("C027", "machine name must be a string literal", true);
//                    return false;
//                }
//                break;
//            }
//            case command::goto_line: {
//                if (command.params.empty()) {
//                    result.add_message("C020", "line number is required", true);
//                    return false;
//                }
//                const auto& param = command.params[0];
//                if (param.which() == 0) {
//                    uint32_t value = boost::get<uint32_t>(param);
//                    result.add_message("C020", std::to_string(value));
//                } else {
//                    result.add_message("C020", "line number must be a number", true);
//                    return false;
//                }
//                break;
//            }
            default: {
                result.add_message("C400", "Command not implemented.", true);
                return false;
            }
        }

        return true;
    }

}
