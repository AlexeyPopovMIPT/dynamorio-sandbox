#ifndef MYWRAP_ROLE_CONFIG_HPP
#define MYWRAP_ROLE_CONFIG_HPP

#include <unordered_map>
#include <cstdlib> // abort ()

namespace mywrap {

    struct FuncArgKey {
        const char *func_name; // в таком виде, конечно, работать не будет, 
        const char *arg_name;  // нужно либо сравнивать строки через strcmp,
                               // либо перейти к каким-нибудь идентификаторам
    };

    // YAML file parsed
    std::unordered_map <FuncArgKey, int> func_arg_to_num;

    int load_config (const char *file) {
        // not implemented
        abort ();
    }
}

#endif // !MYWRAP_ROLE_CONFIG_HPP
