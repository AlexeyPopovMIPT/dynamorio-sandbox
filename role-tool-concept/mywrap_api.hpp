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

/* User inherits a class for each role from Func (using directive of this class)
   and defines wrap_pre () and wrap_post () for it. wrap_pre () is called before
   the target function is executed and wrap_post () is called after, with 
   (almost) same *this (ToolService part is changed) */
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
