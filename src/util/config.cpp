#include "util/config.hpp"
#include "curves/curve.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

static std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char character) {
                       return static_cast<char>(std::tolower(character));
                   });
    return value;
}

bool parse_bool(const std::string &value) {
    const std::string v = lowercase(value);
    if (v == "true" || v == "1" || v == "yes" || v == "on")
        return true;
    if (v == "false" || v == "0" || v == "no" || v == "off")
        return false;
    throw std::runtime_error("expected a boolean, got '" + value + "'");
}

std::string extract_title(std::string line) {
    const std::string name = line.substr(1, line.size() - 2);

    if (name.empty()) {
        throw std::runtime_error("empty section name");
    }

    return name;
}

uint32_t parse_uint(const std::string &value) {
    const long number = std::stol(value);

    if (number < 0) {
        throw std::runtime_error("number cannot be negative");
    }

    return static_cast<uint32_t>(number);
}

uint32_t parse_color(const std::string &value) {
    const unsigned long color = std::stoul(value, nullptr, 0);

    if (color > 0xFFFFFF) {
        throw std::runtime_error("color must be between 0x000000 and 0xFFFFFF");
    }

    return static_cast<uint32_t>(color);
}

void read_curve_section(std::ifstream &confFile, Curve &curve) {
    // Invariant: There exists an empty curve object at the end of curves. Fill
    // it now until finding EOF or another line with []. THere just return
    std::string line;
    std::string key, value;
    while (true) {
        const std::streampos line_start = confFile.tellg();

        if (!std::getline(confFile, line)) {
            return; // EOF
        }
        // Read everything until [] section or EOF
        if (line.empty() || line.front() == '#') {
            continue;
        }
        if (line.front() == '[') {
            confFile.seekg(line_start);
            return;
        }

        // Split into key and value
        const std::size_t separator = line.find('=');

        if (separator == std::string::npos) {
            throw std::runtime_error("expected key=value");
        }
        std::string key = line.substr(0, separator);
        std::string value = line.substr(separator + 1);

        if (key == "type") {
            const std::string type = lowercase(value);

            if (type == "hilbert") {
                curve.type = CurveType::Hilbert;
            } else if (type == "moore") {
                curve.type = CurveType::Moore;
            } else if (type == "peano") {
                curve.type = CurveType::Peano;
            } else if (type == "zorder" || type == "z-order") {
                curve.type = CurveType::ZOrder;
            } else if (type == "disk" || type == "circle") {
                curve.type = CurveType::Disk;
            } else {
                throw std::runtime_error("unknown curve type '" + value + "'");
            }

        } else if (key == "depth") {
            curve.depth = parse_uint(value);
        } else if (key == "left_margin") {
            curve.left_margin = parse_uint(value);

        } else if (key == "right_margin") {
            curve.right_margin = parse_uint(value);
        } else if (key == "top_margin") {
            curve.top_margin = parse_uint(value);

        } else if (key == "bottom_margin") {
            curve.bottom_margin = parse_uint(value);

        } else if (key == "line_width") {
            curve.line_width = parse_uint(value);

        } else if (key == "background_color") {
            curve.background_color = parse_color(value);

        } else if (key == "line_color") {
            curve.line_color = parse_color(value);

        } else if (key == "animation_type") {
            const std::string type = lowercase(value);

            if (type == "none") {
                curve.animation_type = AnimationType::None;
            } else if (type == "polished") {
                curve.animation_type = AnimationType::Polished;
            } else {
                throw std::runtime_error("unknown animation type '" + value +
                                         "'");
            }

        } else if (key == "animation_length") {
            curve.animation_length = parse_uint(value);

        } else if (key == "animation_size") {
            curve.animation_size = parse_uint(value);

        } else if (key == "animation_color") {
            curve.animation_color = parse_color(value);

        } else if (key == "step_size") {
            curve.step_size = parse_uint(value);

        } else if (key == "time_step") {
            curve.time_step = std::chrono::milliseconds(parse_uint(value));

        } else if (key == "curve_on") {
            curve.curve_on = parse_bool(value);
        } else if (key == "image_path") {
            curve.image_path =
                value; // empty string = no image, solid background
        } else {
            throw std::runtime_error("unknown configuration parameter '" + key +
                                     "' with value" + value + "!");
        }
    }
}

void read_config(std::vector<Curve> &curves, const std::string &path) {
    std::ifstream config_file(path); // Opens the file for reading
    if (!config_file) {
        throw std::runtime_error(path + ": unable to open file");
    }

    std::string line;

    while (std::getline(config_file, line)) {
        if (line.empty() || line.front() == '#') {
            continue;
        }
        if (line.front() == '[' && line.back() == ']') {
            Curve curve = {};
            curve.monitor_name = extract_title(line);

            read_curve_section(config_file, curve);
            bool duplicate = false;
            for (auto c : curves) {
                if (c.monitor_name == curve.monitor_name) {
                    // Found duplicate -> skip
                    duplicate = true;
                }
            }
            if (!duplicate) {
                curves.push_back(curve);
            }
        }

        if (curves.empty()) {
            throw std::runtime_error("setting appears before any [section]");
        }
    }

    if (curves.empty()) {
        throw std::runtime_error(path + ": no configuration sections found");
    }
}

int create_config() {
    namespace fs = std::filesystem;

    const char *home = std::getenv("HOME");
    if (!home || !*home) {
        std::fprintf(stderr, "HOME is not set\n");
        return -1;
    }

    const fs::path config_path =
        fs::path(home) / ".config/way2curV/config.conf";

    if (fs::exists(config_path)) {
        return 0;
    }

    std::error_code error;
    fs::create_directories(config_path.parent_path(), error);

    if (error) {
        std::fprintf(stderr, "Could not create config directory: %s\n",
                     error.message().c_str());
        return -1;
    }

    fs::copy_file(WAY2CURV_DEFAULT_CONFIG, config_path, fs::copy_options::none,
                  error);

    if (error) {
        std::fprintf(stderr, "Could not create config file: %s\n",
                     error.message().c_str());
        return -1;
    }

    return 0;
}

int configuration(std::vector<Curve> &curve_configs) {
    if (create_config() != 0) {
        return -1;
    }

    std::string conf_path = "";
    if (std::getenv("HOME") &&
        (std::filesystem::exists(std::string(std::getenv("HOME")) +
                                 "/.config/way2curV/config.conf"))) {

        conf_path =
            std::string(std::getenv("HOME")) + "/.config/way2curV/config.conf";
    } else {
        std::fprintf(stderr, "Failed to find the config.conf file");
        return -1;
    }

    try {
        read_config(curve_configs, conf_path);
    } catch (const std::exception &error) {
        std::fprintf(stderr, "%s\n", error.what());
        return -1;
    }
    return 0;
}