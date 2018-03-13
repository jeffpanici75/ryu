//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//

#include "state.h"
#include "engine.h"
#include "context.h"
#include "id_pool.h"

namespace ryu::core {

    state::state(
            const std::string& name,
            bool render_parent) : _id(core::id_pool::instance()->allocate()),
                                  _name(name),
                                  _render_parent(render_parent) {
    }

    state::~state() {
        core::id_pool::instance()->release(_id);
    }

    void state::end_state() {
        _context->pop_state();
    }

    void state::deactivate() {
        on_deactivate();
        if (_state_change_callback != nullptr)
            _state_change_callback(change_reasons::focus | change_reasons::visibility);
    }

    uint32_t state::id() const {
        return _id;
    }

    void state::on_deactivate() {
    }

    bool state::is_focused() const {
        return context()->peek_state() == _id;
    }

    bool state::is_visible() const {
        return context()->peek_state() == _id;
    }

    core::context* state::context() {
        return _context;
    }

    void state::update(uint32_t dt) {
        on_update(dt);
    }

    std::string state::name() const {
        return _name;
    }

    bool state::render_parent() const {
        return _render_parent;
    }

    bool state::is_initialized() const {
        return _initialized;
    }

    void state::context(core::context* value) {
        _context = value;
    }

    void state::draw(core::renderer& renderer) {
        on_draw(renderer);
    }

    void state::resize(const core::rect& bounds) {
        if (!_initialized)
            return;
        on_resize(bounds);
    }

    void state::on_resize(const core::rect& bounds) {
    }

    void state::initialize(const core::rect& bounds) {
        if (!_initialized) {
            on_initialize();
            _initialized = true;
            resize(bounds);
        }
    }

    void state::erase_blackboard(const std::string& name) {
        _context->erase_blackboard(name);
    }

    void state::activate(const core::parameter_dict& params) {
        on_activate(params);
        if (_state_change_callback != nullptr)
            _state_change_callback(change_reasons::focus | change_reasons::visibility);
    }

    void state::on_activate(const core::parameter_dict& params) {
    }

    std::string state::blackboard(const std::string& name) const {
        return _context->blackboard(name);
    }

    void state::blackboard(const std::string& name, const std::string& value) {
        _context->blackboard(name, value);
    }

    void state::transition_callback(const state_transition_callable& callback) {
        _callback = callback;
    }

    void state::on_change(const view_container::state_change_callable& callable) {
        _state_change_callback = callable;
    }

    bool state::transition_to(const std::string& name, const parameter_dict& params) {
        if (_callback)
            return _callback(name, params);
        return false;
    }

}