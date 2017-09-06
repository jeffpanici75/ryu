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

#include <cstdint>

namespace ryu::core {

    class padding {
    public:
        padding() = default;

        padding(
                int32_t left,
                int32_t right,
                int32_t top,
                int32_t bottom);

        int32_t left() const;

        void left(int32_t value);

        int32_t right() const;

        void right(int32_t value);

        int32_t top() const;

        void top(int32_t value);

        int32_t bottom() const;

        void bottom(int32_t value);

        int32_t horizontal() const;

        int32_t vertical() const;

    private:
        int32_t _top = 0;
        int32_t _left = 0;
        int32_t _right = 0;
        int32_t _bottom = 0;
    };

};

