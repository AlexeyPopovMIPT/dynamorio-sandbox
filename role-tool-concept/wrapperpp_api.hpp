#pragma once
#ifndef WRAPPERPP_API_HPP
#define WRAPPERPP_API_HPP

namespace wrapperpp {

class Arg {
    void *ptr;
public:
    Arg (void *o): ptr(o) {}
    Arg (ssize_t o): ptr (reinterpret_cast<void *>(o)) {}
    operator void *() {
        return ptr;
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


} // namespace wrapperpp

#endif // !WRAPPERPP_API_HPP
