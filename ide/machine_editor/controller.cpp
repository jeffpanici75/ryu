//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <rttr/type>
#include <core/engine.h>
#include <ide/ide_types.h>
#include <hardware/hardware.h>
#include <hardware/registry.h>
#include "controller.h"

namespace ryu::ide::machine_editor {

    controller::controller(const std::string& name) : core::state(name),
                                                      _header("header-label"),
                                                      _footer("footer-label"),
                                                      _editor("editor"),
                                                      _name_label("name-label"),
                                                      _map_button("map-button"),
                                                      _add_button("add-button"),
                                                      _display_label("display-label"),
                                                      _delete_button("delete-button"),
                                                      _name_textbox("name-textbox"),
                                                      _panel("layout-panel"),
                                                      _address_space_label("address-space-label"),
                                                      _display_pick_list("display-pick-list"),
                                                      _row1_panel("row1"),
                                                      _row2_panel("row2"),
                                                      _address_space_textbox("address-space-textbox"),
                                                      _button_panel("buttons") {
    }

    void controller::on_deactivate() {
        _machine = nullptr;
    }

    void controller::update_values() {
        int machine_id = 0;
        if (_machine != nullptr) {
            machine_id = _machine->id();
            _name_textbox.value(_machine->name());
            auto disp = _machine->display();
            if (disp != nullptr)
                _display_pick_list.value(disp->name());
            _address_space_textbox.value(fmt::format("{0:08x}", _machine->address_space()));
        }
        std::string project_name = "(none)";
//        if (_project != nullptr) {
//            project_name = _project->name();
//        }
        _header.value(fmt::format("machine editor | project: {0} | id: {1}", project_name, machine_id));
        _header.resize(context()->bounds());
    }

    void controller::on_initialize() {
        auto family = context()->engine()->find_font_family("hack");
        auto face = family->find_style(core::font::styles::normal);

        _header.font_family(family);
        _header.palette(context()->palette());
        _header.dock(core::dock::styles::top);
        _header.fg_color(ide::colors::info_text);
        _header.bg_color(ide::colors::fill_color);
        _header.margin({_metrics.left_padding, _metrics.right_padding, 5, 5});

        _footer.font_family(family);
        _footer.palette(context()->palette());
        _footer.dock(core::dock::styles::bottom);
        _footer.fg_color(ide::colors::info_text);
        _footer.bg_color(ide::colors::fill_color);
        _footer.margin({_metrics.left_padding, _metrics.right_padding, 5, 5});
        _footer.value("F1=Map | F2=Add | DEL=Remove | ESC=Close");

        _name_label.value("name:");
        _name_label.font_family(family);
        _name_label.margin({0, 0, 0, 0});
        _name_label.palette(context()->palette());
        _name_label.dock(core::dock::styles::left);
        _name_label.fg_color(ide::colors::text);
        _name_label.bg_color(ide::colors::fill_color);
        _name_label.sizing(core::view::sizing::types::fixed);
        _name_label.halign(core::alignment::horizontal::right);
        _name_label.bounds().size(
                face->measure_text("display:"),
                face->line_height);

        _name_textbox.enabled(true);
        _name_textbox.font_family(family);
        _name_textbox.margin({5, 0, 3, 0});
        _name_textbox.palette(context()->palette());
        _name_textbox.dock(core::dock::styles::left);
        _name_textbox.fg_color(ide::colors::text);
        _name_textbox.bg_color(ide::colors::fill_color);
        _name_textbox.initialize(1, 32);
        _name_textbox.on_key_down([&](int keycode) {
            return true;
        });
        _name_textbox.on_tab([&]() {
            _panel.focus(_address_space_textbox.id());
        });
        _name_textbox.bounds().size(face->width * _name_textbox.length(), face->line_height);

        _address_space_label.font_family(family);
        _address_space_label.margin({15, 0, 0, 0});
        _address_space_label.value("address space: $");
        _address_space_label.palette(context()->palette());
        _address_space_label.dock(core::dock::styles::left);
        _address_space_label.fg_color(ide::colors::text);
        _address_space_label.bg_color(ide::colors::fill_color);
        _address_space_label.sizing(core::view::sizing::types::fixed);
        _address_space_label.halign(core::alignment::horizontal::left);
        _address_space_label.bounds().size(
                face->measure_text(_address_space_label.value()),
                face->line_height);

        _address_space_textbox.enabled(true);
        _address_space_textbox.font_family(family);
        _address_space_textbox.margin({5, 0, 3, 0});
        _address_space_textbox.palette(context()->palette());
        _address_space_textbox.dock(core::dock::styles::left);
        _address_space_textbox.fg_color(ide::colors::text);
        _address_space_textbox.bg_color(ide::colors::fill_color);
        _address_space_textbox.initialize(1, 8);
        _address_space_textbox.on_key_down([&](int keycode) {
            return isxdigit(keycode);
        });
        _address_space_textbox.on_tab([&]() {
            _panel.focus(_display_pick_list.id());
        });
        _address_space_textbox.bounds().size(
                face->width * _address_space_textbox.length(),
                face->line_height);

        _display_label.value("display:");
        _display_label.font_family(family);
        _display_label.margin({0, 0, 0, 0});
        _display_label.palette(context()->palette());
        _display_label.dock(core::dock::styles::left);
        _display_label.fg_color(ide::colors::text);
        _display_label.bg_color(ide::colors::fill_color);
        _display_label.sizing(core::view::sizing::types::fixed);
        _display_label.halign(core::alignment::horizontal::left);
        _display_label.bounds().size(
                face->measure_text(_display_label.value()),
                face->line_height);

        auto& displays = _display_pick_list.options();
        displays.push_back("Atari 19\"");
        displays.push_back("Atari 25\"");
        displays.push_back("Bally/Midway");
        displays.push_back("Eastern Micro Electronics, Inc");
        displays.push_back("Electrohome");
        displays.push_back("Eygo 814");
        displays.push_back("Eygo 820");
        displays.push_back("Eygo 822");
        displays.push_back("Happ Controls KJ-XX15 13\"");
        displays.push_back("Happ Controls KJ-XX15 19\"");
        displays.push_back("Hantarex MT-3000 9\"");
        displays.push_back("Hantarex MT-3000 12\"");
        displays.push_back("Hantarex MTC-900/E");
        displays.push_back("Hantarex MTC-9000");
        displays.push_back("Hantarex MTC-9110");
        displays.push_back("Kortek Imperial 19\"");
        displays.push_back("LAI LAI-KZ-14XYB");
        displays.push_back("LAI LAI-KZ-20XYB");
        displays.push_back("Matsushita 14\"");
        displays.push_back("Matsushita TM-202G");
        displays.push_back("Motorola M5000 19\"");
        displays.push_back("Motorola M7000 23\"");
        displays.push_back("Motorola XM501-10 19\"");
        displays.push_back("Motorola XM701-10 23\"");
        displays.push_back("Nanao MC-2000-S");
        displays.push_back("Neotec NT-2515C / N25E 25\"");
        displays.push_back("QNIC 14\"");
        displays.push_back("QNIC 20\"");
        displays.push_back("QNIC 25\"");
        displays.push_back("Sanyo 14\"");
        displays.push_back("Sanyo 20-EZV");
        displays.push_back("Sanyo 20-Z2AW");
        displays.push_back("Sharp XM-2001N");
        displays.push_back("Sharp Image SI-3195 Sony 19\"");
        displays.push_back("Sharp Image SI-327 27\"");
        displays.push_back("TEC TM-600 19\"");
        displays.push_back("TEC TM623 23\"");
        displays.push_back("Wells-Gardner Raster");
        displays.push_back("Williams Raster");
        displays.push_back("Zenith CD-19MRF06");
        _display_pick_list.font_family(family);
        _display_pick_list.margin({10, 0, 4, 0});
        _display_pick_list.palette(context()->palette());
        _display_pick_list.dock(core::dock::styles::left);
        _display_pick_list.fg_color(ide::colors::text);
        _display_pick_list.bg_color(ide::colors::fill_color);
        _display_pick_list.on_tab([&]() {
            _panel.focus(_map_button.id());
        });
        _display_pick_list.bounds().size(
                face->width * _display_pick_list.length(),
                face->line_height);

        _map_button.value("Map");
        _map_button.font_family(family);
        _map_button.margin({5, 5, 5, 5});
        _map_button.palette(context()->palette());
        _map_button.dock(core::dock::styles::left);
        _map_button.fg_color(ide::colors::light_grey);
        _map_button.bg_color(ide::colors::light_blue);
        _map_button.on_tab([&]() {
            _panel.focus(_add_button.id());
        });
        _map_button.bounds().size(
                face->measure_text("Delete") + 50,
                face->line_height + 30);

        _add_button.value("Add");
        _add_button.font_family(family);
        _add_button.margin({5, 5, 5, 5});
        _add_button.palette(context()->palette());
        _add_button.dock(core::dock::styles::left);
        _add_button.fg_color(ide::colors::light_grey);
        _add_button.bg_color(ide::colors::light_blue);
        _add_button.on_tab([&]() {
            _panel.focus(_delete_button.id());
        });
        _add_button.bounds().size(
                face->measure_text("Delete") + 50,
                face->line_height + 30);

        _delete_button.value("Delete");
        _delete_button.font_family(family);
        _delete_button.margin({5, 5, 5, 5});
        _delete_button.palette(context()->palette());
        _delete_button.dock(core::dock::styles::left);
        _delete_button.fg_color(ide::colors::light_grey);
        _delete_button.bg_color(ide::colors::light_blue);
        _delete_button.on_tab([&]() {
            _panel.focus(_name_textbox.id());
        });
        _delete_button.bounds().size(
                face->measure_text(_delete_button.value()) + 50,
                face->line_height + 30);

        _row1_panel.palette(context()->palette());
        _row1_panel.dock(core::dock::styles::top);
        _row1_panel.bg_color(ide::colors::transparent);
        _row1_panel.bounds().height(face->line_height * 2);
        _row1_panel.margin({_metrics.left_padding, _metrics.right_padding, 5, 5});
        _row1_panel.add_child(&_name_label);
        _row1_panel.add_child(&_name_textbox);
        _row1_panel.add_child(&_address_space_label);
        _row1_panel.add_child(&_address_space_textbox);

        _row2_panel.palette(context()->palette());
        _row2_panel.dock(core::dock::styles::top);
        _row2_panel.bg_color(ide::colors::transparent);
        _row2_panel.bounds().height(face->line_height * 2);
        _row2_panel.margin({_metrics.left_padding, _metrics.right_padding, 5, 5});
        _row2_panel.add_child(&_display_label);
        _row2_panel.add_child(&_display_pick_list);

        _button_panel.palette(context()->palette());
        _button_panel.dock(core::dock::styles::bottom);
        _button_panel.bg_color(ide::colors::transparent);
        _button_panel.bounds().height(face->line_height * 2);
        _button_panel.margin({_metrics.left_padding, _metrics.right_padding, 5, 5});
        _button_panel.add_child(&_map_button);
        _button_panel.add_child(&_add_button);
        _button_panel.add_child(&_delete_button);

        _panel.font_family(family);
        _panel.palette(context()->palette());
        _panel.dock(core::dock::styles::fill);
        _panel.fg_color(ide::colors::info_text);
        _panel.bg_color(ide::colors::fill_color);
        _panel.add_child(&_header);
        _panel.add_child(&_row1_panel);
        _panel.add_child(&_row2_panel);
        _panel.add_child(&_footer);
        _panel.add_child(&_button_panel);
        _panel.focus(_name_textbox.id());
        
        _editor.font_family(family);
        _editor.palette(context()->palette());
        _editor.initialize();
//        _editor.focus(_editor.id());

//        auto base = rttr::type::get<ryu::hardware::integrated_circuit>();
//        auto component_types = base.get_derived_classes();
//        for(auto& t : component_types) {
//            std::cout << t.get_name() << "\n";
//            std::cout << "properties:\n";
//            for (auto& p : t.get_properties())
//                std::cout << "\tname: " << p.get_name() << "\n";
//        }
    }

    void controller::on_update(uint32_t dt) {
    }

    hardware::machine* controller::machine() {
        return _machine;
    }

    void controller::on_draw(core::renderer& surface) {
        _panel.draw(surface);
    }

    void controller::machine(hardware::machine* value) {
        _machine = value;
        update_values();
    }

    void controller::on_resize(const core::rect& bounds) {
        _panel.resize(bounds);
    }

    bool controller::on_process_event(const SDL_Event* e) {
        if (e->type == SDL_KEYDOWN) {
            switch (e->key.keysym.sym) {
                case SDLK_ESCAPE: {
                    end_state();
                    return true;
                }
            }
        }
        return _panel.process_event(e);
    }

    void controller::on_activate(const core::parameter_dict& params) {
        auto it = params.find("name");
        if (it != params.end()) {
            auto mach = hardware::registry::instance()->find_machine(it->second);
            if (mach == nullptr) {
                mach = hardware::registry::instance()->new_machine();
                mach->name(it->second);
            }
            machine(mach);
        }
    }

}