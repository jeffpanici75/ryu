//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <sstream>
#include <common/string_support.h>
#include "document.h"

namespace ryu::core {

    document::document() {
        _piece_table.undo_manager(&_undo_manager);
    }

    void document::home() {
        _column = 0;
        raise_document_changed();
    }

    void document::clear() {
        _row = 0;
        _column = 0;
        _piece_table.load({});
        raise_document_changed();
    }

    void document::page_up() {
        _row -= _page_height;
        clamp_row();
        raise_document_changed();
    }

    void document::first_page() {
        _row = 0;
        raise_document_changed();
    }

    void document::page_down() {
        _row += _page_height;
        clamp_row();
        raise_document_changed();
    }

    void document::last_page() {
        _row = _rows - _page_height;
        clamp_row();
        raise_document_changed();
    }

    bool document::scroll_up() {
        --_row;
        auto clamped = clamp_row();
        raise_document_changed();
        return clamped;
    }

    bool document::clamp_row() {
        if (_row < 0) {
            _row = 0;
            return true;
        }

        auto bottom = _rows - _page_height;
        if (_row > bottom) {
            _row = bottom;
            return true;
        }

        return false;
    }

    position_t document::redo() {
        auto position = _piece_table.redo();
        if (!position.empty()) {
            move_to(
                static_cast<uint32_t>(position.row),
                static_cast<uint16_t>(position.column));
        }
        return position;
    }

    position_t document::undo() {
        auto position = _piece_table.undo();
        if (!position.empty()) {
            move_to(
                static_cast<uint32_t>(position.row),
                static_cast<uint16_t>(position.column));
        }
        return position;
    }

    bool document::scroll_down() {
        ++_row;
        auto clamped = clamp_row();
        raise_document_changed();
        return clamped;
    }

    bool document::scroll_right() {
        ++_column;
        auto clamped = clamp_column();
        raise_document_changed();
        return clamped;
    }

    int32_t document::row() const {
        return _row;
    }

    bool document::clamp_column() {
        if (_column < 0) {
            _column = 0;
            return true;
        }

        auto right = _columns - _page_width;
        if (_column > right) {
            _column = right;
            return true;
        }

        return false;
    }

    bool document::scroll_left() {
        --_column;
        auto clamped = clamp_column();
        raise_document_changed();
        return clamped;
    }

    bool document::is_line_empty() {
        // XXX: there has to be a better way to do this
        return _piece_table.sequence(virtual_row()).empty();
    }

    core::caret* document::caret() {
        return _caret;
    }

    fs::path document::path() const {
        return _path;
    }

    uint32_t document::rows() const {
        return _rows;
    }

    bool document::row(uint32_t row) {
        _row = row;
        auto clamped = clamp_row();
        raise_document_changed();
        return clamped;
    }

    int16_t document::column() const {
        return _column;
    }

    uint32_t document::offset() const {
        return static_cast<uint32_t>((virtual_row() * _columns) + virtual_column());
    }

    uint16_t document::find_line_end() {
        // XXX: there is a better way to do this with the lines
        auto elements = _piece_table
                .sequence(static_cast<uint32_t>(virtual_row()))
                .sequence();
        return static_cast<uint16_t>(elements.size());
    }

    uint16_t document::columns() const {
        return _columns;
    }

    void document::end(uint16_t column) {
        _column = _columns - column;
        raise_document_changed();
    }

    uint8_t document::page_width() const {
        return _page_width;
    }

    uint8_t document::page_height() const {
        return _page_height;
    }

    uint32_t document::virtual_row() const {
        return static_cast<uint32_t>(_row + _caret->row());
    }

    bool document::get(element_t& element) {
        auto elements = _piece_table
                .sequence(static_cast<uint32_t>(virtual_row()))
                .sequence();
        auto column = virtual_column();
        if (column < elements.size()) {
            element.attr = elements[column].attr;
            element.value = elements[column].value;
            return true;
        }
        return false;
    }

    bool document::column(uint16_t column) {
        _column = column;
        auto clamped = clamp_column();
        raise_document_changed();
        return clamped;
    }

    void document::raise_document_changed() {
        if (_document_changed_callback != nullptr)
            _document_changed_callback();
    }

    uint32_t document::virtual_column() const {
        return static_cast<uint32_t>(_column + _caret->column());
    }

    void document::caret(core::caret* value) {
        _caret = value;
    }

    void document::shift_left(uint16_t times) {
        _piece_table.delete_at(virtual_row(), virtual_column(), times);
    }

    void document::path(const fs::path& value) {
        _path = value;
    }

    void document::insert(element_list_t& value) {
        auto column = virtual_column();
        if (column + value.size() > _columns) {
            value.truncate(_columns - column);
        }
        _piece_table.insert_at(virtual_row(), column, value);
        _caret->column(static_cast<uint8_t>(_caret->column() + value.size()));
    }

    void document::move_to(uint32_t row, uint16_t column) {
        _row = (row / _page_height) * _page_height;
        _column = (column / _page_width) * _page_width;
        _caret->row(static_cast<uint8_t>(row - _row));
        _caret->column(static_cast<uint8_t>(column - _column));
        raise_document_changed();
    }

    void document::page_size(uint8_t height, uint8_t width) {
        _page_width = width;
        _page_height = height;

        _rows = std::max<uint32_t>(_page_height, _rows);
        _columns = std::max<uint8_t>(_page_width, _columns);

        if (_caret != nullptr) {
            _caret->page_size(height, width);
        }

        raise_document_changed();
    }

    void document::document_size(uint32_t rows, uint16_t columns) {
        _rows = rows;
        _columns = columns;
        raise_document_changed();
    }

    void document::shift_right(const attr_t& attr, uint16_t times) {
        _piece_table.insert_at(
                virtual_row(),
                virtual_column(),
                element_list_t::from_string(attr, std::string(times, ' ')));
    }

    bool document::load(core::result& result, const fs::path& path) {
        _path = path;

        if (!fs::exists(_path)) {
            result.add_message(
                    "D001",
                    fmt::format("document does not exist: {}", _path.string()),
                    true);
            return false;
        }

        try {
            std::ifstream file(_path.string());
            if (file.is_open()) {
                load(result, file);
                file.close();
            }
        } catch (std::exception& e) {
            result.add_message(
                    "D001",
                    fmt::format("unable to load document: {}", e.what()),
                    true);
        }

        return !result.is_failed();
    }

    bool document::load(core::result& result, std::istream& stream) {
        clear();
        raise_document_changed();
        return true;
    }

    bool document::save(core::result& result, std::ostream& stream) {
        raise_document_changed();
        return true;
    }

    bool document::save(core::result& result, const fs::path& path) {
        auto target_path = path;
        if (target_path.empty())
            target_path = _path;

        try {
            std::ofstream file(target_path.string());
            if (file.is_open()) {
                save(result, file);
                file.close();
            }
        } catch (std::exception& e) {
            result.add_message(
                    "D001",
                    fmt::format("unable to save document: {}", e.what()),
                    true);
        }

        return !result.is_failed();
    }

    attr_span_list_t document::line_at(uint32_t row, uint32_t start, uint32_t end) {
        return _piece_table.sub_sequence(row, start, end);
    }

    void document::on_document_changed(const document::document_changed_callable& callable) {
        _document_changed_callback = callable;
    }

}