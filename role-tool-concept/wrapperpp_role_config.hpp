#pragma once
#ifndef WRAPPERPP_ROLE_CONFIG_HPP
#define WRAPPERPP_ROLE_CONFIG_HPP

#include <unordered_map>
#include <cstdlib>

namespace wrapperpp {

    struct FuncArgKey {
        const char *func_name;
        const char *arg_name;
    };

    std::unordered_map <FuncArgKey, int> func_arg_to_num; // сделать на шаблонах?

    int load_config (const char *file) {
        // not implemented
        abort ();
    }
}

#endif // !WRAPPERPP_ROLE_CONFIG_HPP
