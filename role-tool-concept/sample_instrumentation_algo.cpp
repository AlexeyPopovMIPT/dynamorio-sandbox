#include <cstdio>
#include <algorithm> // std::copy
#include "mywrap_dr.hpp"

struct Send: public mywrap::IFunc {
    void wrap_pre () override {
        char *buffer = get_arg (*this, "Buffer");
        ssize_t sz = get_arg (*this, "Size");
        char *buffer_copy = (char *) allocate (sz);
        std::copy (buffer, buffer + sz, buffer_copy);
        push (buffer_copy);
        push (sz);
    }

    void wrap_post () override {
        ssize_t sz = pop();
        char *buffer_copy = pop();
        ssize_t retval = get_retval ();

        printf ("Call to %s for passing %llu bytes \"", name, sz);
        fwrite (buffer_copy, 1, sz, stdout);
        printf ("\" returned %lld\n", retval);

        cleance (buffer_copy, sz);
    }
};


