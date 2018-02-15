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
#include <algorithm>
#include "piece_table.h"

namespace ryu::core {

    void piece_node_t::copy_elements(
            attr_line_list& lines,
            uint32_t begin,
            uint32_t end) {
        attr_span_list_t* current_line = nullptr;
        attr_span_t* current_span = nullptr;
        if (!lines.empty()) {
            current_line = &lines.back();
            if (!current_line->empty())
                current_span = &current_line->back();
        } else {
            lines.push_back(attr_span_list_t {});
            current_line = &lines.back();
        }

        std::stringstream stream {};
        for (size_t i = begin; i < end; i++) {
            auto& element = buffer->elements[start + i];
            if (current_span != nullptr) {
                if (current_span->attr != element.attr) {
                    current_span->text += stream.str();
                    stream.str("");
                    current_line->push_back(attr_span_t {element.attr});
                    current_span = &current_line->back();
                }
            } else {
                current_line->push_back(attr_span_t {element.attr});
                current_span = &current_line->back();
            }
            if (element.safe_value(stream)) {
                current_span->text += stream.str();
                stream.str("");
                lines.push_back(attr_span_list_t {});
                current_line = &lines.back();
                current_span = nullptr;
            }
        }

        if (current_span != nullptr)
            current_span->text += stream.str();
    }

    void piece_node_t::copy_elements(attr_line_list& lines) {
        copy_elements(lines, 0, length);
    }

    ///////////////////////////////////////////////////////////////////////////

    void piece_table_t::undo() {
        _pieces.undo();
    }

    void piece_table_t::redo() {
        _pieces.redo();
    }

    void piece_table_t::clear() {
        _pieces.clear();
        _changes.clear();
        _original.clear();
    }

    void piece_table_t::rebuild() {
        _lines.clear();
        auto current_node = _pieces.head();
        while (current_node != nullptr) {
            current_node->copy_elements(_lines);
            current_node = current_node->next;
        }
    }

    void piece_table_t::checkpoint() {
        _pieces.checkpoint();
    }

    const selection_t& piece_table_t::add_selection(
            selection_t::types type,
            uint32_t start,
            uint32_t length,
            const std::string& name) {
        _selections.push_back(selection_t{name, start, length, type});
        return _selections.back();
    }

    const attr_line_list& piece_table_t::sequence() {
        return _lines;
    }

    const selection_list& piece_table_t::selections() const {
        return _selections;
    }

    void piece_table_t::load(const piece_table_buffer_t& buffer) {
        clear();
        _original = buffer;
        _lines.clear();

        if (!buffer.empty()) {
            _pieces.add_tail(std::make_shared<piece_node_t>(
                    0,
                    _original.elements.size(),
                    &_original));
        }
    }

    void piece_table_t::delete_at(uint32_t offset, uint32_t length) {
        if (length == 0)
            return;

        auto initial_piece_find_result = _pieces.find_for_offset(offset);
        auto final_piece_find_result = _pieces.find_for_offset(offset + length);

        if (initial_piece_find_result.data == final_piece_find_result.data) {
            if (initial_piece_find_result.offset.start == offset) {
                auto cloned_node = _pieces.clone_and_swap(initial_piece_find_result.data);
                cloned_node->start += length;
                cloned_node->length -= length;
            } else if (final_piece_find_result.offset.end == offset) {
                auto cloned_node = _pieces.clone_and_swap(initial_piece_find_result.data);
                cloned_node->length -= length;
            } else {
                auto new_start = (initial_piece_find_result.data->start
                                 + (offset - initial_piece_find_result.data->start)
                                 + length);

                auto new_piece = std::make_shared<piece_node_t>(
                        new_start,
                        initial_piece_find_result.data->length - new_start,
                        &_changes);

                auto cloned_piece = _pieces.clone_and_swap(initial_piece_find_result.data);
                cloned_piece->length = offset;

                _pieces.insert_after(cloned_piece, new_piece);
            }
        } else {
            uint32_t node_start_offset = 0;
            uint32_t delete_offset_start = offset;
            uint32_t delete_offset_end = offset + length;
            auto current_node = _pieces.head();
            while (current_node != nullptr
                && delete_offset_start < delete_offset_end) {

                auto node_end_offset = node_start_offset + current_node->length;
                auto elements_remaining = delete_offset_start - node_start_offset;

                if (!(delete_offset_start >= node_start_offset
                   && delete_offset_start <= node_end_offset)) {
                    goto next_node;
                }

                if (delete_offset_end < node_end_offset) {
                    auto element_removed_count = delete_offset_end - node_start_offset;
                    auto cloned_node = _pieces.clone_and_swap(current_node);
                    cloned_node->start += element_removed_count;
                    cloned_node->length -= element_removed_count;
                    goto next_node;
                }

                if (elements_remaining == 0) {
                    _pieces.swap_deleted_node(current_node);
                } else {
                    auto cloned_node = _pieces.clone_and_swap(current_node);
                    cloned_node->length -= cloned_node->length - elements_remaining;
                }

            next_node:
                node_start_offset += current_node->length;
                delete_offset_start = node_start_offset;
                current_node = current_node->next;
            }
        }

        _pieces.update_offsets();
    }

    void piece_table_t::remove_selection(const selection_t& selection) {
        _selections.erase(std::remove(
                _selections.begin(),
                _selections.end(),
                selection),
            _selections.end());
    }

    void piece_table_t::insert_at(uint32_t offset, const element_list_t& elements) {
        if (elements.empty())
            return;

        for (const auto& element : elements)
            _changes.elements.push_back(element);

        auto new_change_offset = _changes.size() - elements.size();
        auto find_result = _pieces.find_for_offset(offset);
        auto is_change_piece = find_result.data != nullptr ?
                               find_result.data->buffer->type == piece_table_buffer_t::changes :
                               false;
        switch (find_result.type) {
            case piece_find_result_t::none: {
                auto node = std::make_shared<piece_node_t>(
                        new_change_offset,
                        elements.size(),
                        &_changes);
                node->offset = offset_t {offset, static_cast<uint32_t>(offset + elements.size())};
                _pieces.add_tail(node);
                break;
            }
            case piece_find_result_t::first: {
                if (is_change_piece && find_result.data->end() + elements.size() == offset) {
                    auto cloned_node = _pieces.clone_and_swap(find_result.data);
                    cloned_node->length++;
                } else {
                    auto node = std::make_shared<piece_node_t>(
                            new_change_offset,
                            elements.size(), &_changes);
                    node->offset = offset_t {offset, static_cast<uint32_t>(offset + elements.size())};
                    _pieces.add_tail(node);
                }
                break;
            }
            case piece_find_result_t::final: {
                if (is_change_piece && find_result.data->end() + elements.size() == offset) {
                    auto cloned_node = _pieces.clone_and_swap(find_result.data);
                    cloned_node->length++;
                } else {
                    auto node = std::make_shared<piece_node_t>(
                            new_change_offset,
                            elements.size(),
                            &_changes);
                    node->offset = offset_t {offset, static_cast<uint32_t>(offset + elements.size())};
                    _pieces.add_tail(node);
                }
                break;
            }
            case piece_find_result_t::medial: {
                if (new_change_offset == find_result.data->end()
                &&  find_result.offset.end == offset) {
                    auto cloned_node = _pieces.clone_and_swap(find_result.data);
                    cloned_node->length++;
                    break;
                }

                auto new_piece = std::make_shared<piece_node_t>(
                        static_cast<uint32_t>(new_change_offset),
                        elements.size(),
                        &_changes);

                auto left_piece = _pieces.clone_and_swap(find_result.data);

                int32_t right_piece_length = std::max<int32_t>(
                        static_cast<int32_t>(left_piece->offset.end - (offset + 1)),
                        0);
                auto right_piece = std::make_shared<piece_node_t>(
                        offset + 1,
                        right_piece_length,
                        left_piece->buffer
                );

                int32_t left_piece_length = std::max<int32_t>(
                        static_cast<int32_t>(left_piece->offset.end - offset),
                        0);
                left_piece->length -= left_piece_length;

                _pieces.insert_after(left_piece, new_piece);
                _pieces.insert_after(new_piece.get(), right_piece);
                break;
            }
            default:
                // XXX: should never happen
                break;
        }

        _pieces.update_offsets();
    }

    attr_line_list piece_table_t::cut(const selection_t& selection) {
        auto lines = copy(selection);
        delete_selection(selection);
        return lines;
    }

    attr_line_list piece_table_t::copy(const selection_t& selection) {
        return sub_sequence(selection.start, selection.start + selection.length);
    }

    void piece_table_t::delete_selection(const selection_t& selection) {
        delete_at(selection.start, selection.length);
    }

    attr_line_list piece_table_t::sub_sequence(uint32_t start, uint32_t end) {
        attr_line_list lines;
        uint32_t node_start_offset = 0;
        auto current_node = _pieces.head();
        while (current_node != nullptr) {
            auto node_end_offset = node_start_offset + current_node->length;
            if (node_start_offset < start && node_end_offset < start) goto next_node;
            if (node_start_offset > end) break;

            if (end <= node_end_offset) {
                if (start >= node_start_offset && start <= node_end_offset) {
                    current_node->copy_elements(
                            lines,
                            start,
                            end - node_start_offset);
                } else {
                    current_node->copy_elements(
                            lines,
                            0,
                            end - node_start_offset);
                }
            } else if (start <= node_start_offset) {
                current_node->copy_elements(lines);
            } else {
                auto start_offset = start - current_node->start;
                current_node->copy_elements(
                        lines,
                        start_offset,
                        current_node->start + current_node->length);
            }
        next_node:
            node_start_offset += current_node->length;
            current_node = current_node->next;
        }
        return lines;
    }

    void piece_table_t::paste(const selection_t& selection, const element_list_t& elements) {
        delete_at(selection.start, selection.length);
        insert_at(selection.start, elements);
    }

    ///////////////////////////////////////////////////////////////////////////

    void piece_list_t::undo() {
        if (_undo_stack.empty())
            return;

        auto node = _undo_stack.top();
        if (node == nullptr) {
            _head = nullptr;
            _tail = nullptr;
            _undo_stack.pop();
            return;
        }

        swap_node(node, _redo_stack);

        _undo_stack.pop();
    }

    void piece_list_t::redo() {
        if (_redo_stack.empty())
            return;

        auto node = _redo_stack.top();
        if (node == nullptr) {
            _head = nullptr;
            _tail = nullptr;
            _redo_stack.pop();
            return;
        }

        swap_node(node, _undo_stack);

        _redo_stack.pop();
    }

    void piece_list_t::clear() {
        _head = nullptr;
        _tail = nullptr;
        while (!_undo_stack.empty())
            _undo_stack.pop();
        while (!_redo_stack.empty())
            _redo_stack.pop();
        _owned_pieces.clear();
    }

    void piece_list_t::checkpoint() {
        clear_stack(_undo_stack);
        clear_stack(_redo_stack);
    }

    size_t piece_list_t::size() const {
        size_t count = 0;
        auto current_node = _head;
        while (current_node != nullptr) {
            ++count;
            current_node = current_node->next;
        }
        return count;
    }

    void piece_list_t::swap_node(
            piece_node_t* node,
            piece_node_stack& target_stack) {
        if (node->prev == nullptr) {
            auto current_head = _head;
            target_stack.push(current_head);
            _head = node;
            if (current_head == _tail) {
                _tail = _head;
            }
        } else if (node->next == nullptr) {
            auto current_tail = _tail;
            target_stack.push(current_tail);
            _tail = node;
            if (current_tail == _head) {
                _head = _tail;
            }
        }

        if (node->prev != nullptr) {
            auto prev_node = node->prev;
            prev_node->next = node;
        }

        if (node->next != nullptr) {
            auto next_node = node->next;
            next_node->prev = node;
        }
    }

    void piece_list_t::update_offsets() {
        uint32_t offset = 0;
        auto current_node = _head;
        while (current_node != nullptr) {
            current_node->offset = offset_t {offset, offset + current_node->length};
            offset += current_node->length;
            current_node = current_node->next;
        }
    }

    void piece_list_t::swap_deleted_node(piece_node_t* node) {
        _undo_stack.push(node);

        if (node->prev == nullptr) {
            auto current_head = _head;
            _head = node;
            if (current_head == _tail) {
                _tail = _head;
            }
        } else if (node->next == nullptr) {
            auto current_tail = _tail;
            _tail = node;
            if (current_tail == _head) {
                _head = _tail;
            }
        }

        auto current_node_prev = node->prev;
        if (node->prev != nullptr) {
            node->prev->next = node->next;
        }
        if (node->next != nullptr) {
            node->next->prev = current_node_prev;
        }
    }

    void piece_list_t::clear_stack(piece_node_stack& stack) {
        while (!stack.empty()) {
            auto top = stack.top();
            auto it = _owned_pieces.begin();
            for (; it != _owned_pieces.end(); ++it) {
                if ((*it).get() == top) {
                    _owned_pieces.erase(it);
                    break;
                }
            }
            stack.pop();
        }
    }

    void piece_list_t::add_tail(const piece_node_shared_ptr& piece) {
        auto raw_ptr = piece.get();
        _owned_pieces.insert(piece);
        if (_head == nullptr) {
            _undo_stack.push(nullptr);
            _head = raw_ptr;
            _tail = raw_ptr;
        } else {
            _undo_stack.push(_tail);
            raw_ptr->prev = _tail;
            _tail->next = raw_ptr;
            _tail = raw_ptr;
        }
    }

    void piece_list_t::insert_head(const piece_node_shared_ptr& piece) {
        auto raw_ptr = piece.get();
        _owned_pieces.insert(piece);
        if (_head == nullptr) {
            _undo_stack.push(nullptr);
            _head = raw_ptr;
            _tail = raw_ptr;
        } else {
            _undo_stack.push(_head);
            raw_ptr->next = _head;
            _head->prev = raw_ptr;
            _head = raw_ptr;
        }
    }

    piece_node_t* piece_list_t::clone_and_swap(piece_node_t* node) {
        auto clone_node = std::make_shared<piece_node_t>(
                node->start,
                node->length,
                node->buffer);
        clone_node->offset = node->offset;
        _owned_pieces.insert(clone_node);
        auto clone_raw_ptr = clone_node.get();
        if (node->prev != nullptr) {
            node->prev->next = clone_raw_ptr;
            clone_raw_ptr->prev = node->prev;
        } else {
            _head = clone_raw_ptr;
        }
        if (node->next != nullptr) {
            node->next->prev = clone_raw_ptr;
            clone_raw_ptr->next = node->next;
        } else {
            _tail = clone_raw_ptr;
        }
        _undo_stack.push(node);
        return clone_raw_ptr;
    }

    piece_find_result_t piece_list_t::find_for_offset(uint32_t offset) {
        piece_find_result_t result {};
        if (empty()) {
            return result;
        }

        auto total_linear_length = total_length();

        if (offset == 0) {
            result.data = _head;
            result.type = piece_find_result_t::types::first;
            result.offset = result.data->offset;
        } else if (offset >= total_linear_length) {
            result.data = _tail;
            result.type = piece_find_result_t::types::final;
            result.offset = result.data->offset;
        } else {
            auto current_node = _head;
            while (current_node != nullptr) {
                if (current_node->offset.within(offset)) {
                    break;
                }
                current_node = current_node->next;
            }
            result.data = current_node;
            result.type = piece_find_result_t::types::medial;
            if (current_node != nullptr)
                result.offset = current_node->offset;
        }

        return result;
    }

    void piece_list_t::insert_after(piece_node_t* node, const piece_node_shared_ptr& piece) {
        _owned_pieces.insert(piece);

        const auto piece_raw_ptr = piece.get();
        auto node_next = node->next;

        piece_raw_ptr->next = node_next;
        piece_raw_ptr->prev = node;
        if (node_next != nullptr)
            node_next->prev = piece_raw_ptr;
        else
            _tail = piece_raw_ptr;

        node->next = piece_raw_ptr;
    }

    void piece_list_t::insert_before(piece_node_t* node, const piece_node_shared_ptr& piece) {
        _owned_pieces.insert(piece);

        const auto piece_raw_ptr = piece.get();
        auto node_prev = node->prev;

        piece_raw_ptr->next = node;
        piece_raw_ptr->prev = node_prev;
        if (node_prev != nullptr)
            node_prev->next = piece_raw_ptr;
        else
            _head = piece_raw_ptr;

        node->prev = piece_raw_ptr;
    }

};