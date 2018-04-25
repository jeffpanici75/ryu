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

#include <core/view.h>
#include <logger_factory.h>

namespace ryu::core {

    class label : public core::view {
    public:
        label(
            const std::string& name,
            core::view_host* host);

        ~label() override;

        border::types border() const;

        void border(border::types value);

        std::string value() const override;

        alignment::vertical::types valign() const;

        alignment::horizontal::types halign() const;

        void value(const std::string& text) override;

        void valign(alignment::vertical::types value);

        void halign(alignment::horizontal::types value);

    protected:
        void on_draw(core::renderer& surface) override;

        void on_resize(const core::rect& context_bounds) override;

    private:
        void update_content_bounds();

    private:
        border::types _border = border::types::none;
        alignment::vertical::types _valign = alignment::vertical::middle;
        alignment::horizontal::types _halign = alignment::horizontal::none;
    };

};

