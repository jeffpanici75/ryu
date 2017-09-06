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
#include <SDL_rect.h>

namespace ryu::core {

    class rect {
    public:
        rect() = default;

        rect(int32_t left, int32_t top, int32_t width, int32_t height);

        int32_t left() const;

        int32_t top() const;

        int32_t width() const;

        int32_t height() const;

        int32_t right() const;

        int32_t bottom() const;

        inline SDL_Rect to_sdl_rect() {
            return SDL_Rect{left(), top(), width(), height()};
        }

        rect& pos(int32_t left, int32_t top);

        void inflate(int32_t dx, int32_t dy);

        void deflate(int32_t dx, int32_t dy);

        bool contains(const rect& rect) const;

        bool intersects(const rect& rect) const;

        rect& size(int32_t width, int32_t height);

        bool contains(int32_t x, int32_t y) const;

        rect center_inside(const rect& target, bool scale = false);

    private:
        int32_t _top {};
        int32_t _left {};
        int32_t _width {};
        int32_t _height {};
    };

};

