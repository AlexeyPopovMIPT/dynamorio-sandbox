#ifndef MYWRAP_IMPL
#define MYWRAP_IMPL

#include "mywrap_api.hpp"
#include "mywrap_role_config.hpp"
#include <cassert>
#include <optional>
#include "../../include/dr_api.h"
#include "../../ext/include/drwrap.h"

namespace mywrap {

/* ToolService implementation for DynamoRIO */
struct drService;

/* Inherit role from it */
using Func = ProtoFunc<drService>;

struct drService {
private:
    void *wrapcxt;
    enum {
        PRE_CB, POST_CB
    } state;
public:

    void to_post_cb (void *new_context) {
        wrapcxt = new_context;
        state = POST_CB;
    }

    explicit drService (void *wrapcxt): wrapcxt(wrapcxt) {}

    std::optional<WordSizeCell> get_arg (const char *arg_name) const {
         if (__builtin_expect(state == POST_CB, 0))
            return std::nullopt;

        const Func *func = static_cast<const Func *>(this);

        auto it = func_arg_to_num.find({func->name, arg_name});
        if (__builtin_expect(it == func_arg_to_num.end(), 0)) 
            return std::nullopt;
        return drwrap_get_arg (wrapcxt, it->second);
    }
    bool set_arg (const char *arg_name, WordSizeCell val) {
        if (__builtin_expect(state == POST_CB, 0))
            return false;

        const Func *func = static_cast<const Func *>(this);
        return 
        drwrap_set_arg (wrapcxt, func_arg_to_num[{func->name, arg_name}], val);
    }

    std::optional<WordSizeCell> get_retval () const {
        if (__builtin_expect(state == PRE_CB, 0))
            return std::nullopt;
        return drwrap_get_retval (wrapcxt);
    }
    bool set_retval (WordSizeCell val) {
        if (__builtin_expect(state == PRE_CB, 0))
            return false;
        return drwrap_set_retval (wrapcxt, val);
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
