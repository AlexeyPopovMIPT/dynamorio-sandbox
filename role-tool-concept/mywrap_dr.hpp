#ifndef MYWRAP_IMPL
#define MYWRAP_IMPL

#include "mywrap_api.hpp"
#include "mywrap_role_config.hpp"
#include <cassert>
#include "../../include/dr_api.h" // TODO как нормально включать DynamoRIO
#include "../../ext/include/drwrap.h"

namespace mywrap {

struct drWrapContext;
struct drForwardList;

using IFunc = Proto_IFunc<drWrapContext, drForwardList>;

struct drForwardList {
    struct Node{
        WordSizeCell val;
        Node *next;
    };
    Node *head;

    void *allocate (size_t sz) {
        return dr_global_alloc (sz);
    }

    void cleance (void *ptr, size_t sz) {
        dr_global_free (ptr, sz);
    }

    void push (WordSizeCell value) {
        Node *new_node = (Node *) allocate (sizeof (Node));
        new_node->val = value;
        Node *old_head = head;
        head = new_node;
        head->next = old_head;
    }

    WordSizeCell pop () {
        assert (head != nullptr);
        WordSizeCell ret = head->val;
        Node *new_head = head->next;
        dr_global_free (head, sizeof (Node));
        head = new_head;
        return ret;
    }
};

struct drWrapContext {
    void *wrapcxt;

    WordSizeCell get_arg (const IFunc &func, const char *arg_name) const {
        return 
        drwrap_get_arg (wrapcxt, func_arg_to_num[{func.name, arg_name}]);
    }
    bool set_arg (const IFunc &func, const char *arg_name, WordSizeCell val) {
        return 
        drwrap_set_arg (wrapcxt, func_arg_to_num[{func.name, arg_name}], val);
    }

    WordSizeCell get_retval () const {
        return
        drwrap_get_retval (wrapcxt);
    }
    bool set_retval (WordSizeCell val) {
        return
        drwrap_set_retval (wrapcxt, val);
    }
};

} // namespace mywrap

#endif // !MYWRAP_IMPL
