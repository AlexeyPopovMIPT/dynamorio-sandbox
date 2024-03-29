#include <cstdio>
#include <algorithm> // std::copy
#include "mywrap_dr.hpp"

struct Send: public mywrap::Func {
    using mywrap::Func::ProtoFunc;

    char *buffer_copy = nullptr;
    size_t sz = 0;

    void wrap_pre () {
        char *buffer = get_arg ("Buffer").value_or (nullptr);
        sz = get_arg ("Size").value_or (0);
        buffer_copy = (char *) allocate (sz);
        std::copy (buffer, buffer + sz, buffer_copy);
    }

    void wrap_post () {
        ssize_t retval = get_retval ().value_or (-1);

        printf ("Call to %s for passing %llu bytes \"", name, sz);
        fwrite (buffer_copy, 1, sz, stdout);
        printf ("\" returned %lld\n", retval);

        cleance (buffer_copy, sz);
    }
};


