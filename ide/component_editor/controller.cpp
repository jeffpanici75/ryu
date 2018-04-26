//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <logger_factory.h>
#include <hardware/registry.h>
#include "controller.h"

namespace ryu::ide::component_editor {

    static logger* s_log = logger_factory::instance()->create(
            "component_editor::controller",
            logger::level::info);

    controller::controller(const std::string& name) : core::state(name) {
    }

    void controller::bind_events() {
        action_provider().register_handler(
            core::input_action::find_by_name("component_editor_leave"),
            [this](const core::event_data_t& data) {
                end_state();
                return true;
            });
    }

    void controller::on_deactivate() {
        _layout_panel->visible(false);
    }

    void controller::on_initialize() {
        define_actions();
        bind_events();
    }

    void controller::update_values() {
        _name_text_box->value("");
        _circuit_pick_list->value("");
        _address_text_box->value("");
        _description_text_editor->clear();

        if (_component == nullptr)
            return;

        _name_text_box->value(_component->name());
        _address_text_box->value(fmt::format(
            "{0:08x}",
            _component->address()));
        auto ic_instance = _component->ic();
        if (ic_instance != nullptr)
            _circuit_pick_list->value(ic_instance->name());
    }

    void controller::define_actions() {
        auto leave_action = core::input_action::create_no_map(
            "component_editor_leave",
            "Internal",
            "Close the component editor and return to previous state.");
        if (!leave_action->has_bindings())
            leave_action->bind_keys({core::key_escape});
    }

    hardware::component* controller::component() {
        return _component;
    }

    bool controller::on_load(core::result& result) {
        _layout_panel = core::view_factory::create_loadable_view(
                this,
                "loadable-view",
                context()->font_family(),
                &context()->palette(),
                context()->prefs(),
                ide::colors::info_text,
                ide::colors::fill_color,
                result,
                "assets/views/component-editor.yaml");
        s_log->result(result);

        _close_button = _layout_panel->find_by_name<core::button>("close-button");
        _name_text_box = _layout_panel->find_by_name<core::text_box>("name-text-box");
        _circuit_pick_list = _layout_panel->find_by_name<core::pick_list>("ic-pick-list");
        _address_text_box = _layout_panel->find_by_name<core::text_box>("address-text-box");
        _description_text_editor = _layout_panel->find_by_name<core::text_editor>("description-text-editor");

        return !result.is_failed();
    }

    void controller::on_draw(core::renderer& surface) {
        _layout_panel->draw(surface);
    }

    void controller::on_resize(const core::rect& bounds) {
        _layout_panel->resize(bounds);
    }

    void controller::component(hardware::component* value) {
        _component = value;
        update_values();
    }

    void controller::on_activate(const core::parameter_dict& params) {
        _layout_panel->visible(true);

        auto it = params.find("machine-id");
        if (it != params.end()) {
            auto machine_id = boost::get<uint32_t>(it->second);
            auto mach = hardware::registry::instance()->find_machine(machine_id);
            it = params.find("component-id");
            if (it != params.end()) {
                auto component_id = boost::get<uint32_t>(it->second);
                component(mach->find_component(component_id));
            }
        }
    }

    void controller::on_update(uint32_t dt, core::pending_event_list& events) {
        _layout_panel->update(dt, events);
    }

}