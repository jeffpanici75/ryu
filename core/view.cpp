//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <utility>
#include "view.h"

namespace ryu::core {

    view::view(
            types::id type,
            const std::string& name,
            core::view_host* host) : _id(core::id_pool::instance()->allocate()),
                                     _name(name),
                                     _type(type),
                                     _container(host) {
    }

    view::~view() {
        core::id_pool::instance()->release(_id);
    }

    uint32_t view::id() const {
        return _id;
    }

    bool view::layout() const {
        return (_flags & config::flags::layout) != 0;
    }

    short view::index() const {
        return _index;
    }

    core::rect& view::bounds() {
        return _rect;
    }

    core::view* view::parent() {
        return _parent;
    }

    bool view::enabled() const {
        return (_flags & config::flags::enabled) != 0;
    }

    bool view::visible() const {
        return _container->is_visible()
               && (_flags & config::flags::visible) != 0;
    }

    bool view::tabstop() const {
        return (_flags & config::flags::tabstop) != 0;
    }

    bool view::focused() const {
        return _container->is_focused()
               && (_flags & config::flags::focused) != 0;
    }

    void view::clear_children() {
        for (auto child : _children)
            child->_parent = nullptr;
        _children.clear();
    }

    view_list& view::children() {
        return _children;
    }

    void view::requires_layout() {
        auto container = parent();
        if (container != nullptr)
            container->layout(true);
        else
            layout(true);
    }

    void view::layout(bool value) {
        if (value)
            _flags |= config::flags::layout;
        else
            _flags &= ~config::flags::layout;
    }

    core::padding& view::margin() {
        return _margin;
    }

    void view::index(short value) {
        _index = value;
    }

    void view::on_focus_changed() {
    }

    core::view* view::find_root() {
        auto current = this;
        while (true) {
            auto next = current->parent();
            if (next == nullptr)
                break;
            current = next;
        }
        return current;
    }

    core::palette* view::palette() {
        return _palette;
    }

    void view::enabled(bool value) {
        if (value)
            _flags |= config::flags::enabled;
        else
            _flags &= ~config::flags::enabled;
    }

    core::padding& view::padding() {
        return _padding;
    }

    void view::visible(bool value) {
        if (value)
            _flags |= config::flags::visible;
        else
            _flags &= ~config::flags::visible;

        for (auto child : _children)
            child->visible(value);
    }

    void view::tabstop(bool value) {
        if (value)
            _flags |= config::flags::tabstop;
        else
            _flags &= ~config::flags::tabstop;
    }

    std::string view::name() const {
        return _name;
    }

    core::rect view::client_bounds() {
        auto rect = bounds();
        auto pad = padding();
        core::rect padded;
        padded.pos(rect.left() + pad.left(), rect.top() + pad.top());
        padded.size(rect.width() - pad.right(), rect.height() - pad.bottom());
        return padded;
    }

    uint8_t view::bg_color() const {
        return _bg_color;
    }

    uint8_t view::fg_color() const {
        return _fg_color;
    }

    uint8_t view::font_style() const {
        return _font_style;
    }

    void view::bg_color(uint8_t value) {
        _bg_color = value;
    }

    void view::fg_color(uint8_t value) {
        _fg_color = value;
    }

    view::types::id view::type() const {
        return _type;
    }

    void view::inner_focus(bool value) {
        if (value)
            _flags |= config::flags::focused;
        else
            _flags &= ~config::flags::focused;
        on_focus_changed();
    }

    core::dock::styles view::dock() const {
        return _dock;
    }

    void view::font_style(uint8_t styles) {
        _font_style = styles;
    }

    void view::add_child(core::view* child) {
        if (child == nullptr)
            return;
        child->_parent = this;
        _children.push_back(child);
    }

    view::sizing::types view::sizing() const {
        return _sizing;
    }

    void view::dock(core::dock::styles style) {
        _dock = style;
    }

    void view::draw(core::renderer& renderer) {
        if (!visible())
            return;

        if (layout()) {
            resize(renderer.bounds());
        }

        on_draw(renderer);
        draw_children(renderer);
    }

    void view::remove_child(core::view* child) {
        if (child == nullptr)
            return;
        _children.erase(std::find(_children.begin(), _children.end(), child));
    }

    void view::bounds(const core::rect& value) {
        _rect = value;
    }

    void view::palette(core::palette* palette) {
        _palette = palette;
    }

    const core::font_t* view::font_face() const {
        auto family = font_family();
        if (family == nullptr)
            return nullptr;
        return family->find_style(_font_style);
    }

    void view::sizing(view::sizing::types value) {
        _sizing = value;
    }

    void view::on_draw(core::renderer& renderer) {
    }

//    bool view::process_event(const SDL_Event* e) {
//        if (focused()) {
//            if (!visible()) {
//                const auto* current = this;
//                while (!current->visible()) {
//                    if (current->_on_tab_callable != nullptr) {
//                        current = current->_on_tab_callable();
//                    } else {
//                        break;
//                    }
//                }
//                find_root()->focus(current);
//            } else {
//                if (e->type == SDL_KEYDOWN) {
//                    switch (e->key.keysym.sym) {
//                        case SDLK_TAB: {
//                            if (_on_tab_callable != nullptr) {
//                                const auto* next_view = _on_tab_callable();
//                                if (next_view != nullptr) {
//                                    find_root()->focus(next_view);
//                                    return true;
//                                }
//                            }
//                            break;
//                        }
//                    }
//                }
//                if (on_process_event(e))
//                    return true;
//            }
//        }
//
//        for (auto child : _children) {
//            if (child->process_event(e))
//                return true;
//        }
//
//        return false;
//    }

    void view::focus(const core::view* target) {
        if (target == nullptr)
            return;

        for (auto child : _children)
            child->focus(target);

        inner_focus(target->_id == this->_id);
    }

    // XXX: this works, but it has be to invoked manually on the view who needs the notification
    //      i'm worried i'll forget this is here....need to find a way to refactor view's interface
    //      so we invoke an initialize method outside of the constructor.  maybe a static ctor?
    void view::listen_for_on_container_change() {
        _container->on_change([this](view_host::change_reason_flags flags) {
            on_focus_changed();
        });
    }

    core::view* view::get_child_at(size_t index) {
        if (index < _children.size())
            return _children[index];
        return nullptr;
    }

    core::font_family* view::font_family() const {
        return _font;
    }

    void view::margin(const core::padding& value) {
        _margin = value;
    }

    void view::padding(const core::padding& value) {
        _padding = value;
    }

    void view::font_family(core::font_family* font) {
        _font = font;
    }

    void view::draw_children(core::renderer& renderer) {
        for (auto child : _children)
            child->draw(renderer);
    }

    void view::resize(const core::rect& context_bounds) {
        layout(false);
        on_resize(context_bounds);
        for (auto child : _children)
            child->resize(context_bounds);
    }

    void view::on_resize(const core::rect& context_bounds) {
    }

    void view::on_tab(const view::on_tab_callable& callable) {
        _on_tab_callable = callable;
    }

}