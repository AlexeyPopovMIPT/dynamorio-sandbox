#include <cstdio>
#include "dynamorio.hpp"

struct Send: public wrapperpp::IFunc {
    void wrap_pre () override {
        wrapperpp::Arg buffer = get_arg (*this, "Buffer");
        wrapperpp::Arg sz = get_arg (*this, "Size");
        char *buffer_copy = (char *) allocate (sz);
        std::copy ((char *)(void *)buffer, (char *)(void *)buffer + sz, buffer_copy);
        push (buffer_copy);
        push (sz);
    }

    void wrap_post () override {
        wrapperpp::Arg sz = pop();
        wrapperpp::Arg buffer_copy = pop();
        wrapperpp::Arg retval = get_retval ();

        printf ("Call to %s for passing %llu bytes \"", name, (size_t)sz);
        fwrite (buffer_copy, 1, sz, stdout);
        printf ("\" returned %lld\n", (ssize_t)retval);

        cleance(buffer_copy, sz);
    }
};


