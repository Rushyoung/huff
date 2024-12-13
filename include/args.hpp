#pragma once

#include <map>
#include <string>
#include <vector>

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

    option as(type _type);

    option default_val(std::string _value);

    option required();
};

struct __data {
    std::string value;
    __data(std::string value): value(value) {}

    template <typename T>
    T as();
};
template <> int         __data::as<int>         ();
template <> double      __data::as<double>      ();
template <> bool        __data::as<bool>        ();
template <> char*       __data::as<char*>       ();
template <> std::string __data::as<std::string> ();



class arg {
public:
    struct arg_data {
        bool is_marked;
        std::string value;
        type arg_type;
        arg_data():
            value(""),
            arg_type(type::string),
            is_marked(false) {}
        arg_data(std::string value, type arg_type): 
            value(value), 
            arg_type(arg_type),
            is_marked(false) {}
        void mark();
    };

private:
    std::map<std::string, arg_data>    data;
public:
    arg():
        data(){};
    arg(std::map<std::string, arg_data> data):
        data(data){}

    bool has(std::string name);

    __data operator[](std::string name);
};


class arg_flow{
private:
    int current_arg;
    int current_int;
    int arg_size;
    std::vector<std::string> arg_str;
public:
    struct token {
        enum class type {
            option,
            value,
            end
        };
        type tk_type;
        std::string value;
        token(type t, std::string v=""): tk_type(t), value(v) {}
    };

    arg_flow(int arg_size, std::vector<std::string> get_arg):
        current_arg(1),
        current_int(0),
        arg_size(arg_size),
        arg_str(get_arg) {}
    
    token next();
};


class settings {
private:
    std::vector<option> options;

    bool legal_option(std::string name);
    bool legal_short(std::string short_name);
    bool legal_long(std::string long_name);
public:
    template <typename... opts>
    settings(opts... _options): options{_options...} {}

    arg parse(int arg_size, char *arg_str[]);

    void print_help();
};


}