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
#include <cstdint>
#include <functional>

namespace ryu::hardware {

    class memory_map_entry {
    public:
        using memory_map_entry_flags = uint8_t;
        using read_callable = std::function<uint8_t (uint32_t)>;
        using write_callable = std::function<void (uint32_t, uint8_t)>;

        enum flags : uint8_t {
            none                   = 0b00000000,
            static_config_required = 0b00000001,
        };

        memory_map_entry(
                uint32_t offset,
                uint32_t size,
                const std::string& name,
                const std::string& description,
                memory_map_entry_flags flags = flags::none);

        memory_map_entry(
                uint32_t offset,
                uint32_t size,
                const std::string& name,
                const std::string& description,
                const read_callable& reader,
                const write_callable& writer,
                memory_map_entry_flags flags = flags::none);

        uint32_t size() const;

        uint32_t offset() const;

        std::string name() const;

        std::string description() const;

        inline bool requires_static_config() const {
            return (_flags & flags::static_config_required) != 0;
        }

        uint8_t invoke_reader(uint32_t offset) const;

        void invoke_writer(uint32_t offset, uint8_t value);

    private:
        uint32_t _size;
        uint32_t _offset;
        std::string _name;
        std::string _description;
        read_callable _read_callable;
        write_callable _write_callable;
        memory_map_entry_flags _flags = flags::none;
    };

};

