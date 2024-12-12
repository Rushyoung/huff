#pragma once

#include <string>

namespace args {

enum class type {
    string,
    number,
    boolean
};

struct option {
    std::string short_name;
    std::string long_name;
    std::string description;
    type arg_type;
    std::string value;
    bool is_required;

    option(std::string long_name, std::string description):
        short_name(""),
        long_name(long_name),
        description(description),
        value(""),
        is_required(false),
        arg_type(type::string) {}
    
    option(std::string short_name, std::string long_name, std::string description):
        short_name(short_name),
        long_name(long_name),
        description(description),
        value(""),
        is_required(false),
        arg_type(type::string) {}

    option as(type _type) {
        arg_type = _type;
        return *this;
    }

    option default_val(std::string _value) {
        is_required = false;
        value = _value;
        return *this;
    }

    option required() {
        is_required = true;
        return *this;
    }
};

}