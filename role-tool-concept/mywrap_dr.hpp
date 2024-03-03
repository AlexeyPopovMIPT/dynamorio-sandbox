#ifndef MYWRAP_IMPL
#define MYWRAP_IMPL

#include "mywrap_api.hpp"
#include "mywrap_role_config.hpp"
#include <cassert>
#include "../../include/dr_api.h" // TODO как нормально включать DynamoRIO
#include "../../ext/include/drwrap.h"

namespace mywrap {

struct drService;

using Func = ProtoFunc<drService>;

struct drService {
    void *wrapcxt;

    explicit drService (void *wrapcxt): wrapcxt(wrapcxt) {}

    WordSizeCell get_arg (const char *arg_name) const {
        const Func *func = static_cast<const Func *>(this);
        return 
        drwrap_get_arg (wrapcxt, func_arg_to_num[{func->name, arg_name}]);
    }
    bool set_arg (const char *arg_name, WordSizeCell val) {
        const Func *func = static_cast<const Func *>(this);
        return 
        drwrap_set_arg (wrapcxt, func_arg_to_num[{func->name, arg_name}], val);
    }

    WordSizeCell get_retval () const {
        return
        drwrap_get_retval (wrapcxt);
    }
    bool set_retval (WordSizeCell val) {
        return
        drwrap_set_retval (wrapcxt, val);
    }

    static void *allocate (size_t sz) {
        return dr_global_alloc (sz);
    }

    static void cleance (void *ptr, size_t sz) {
        dr_global_free (ptr, sz);
    }

};

} // namespace mywrap

#endif // !MYWRAP_IMPL
