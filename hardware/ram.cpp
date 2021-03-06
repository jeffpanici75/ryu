//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <common/bytes.h>
#include "ram.h"

RTTR_REGISTRATION {
    rttr::registration::class_<ryu::hardware::ram>("ryu::hardware::ram") (
        rttr::metadata(ryu::hardware::meta_data_key::type_id, ryu::hardware::ram_id),
        rttr::metadata(ryu::hardware::meta_data_key::type_name, "RAM IC"),
        rttr::metadata(ryu::hardware::meta_data_key::type_category, ryu::hardware::category_memory)
    )
    .constructor<>(rttr::registration::public_access)  (
            rttr::policy::ctor::as_raw_ptr
    );
}

namespace ryu::hardware {

    ram::ram() : integrated_circuit("ram-ic") {
    }

    ram::~ram() {
        delete _buffer;
        _buffer = nullptr;
    }

    void ram::init() {
    }

    void ram::zero() {
        std::memset(_buffer, 0, address_space());
    }

    void ram::on_initialize() {
    }

    ryu::core::byte_list ram::write_word(
            uint32_t address,
            uint16_t value,
            integrated_circuit::endianness::types endianess) {
        ryu::core::byte_list data {};

        if (is_platform_little_endian()
        &&  endianess == integrated_circuit::endianness::types::big) {
            value = endian_swap_word(value);
        }
        auto byte_ptr = reinterpret_cast<uint8_t*>(&value);
        _buffer[address]     = *byte_ptr;
        _buffer[address + 1] = *(byte_ptr + 1);

        data.push_back(*byte_ptr);
        data.push_back(*(byte_ptr + 1));

        return data;
    }

    ryu::core::byte_list ram::write_dword(
            uint32_t address,
            uint32_t value,
            integrated_circuit::endianness::types endianess) {
        ryu::core::byte_list data {};

        if (is_platform_little_endian()
        &&  endianess == integrated_circuit::endianness::types::big) {
            value = endian_swap_dword(value);
        }
        auto byte_ptr = reinterpret_cast<uint8_t*>(&value);
        _buffer[address]     = *byte_ptr;
        _buffer[address + 1] = *(byte_ptr + 1);
        _buffer[address + 2] = *(byte_ptr + 2);
        _buffer[address + 3] = *(byte_ptr + 3);

        data.push_back(*byte_ptr);
        data.push_back(*(byte_ptr + 1));
        data.push_back(*(byte_ptr + 2));
        data.push_back(*(byte_ptr + 3));

        return data;
    }

    uint16_t ram::read_word(
            uint32_t address,
            integrated_circuit::endianness::types endianess) const {
        auto value = *(reinterpret_cast<uint16_t*>(_buffer + address));

        if (is_platform_little_endian()
        &&  endianess == integrated_circuit::endianness::types::big) {
            return endian_swap_word(value);
        }

        return value;
    }

    uint32_t ram::read_dword(
            uint32_t address,
            integrated_circuit::endianness::types endianess) const {
        auto value = *(reinterpret_cast<uint32_t*>(_buffer + address));

        if (is_platform_little_endian()
        &&  endianess == integrated_circuit::endianness::types::big) {
            return endian_swap_dword(value);
        }

        return value;
    }

    void ram::reallocate() {
        clear_memory_map();

        add_memory_map_entry(0, address_space(), "RAM", "Read/write memory block");

        delete _buffer;
        _buffer = new uint8_t[address_space()];
        fill(0xa9);
    }

    void ram::fill(uint8_t value) {
        std::memset(_buffer, value, address_space());
    }

    void ram::on_address_space_changed() {
        reallocate();
    }

    access_type_flags ram::access_type() const {
        return access_types::writable | access_types::readable;
    }

    uint8_t ram::read_byte(uint32_t address) const {
        return _buffer[address];
    }

    void ram::write_byte(uint32_t address, uint8_t value) {
        _buffer[address] = value;
    }

}
