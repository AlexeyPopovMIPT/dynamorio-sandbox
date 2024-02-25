#ifndef WRAPPERPP_IMPL
#define WRAPPERPP_IMPL

#include "wrapperpp_api.hpp"
#include "wrapperpp_role_config.hpp"
#include <cassert>
#include "../../include/dr_api.h"
#include "../../ext/include/drwrap.h"

namespace wrapperpp {

struct drWrapContext;
struct drForwardList;

using IFunc = Proto_IFunc<drWrapContext, drForwardList>;

struct drForwardList {
    struct Node{
        Arg val;
        Node *next;
    };
    Node *head;

    void *allocate (size_t sz) {
        return dr_global_alloc (sz);
    }

    void cleance (void *ptr, size_t sz) {
        dr_global_free (ptr, sz);
    }

    void push (Arg value) {
        Node *new_node = (Node *) allocate (sizeof (Node));
        new_node->val = value;
        Node *old_head = head;
        head = new_node;
        head->next = old_head;
    }

    Arg pop () {
        assert (head != nullptr);
        Arg ret = head->val;
        Node *new_head = head->next;
        dr_global_free (head, sizeof (Node));
        head = new_head;
        return ret;
    }
};

struct drWrapContext {
    void *wrapcxt;

    Arg get_arg (const IFunc &func, const char *arg_name) const {
        return 
        drwrap_get_arg (wrapcxt, func_arg_to_num[{func.name, arg_name}]);
    }
    bool set_arg (const IFunc &func, const char *arg_name, Arg val) {
        return 
        drwrap_set_arg (wrapcxt, func_arg_to_num[{func.name, arg_name}], val);
    }

    Arg get_retval () const {
        return
        drwrap_get_retval (wrapcxt);
    }
    bool set_retval (Arg val) {
        return
        drwrap_set_retval (wrapcxt, val);
    }
};

} // namespace wrapperpp

#endif // !WRAPPERPP_IMPL