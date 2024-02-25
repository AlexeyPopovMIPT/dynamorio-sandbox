// Example of output file of the "declarer" tool. Contains all necessary 
// declarations that will be #included in sample_definer_output.c

#include "../../include/dr_api.h" // TODO how to know what to include?..

typedef int (*pre_cb) (void *, DR_PARAM_OUT void **);
typedef int (*post_cb) (void *, void **);

typedef struct wrap_func {
    const char *const name;
    pre_cb wrap_pre;
    post_cb wrap_post;
} wrap_func_t;

pre_cb wrap_pre_Send;
post_cb wrap_post_Send;

const int Send_arg_Descriptor = 0;
const int Send_arg_Buffer = 1;
const int Send_arg_Size = 2;
const int Send_arg_Flags = 3;
const int Send_arg_Ptr_to_Sockaddr = 4;
const int Send_arg_Socklen = 5;

pre_cb wrap_pre_Recv;
post_cb wrap_post_Recv;

const int Recv_arg_Descriptor = 0;
const int Recv_arg_Buffer = 1;
const int Recv_arg_Size = 2;
const int Recv_arg_Flags = 3;
const int Recv_arg_Ptr_to_Sockaddr = 4;
const int Recv_arg_Ptr_to_Socklen = 5;

const wrap_func_t roles [] = {
    {.name = "sendto", .wrap_pre = wrap_pre_Send, .wrap_post = wrap_post_Send},
    {.name = "recvfrom", .wrap_pre = wrap_pre_Recv, 
     .wrap_post = wrap_post_Recv}
};
