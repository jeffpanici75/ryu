//
// Ryu
//
// Copyright (C) 2017 Jeff Panici
// All Rights Reserved.
//
// See the LICENSE file for details about the license covering
// this source code file.
//

#include <algorithm>
#include <hardware/machine.h>
#include <common/string_support.h>
#include <boost/algorithm/string.hpp>
#include "project.h"
#include "project_file.h"

namespace ryu::core {

    project_file project_file::load(
            core::result& result,
            YAML::Node& node) {
        auto id = node["id"];
        if (id == nullptr) {
            result.add_message(
                    "C031",
                    "project_file id is a required value",
                    true);
            return {};
        }

        auto path = node["path"];
        if (path == nullptr) {
            result.add_message(
                    "C031",
                    "project_file path is a required value",
                    true);
            return {};
        }

        auto type = node["type"];
        if (type == nullptr) {
            result.add_message(
                    "C031",
                    "project_file type is a required value",
                    true);
            return {};
        }

        project_file file(
                id.as<uint32_t>(),
                fs::path(path.as<std::string>()),
                project_file_type::code_to_type(type.as<std::string>()));

        auto sequence_node = node["sequence"];
        if (sequence_node != nullptr && sequence_node.IsScalar()) {
            file.sequence(sequence_node.as<uint16_t>());
        }

        auto should_assemble_node = node["should_assemble"];
        if (should_assemble_node != nullptr && should_assemble_node.IsScalar()) {
            file.should_assemble(should_assemble_node.as<bool>());
        }

        return file;
    }

    project_file::project_file(
            uint32_t id,
            const fs::path& path,
            project_file_type::codes type) : _id(id),
                                             _path(path),
                                             _type(type) {
        core::id_pool::instance()->mark_used(_id);
        switch (_type) {
            case project_file_type::source:
                _sequence = 3;
                break;
            case project_file_type::data:
            case project_file_type::tiles:
            case project_file_type::actor:
            case project_file_type::module:
            case project_file_type::sample:
            case project_file_type::sprites:
            case project_file_type::palette:
            case project_file_type::background:
                _sequence = 2;
                _should_assemble = true;
                break;
            case project_file_type::environment:
                _sequence = 1;
                _should_assemble = true;
                break;
            default:
                break;
        }
    }

    bool project_file::read(
            core::result& result,
            std::iostream& stream) {
        fs::path file_path = full_path();
        try {
            std::ifstream file;
            file.open(file_path.string());
            stream << file.rdbuf();
            file.close();
        } catch (std::exception& e) {
            result.add_message(
                    "P001",
                    fmt::format("unable to read project_file: {}", e.what()),
                    true);
        }
        return true;
    }

    bool project_file::write(
            core::result& result,
            std::iostream& stream) {
        fs::path file_path = full_path();
        try {
            std::ofstream file;
            file.open(file_path.string());
            file << stream.rdbuf();
            file.close();
        } catch (std::exception& e) {
            result.add_message(
                    "P001",
                    fmt::format("unable to write project_file: {}", e.what()),
                    true);
        }
        return true;
    }

    // XXX: consider using ctemplate and assets/templates/*.tmpl
    //      to make this feature nicer
    bool project_file::create_stub_file(
            core::result& result,
            const fs::path& path) {
        fs::path file_path = path;

        if (!file_path.is_absolute()) {
            if (_type == project_file_type::environment) {
                file_path = project::find_project_root()
                        .append(".ryu")
                        .append(file_path.string());
            } else {
                file_path = project::find_project_root()
                        .append(file_path.string());
            }
        }

        if (fs::exists(file_path)) {
            return true;
        }

        if (!fs::is_directory(file_path.parent_path())) {
            boost::system::error_code ec;
            if (!fs::create_directories(file_path.parent_path(), ec)) {
                result.add_message(
                    "C031",
                    fmt::format("unable to create directory: {}", ec.message()),
                    true);
                return false;
            }
        }

        std::stringstream stream;
        stream << "*\n";
        stream << "* " << file_path.filename() << "\n";
        stream << "*\n\n";

        write(result, stream);

        return !result.is_failed();
    }

    bool project_file::dirty() const {
        return _dirty;
    }

    uint32_t project_file::id() const {
        return _id;
    }

    fs::path project_file::full_path() const {
        if (_type == project_file_type::environment) {
            return project::find_project_root()
                    .append(".ryu")
                    .append(_path.string())
                    .replace_extension(".env");
        } else {
            return project::find_project_root()
                    .append(_path.string());
        }
    }

    fs::path project_file::path() const {
        return _path;
    }

    void project_file::dirty(bool value) {
        _dirty = value;
    }

    std::string project_file::name() const {
        return _path.filename().string();
    }

    uint16_t project_file::sequence() const {
        return _sequence;
    }

    bool project_file::should_assemble() const {
        return _should_assemble;
    }

    void project_file::sequence(uint16_t value) {
        // XXX: i may regret this later....
        if (_type == project_file_type::codes::source)
            _sequence = value;
    }

    void project_file::should_assemble(bool flag) {
        if (flag != _should_assemble) {
            _should_assemble = flag;
            _dirty = true;
        }
    }

    void project_file::path(const fs::path& value) {
        if (value != _path) {
            _path = value;
            _dirty = true;
        }
    }

    project_file_type::codes project_file::type() const {
        return _type;
    }

    void project_file::type(project_file_type::codes value) {
        if (value != _type) {
            _type = value;
            _dirty = true;
        }
    }

    bool project_file::save(core::result& result, YAML::Emitter& emitter) {
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "id" << YAML::Value << _id;
        emitter << YAML::Key << "path" << YAML::Value << _path.string();
        emitter << YAML::Key << "type" << YAML::Value << project_file_type::type_to_code(_type);
        emitter << YAML::Key << "should_assemble" << YAML::Value << _should_assemble;
        emitter << YAML::Key << "sequence" << YAML::Value << _sequence;
        emitter << YAML::EndMap;
        return true;
    }

}