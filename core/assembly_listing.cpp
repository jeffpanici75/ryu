//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include "assembly_listing.h"

namespace ryu::core {

    assembly_listing::assembly_listing() {
        using format_options = core::data_table_column_t::format_options;

        _table.line_spacing = 0;
        _table.headers.push_back({
                "Line",
                6,
                6,
                alignment::horizontal::left,
                1,
                format_options::none
        });
        _table.headers.push_back({
                "Opcodes",
                20,
                20,
                alignment::horizontal::left,
                0,
                format_options::none
        });
        _table.headers.push_back({
                "Flags",
                5,
                5,
                alignment::horizontal::left,
                1,
                format_options::none
        });
        _table.headers.push_back({
                "Source",
                40,
                40,
                alignment::horizontal::left,
                1,
                format_options::style_codes
        });

        _table.footers.push_back({"Line Count", 15, 20});
    }

    void assembly_listing::clear() {
        _table.rows.clear();
    }

    void assembly_listing::add_row(
            uint32_t line_number,
            const std::vector<uint32_t>& opcodes,
            assembly_listing::row_flags_t flags,
            const std::string& source) {
        core::data_table_row_t row {};
        row.columns.push_back(fmt::format("{:05d}", line_number));

        std::string opcode_hex;
        for (const auto& opcode : opcodes)
            opcode_hex += fmt::format("{:02x} ", opcode);
        row.columns.push_back(opcode_hex);

        std::string flag_chars;
        if ((flags & row_flags::binary) != 0)
            flag_chars += 'b';
        if ((flags & row_flags::include) != 0)
            flag_chars += 'i';
        if ((flags & row_flags::macro) != 0)
            flag_chars += 'm';
        row.columns.push_back(flag_chars);

        row.columns.push_back(source);

        _table.rows.push_back(row);
    }

    core::data_table_t& assembly_listing::table() {
        return _table;
    }

    void assembly_listing::add_footer(uint32_t total_lines) {
        _table.rows.push_back({
             {fmt::format("{} lines assembled", total_lines)}
        });
    }

}