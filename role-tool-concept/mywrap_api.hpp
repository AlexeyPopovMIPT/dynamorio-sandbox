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

// for every ToolContext class that has get_arg, set_arg, get_retval, set_retval methods
template <class ToolContext, class ForwardList>
class Proto_IFunc : public ToolContext, public ForwardList {
public:
    const char *name;
    virtual void wrap_pre () = 0;
    virtual void wrap_post () = 0;
protected: 
    ~Proto_IFunc () {}
};


int setup (const char *role_config_file) {
    load_config (role_config_file);
}


} // namespace mywrap

#endif // !MYWRAP_API_HPP
