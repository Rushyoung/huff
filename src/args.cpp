#include "args.hpp"

#include <iostream>

namespace args {

/**
 * option class
 */
option option::as(type _type) {
    arg_type = _type;
    return *this;
}

option option::default_val(std::string _value) {
    is_required = false;
    value = _value;
    return *this;
}

option option::required() {
    is_required = true;
    return *this;
}


/**
 * __data class
 */
template <>
number __data::as<number>() {
    return std::stod(value);
}

template <>
boolean __data::as<boolean>() {
    if(value == "true") {
        return true;
    } else if(value == "false") {
        return false;
    } else {
        throw std::invalid_argument("Invalid boolean value: " + value);
    }
}

template <>
string __data::as<string>() {
    return value.c_str();
}


/**
 * arg class
 */

void arg::arg_data::mark() {
    is_marked = true;
    if(arg_type == type::boolean) {
        value = "true";
    }
}

bool arg::has(std::string name) {
    auto it = data.find(name);
    return it != data.end() and (it->second.is_marked or not it->second.value.empty());
}

__data arg::operator[](std::string name) {
    auto it = data.find(name);
    if(it == data.end()) {
        throw std::invalid_argument("Invalid option name: " + name);
    }
    return __data(it->second.value);
}


/**
 * arg_flow class
 */
arg_flow::token arg_flow::next() {
    if(current_arg >= arg_size) {
        return arg_flow::token(arg_flow::token::type::end);
    }

    if(current_int == 0) {
        if(arg_str[current_arg][0] == '-'){
            if(arg_str[current_arg][1] == '-'){
                current_int = 0;
                current_arg++;
                return arg_flow::token(arg_flow::token::type::option,
                                        arg_str[current_arg - 1].substr(2));
            } else {
                current_int = 2;
                return arg_flow::token(arg_flow::token::type::option, arg_str[current_arg].substr(1, 1));
            }
        } else {
            current_arg++;
            return arg_flow::token(arg_flow::token::type::value,
                                    arg_str[current_arg - 1]);
        }
    } else {
        if(current_int >= arg_str[current_arg].size()) {
            current_int = 0;
            current_arg++;
        }
        int idx = current_int;
        current_int = 0;
        current_arg++;
        return arg_flow::token(arg_flow::token::type::value,
                               arg_str[current_arg - 1].substr(idx));
    }
}

/**
 * settings class
 */
bool settings::legal_option(std::string name) {
    for(auto &c: name) {
        if(not isalnum(c) and c != '_') {
            return false;
        }
    }
    return true;
}

bool settings::legal_short(std::string short_name) {
    return short_name[0] == '-' and legal_option(short_name.substr(1));
}

bool settings::legal_long(std::string long_name) {
    if(long_name.size() < 3) {
        return false;
    }
    return long_name.substr(0, 2) == "--" and legal_option(long_name.substr(2));
}


/**
 * parse function
 */
arg settings::parse(int argc, char *args[]) {
    std::map<std::string, arg::arg_data> data;
    std::map<std::string, std::string> short_to_long;
    program_name = args[0];
    std::vector<std::string> argv;
    for(int i = 0; i < argc; i++) {
        argv.push_back(std::string(args[i]));
    }

    for(auto &opt: options) {
        if(opt.short_name != "" and not legal_short(opt.short_name)) {
            throw std::invalid_argument("Invalid short option name: " + opt.short_name);
        }
        if(not legal_long(opt.long_name)) {
            throw std::invalid_argument("Invalid long option name: " + opt.long_name);
        }
        if(opt.short_name != "") {
            short_to_long[opt.short_name.substr(1)] = opt.long_name.substr(2);
        }
        data[opt.long_name.substr(2)] = arg::arg_data(opt.value, opt.arg_type);
    }
    
    // and -h and --help
    short_to_long["h"] = "help";

    arg_flow flow(argc, argv);
    std::vector<option> parsed_options;
    std::string last_option = "";
    for(auto tk = flow.next(); tk.tk_type != arg_flow::token::type::end; tk = flow.next()) {
        if(tk.tk_type == arg_flow::token::type::option) {
            std::string opt_name = tk.value;
            if(opt_name.size() == 1){
                auto it = short_to_long.find(opt_name);
                if(it == short_to_long.end()) {
                    throw std::invalid_argument("Invalid short option name: " + opt_name);
                }
                opt_name = it->second;
            }
            // check if option is help
            if(opt_name == "help") {
                print_help();
            }
            auto it = data.find(opt_name);
            if(it == data.end()) {
                throw std::invalid_argument("Invalid option name: " + tk.value);
            }
            it->second.mark();
            last_option = opt_name;
        } else {
            if(last_option == "") {
                throw std::invalid_argument("Value without option: " + tk.value);
            }
            data[last_option].value = tk.value;
            last_option = "";
        }
    }
    return arg(data);
}

std::string settings::get_name() {
    auto flash = program_name.rfind("/");
    if(flash != std::string::npos) {
        return program_name.substr(flash + 1);
    }
    auto reflash = program_name.rfind("\\");
    if(reflash != std::string::npos) {
        return program_name.substr(reflash + 1);
    }
    return program_name;
}

void settings::print_help() {
    std::cout << "Usage:   " << get_name() << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    for(auto &opt: options) {
        if(opt.short_name == "") {
            std::cout << "      " << opt.long_name << ": " << opt.description << std::endl;
        } else {
            std::cout << "  " << opt.short_name << ", " << opt.long_name << ": " << opt.description << std::endl;
        }
    }
    std::cout << "  -h, --help: Print this help message" << std::endl;
    exit(0);
}


}