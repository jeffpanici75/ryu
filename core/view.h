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
#include <vector>
#include <cstdint>
#include <SDL_events.h>
#include <SDL_render.h>
#include "rect.h"
#include "palette.h"
#include "context.h"
#include "padding.h"

namespace ryu::core {

    class view;

    typedef std::vector<view*> view_list;

    class view {
    public:
        struct config {
            enum flags : uint8_t {
                none    = 0b00000000,
                visible = 0b00000001,
                enabled = 0b00000010,
                tabstop = 0b00000100,
                focused = 0b00001000,
            };
        };

        struct dock {
            enum styles : uint8_t {
                none,
                bottom,
                fill,
                left,
                right,
                top
            };
        };

        struct types {
            enum id : uint8_t {
                none,
                label,
                textbox,
                checkbox,
                combobox,
                list,
                menu,
                menuitem,
                custom
            };
        };

        view(
                core::context* context,
                view* parent,
                types::id type,
                int id,
                const std::string& name);

        virtual ~view() = default;

        int id() const;

        view* parent();

        core::rect& rect();

        void focus(int id);

        short index() const;

        bool focused() const;

        bool enabled() const;

        bool visible() const;

        bool tabstop() const;

        view_list& children();

        types::id type() const;

        core::padding& margin();

        void index(short value);

        void enabled(bool value);

        void visible(bool value);

        void tabstop(bool value);

        std::string name() const;

        core::context* context();

        core::palette* palette();

        core::padding& padding();

        core::rect client_rect();

        uint8_t bg_color() const;

        uint8_t fg_color() const;

        dock::styles dock() const;

        core::font_t* font() const;

        void bg_color(uint8_t value);

        void fg_color(uint8_t value);

        void dock(dock::styles style);

        void font(core::font_t* font);

        void draw(SDL_Renderer* renderer);

        void rect(const core::rect& value);

        void palette(core::palette* palette);

        bool process_event(const SDL_Event* e);

        void margin(const core::padding& value);

        void padding(const core::padding& value);

    protected:
        void focus(bool value);

        virtual void on_focus_changed();

        virtual void on_draw(SDL_Renderer* renderer);

        virtual bool on_process_event(const SDL_Event* e);

    private:
        int _id;
        short _index;
        uint8_t _flags;
        std::string _name;
        core::font_t* _font;
        core::rect _rect {};
        view_list _children;
        core::padding _margin;
        uint8_t _bg_color = 0;
        uint8_t _fg_color = 0;
        core::padding _padding;
        view* _parent = nullptr;
        types::id _type = types::none;
        core::context* _context = nullptr;
        core::palette* _palette = nullptr;
        dock::styles _dock = dock::styles::none;
    };

};

