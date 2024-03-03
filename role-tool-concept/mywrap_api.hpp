#ifndef MYWRAP_IMPL
#error This is an internal header, don't use it directly.
#endif

#ifndef MYWRAP_API_HPP
#define MYWRAP_API_HPP

#include <cstddef> // ssize_t

namespace mywrap {

class WordSizeCell {
    void *ptr;
public:
    WordSizeCell (void *o): ptr(o) {}
    WordSizeCell (ssize_t o): ptr (reinterpret_cast<void *>(o)) {}
    WordSizeCell (int o): WordSizeCell ((ssize_t)o) {}
    template <class T>
    operator T *() {
        return static_cast <T*> ptr;
    }
    operator ssize_t () {
        return reinterpret_cast<ssize_t>(ptr);
    }
};


template <class ToolService>
class ProtoFunc : public ToolService {
public:
    const char *name;

    ProtoFunc (const char *func_name, const ToolService &srv):
        ToolService (srv),
        name (func_name)
    {}

    void wrap_pre () {}
    void wrap_post () {}

protected: 
    ~ProtoFunc () {}
};


int setup (const char *role_config_file) {
    load_config (role_config_file);
}


} // namespace mywrap

#endif // !MYWRAP_API_HPP
