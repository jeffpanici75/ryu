//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//

#pragma once

#include <hardware/integrated_circuit.h>
#include "opcode.h"

namespace ryu::hardware::mc6809 {

    class cpu : public hardware::integrated_circuit {
    public:
        static constexpr uint16_t type_id = 10;

        static const mnemonic_set& mnemonics();

        static const opcode_dictionary& opcodes();

        static void init();

        cpu();

        RTTR_ENABLE(hardware::integrated_circuit)

    private:
        static mnemonic_set _mnemonics;
        static opcode_dictionary _opcodes;
    };

};
