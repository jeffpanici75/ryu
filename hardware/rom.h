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

#include "integrated_circuit.h"
#include "memory.h"

namespace ryu::hardware {

    class rom : public hardware::integrated_circuit,
                public hardware::memory {
    public:
        rom(
                const std::string& name,
                uint8_t* buffer,
                size_t size,
                uint32_t address);

        ~rom() override;

        void zero() override;

        bool write_latch() const;

        size_t size() const override;

        void write_latch(bool enabled);

        void fill(uint8_t value) override;

        std::string type() const override {
            return "rom";
        }

        uint32_t address_space() const override;

        uint8_t read_byte(uint32_t address) const override;

        void write_byte(uint32_t address, uint8_t value) override;

        RTTR_ENABLE(hardware::integrated_circuit, hardware::memory)

    private:
        size_t _size;
        bool _write_latch = false;
        uint8_t* _buffer = nullptr;
    };

};

