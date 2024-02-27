/* **********************************************************
 * Copyright (c) 2015-2018 Google, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Google, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL GOOGLE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Rework of DynamoRIO samples/ssljack.c with the only logging file and 
 * different output format. Original commentary on ssljack.c is given below.
 */

/*
 * ssljack hijacks (wraps) interesting OpenSSL and GnuTLS functions using the
 * drwrap extension. ssljack creates separate read and write files per SSL
 * context containing all the data the app read and wrote.
 *
 * Hacked together by Dhiru Kholia (dhiru [at] openwall [dot] com)
 *
 * "ssl-jack" was initially published on the https://github.com/kholia/dedrop
 * page by me. This revision integrates changes from Dragos-George Comaneci, to
 * improve logging.
 */

#include "dr_api.h"
#include "drwrap.h"
#include "drmgr.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include "function_call_dump.h"


static void
wrap_pre_send(void *wrapcxt, DR_PARAM_OUT void **user_data);
static void
wrap_pre_sendto(void *wrapcxt, DR_PARAM_OUT void **user_data);
static void
wrap_pre_sendmsg(void *wrapcxt, DR_PARAM_OUT void **user_data);
static void
wrap_post_recv(void *wrapcxt, void *user_data);
static void
wrap_post_recvfrom(void *wrapcxt, void *user_data);
static void
wrap_post_recvmsg(void *wrapcxt, void *user_data);
static void
wrap_pre_recvmsg (void *wrapcxt, DR_PARAM_OUT void **user_data);
static void
wrap_pre_recv (void *wrapcxt, DR_PARAM_OUT void **user_data);
static void
wrap_pre_recvfrom (void *wrapcxt, DR_PARAM_OUT void **user_data);
static void
wrap_pre_print_pair (void *wrapcxt, DR_PARAM_OUT void **user_data);

const char* OUT_FILE = "output.log";

static void
module_load_event(void *drcontext, const module_data_t *mod, bool loaded)
{
    dr_fprintf (STDERR, "Load module %s\n", mod->full_path);
    app_pc towrap = (app_pc)dr_get_proc_address(mod->handle, "send");
    if (towrap != NULL) {
        bool ok = drwrap_wrap(towrap, wrap_pre_send, NULL);
        if (!ok) {
            dr_fprintf(STDERR, "Couldn't wrap send\n");
            DR_ASSERT(ok);
        }
    }

    /* Used by ping */
    towrap = (app_pc)dr_get_proc_address(mod->handle, "sendto");
    if (towrap != NULL) {
        bool ok = drwrap_wrap(towrap, wrap_pre_sendto, NULL);
        if (!ok) {
            dr_fprintf(STDERR, "Couldn't wrap sendto\n");
            DR_ASSERT(ok);
        }
    }

    towrap = (app_pc)dr_get_proc_address(mod->handle, "sendmsg");
    if (towrap != NULL) {
        bool ok = drwrap_wrap(towrap, wrap_pre_sendmsg, NULL);
        if (!ok) {
            dr_fprintf(STDERR, "Couldn't wrap sendmsg\n");
            DR_ASSERT(ok);
        }
    }

    towrap = (app_pc)dr_get_proc_address(mod->handle, "recv");
    if (towrap != NULL) {
        bool ok = drwrap_wrap(towrap, wrap_pre_recv, wrap_post_recv);
        if (!ok) {
            dr_fprintf(STDERR, "Couldn't wrap recv\n");
            DR_ASSERT(ok);
        }
    }

    /* Used by ping */
    towrap = (app_pc)dr_get_proc_address(mod->handle, "recvmsg");
    if (towrap != NULL) {
        bool ok = drwrap_wrap(towrap, wrap_pre_recvmsg, wrap_post_recvmsg);
        if (!ok) {
            dr_fprintf(STDERR, "Couldn't wrap recvmsg\n");
            DR_ASSERT(ok);
        }
    }

    towrap = (app_pc)dr_get_proc_address(mod->handle, "recvfrom");
    if (towrap != NULL) {
        bool ok = drwrap_wrap(towrap, wrap_pre_recvfrom, wrap_post_recvfrom);
        if (!ok) {
            dr_fprintf(STDERR, "Couldn't wrap recvfrom\n");
            DR_ASSERT(ok);
        }
    }

    towrap = (app_pc)dr_get_proc_address(mod->handle, "print_pair");
    if (towrap != NULL) {
        bool ok = drwrap_wrap(towrap, wrap_pre_print_pair, NULL);
        if (!ok) {
            dr_fprintf(STDERR, "Couldn't wrap print_pair\n");
            DR_ASSERT(ok);
        } else {
            dr_fprintf(STDERR, "Wrapped print_pair\n");
        }
    }
}

static void
event_exit (void)
{
    drwrap_exit();
    drmgr_exit();
}

DR_EXPORT void
dr_init (client_id_t id)
{
    dr_set_client_name ("DynamoRIO client 'basedonssljack'", "http://dynamorio.org/issues");
    /* make it easy to tell, by looking at log file, which client executed */
    dr_log (NULL, DR_LOG_ALL, 1, "Client basedonssljack initializing\n");
#ifdef SHOW_RESULTS
    if (dr_is_notify_on()) {
        dr_fprintf(STDERR, "Client basedonssljack running! See output.log files for send/recv logs!\n");
    }
#endif

    drmgr_init ();
    drwrap_init ();
    dr_register_exit_event (event_exit);
    drmgr_register_module_load_event (module_load_event);
}

static void
wrap_pre_print_pair (void *wracxt, DR_PARAM_OUT void **user_data) {
    drwrap_set_arg (wracxt, 0, (void*)0xffffffffffffffffLLU);
}

/* Used by ping */
static void
wrap_pre_send (void *wrapcxt, DR_PARAM_OUT void **user_data)
{
    /* ssize_t send(int sockfd, const void *buf, size_t len, int flags); */

    int         sockfd = (int)(ssize_t)    drwrap_get_arg (wrapcxt, 0);
    unsigned char *buf = (unsigned char *) drwrap_get_arg (wrapcxt, 1);
    size_t sz          = (size_t)          drwrap_get_arg (wrapcxt, 2);
    int          flags = (int)(ssize_t)    drwrap_get_arg (wrapcxt, 3);

    FILE *fp;
    if ((fp = fopen (OUT_FILE, "ab+")) == NULL) 
    {
        dr_fprintf (STDERR, "Couldn't open the output file %s\n", OUT_FILE);
        return;
    }

    fcd_arg args[] = {
        {.name="sockfd", .type=int32_t_ty , .value=(void *)(ssize_t)sockfd},
        {.name="buf"   , .type=RAW_BYTES  , .value=(void *)buf    ,
         .printer.size=sz},
        {.name="sz"    , .type=uint64_t_ty, .value=(void *)sz    },
        {.name="flags" , .type=int32_t_ty , .value=(void *)(ssize_t)flags },
    };

    fcd_call call = {
        .arg_count = sizeof(args) / sizeof (*args),
        .name = "send",
        .return_value = NULL,
        .args = args
    };

    fcd_call_dump (fp, &call);
    fclose (fp);
}

static void
wrap_pre_sendto (void *wrapcxt, DR_PARAM_OUT void **user_data) {
    /* ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen); */
    
    void print_sockaddr (fcd_arg const *, FILE *);
    void print_ptr_to_socklen_t (fcd_arg const *, FILE *);
    
    int         sockfd = (int)(ssize_t)      drwrap_get_arg (wrapcxt, 0);
    unsigned char *buf = (unsigned char *)   drwrap_get_arg (wrapcxt, 1);
    size_t sz          = (size_t)            drwrap_get_arg (wrapcxt, 2);
    int          flags = (int)(ssize_t)      drwrap_get_arg (wrapcxt, 3);
    const struct sockaddr *dest_addr = 
                 (const struct sockaddr *)   drwrap_get_arg (wrapcxt, 4);
    socklen_t  addrlen = (socklen_t)(ssize_t)drwrap_get_arg (wrapcxt, 5);

    FILE *fp;
    if ((fp = fopen (OUT_FILE, "ab+")) == NULL) 
    {
        dr_fprintf (STDERR, "Couldn't open the output file %s\n", OUT_FILE);
        return;
    }

    fcd_arg args [] = {
        {.name="sockfd", .type=int32_t_ty , .value=(void *)(ssize_t)sockfd},
        {.name="buf"   , .type=RAW_BYTES  , .value=(void *)buf    ,
         .printer.size=sz},
        {.name="sz"    , .type=uint64_t_ty, .value=(void *)sz    },
        {.name="flags" , .type=int32_t_ty , .value=(void *)(ssize_t)flags },
        {.name="dest_addr", .type=STRUCT, .value=(void *)dest_addr,
         .printer.print_innards=print_sockaddr},
        {.name="addrlen", .type=uint32_t_ty, .value=(void *)(ssize_t)addrlen}
    };
    fcd_call call = {
        .arg_count = sizeof (args) / sizeof (*args),
        .args = args,
        .name = "sendto",
        .return_value = NULL
    };

    fcd_call_dump (fp, &call);

    /*DUMP_CALL_6 (fp, "sendto", sockfd, "%d", buf, "%20s", sz, "%lu", 
                               flags, "0x%X", dest_addr, "%p", addrlen, "%u");
    DUMP_CALL_2 (fp, "dest_addr", dest_addr->sa_data, "\"%s\"", 
                                  dest_addr->sa_family, "%hu");*/
    fclose (fp);
}


static void
wrap_pre_sendmsg (void *wrapcxt, DR_PARAM_OUT void **user_data)
{
    /* int sendmsg (int sockfd, const struct msghdr *msg, int flags);
     */
    void msg_printer (fcd_arg const *, FILE *);

    int        sockfd = (int)(ssize_t) drwrap_get_arg (wrapcxt, 0);
    const struct msghdr *msg = 
               (const struct msghdr *) drwrap_get_arg (wrapcxt, 1);
    int         flags = (int)(ssize_t) drwrap_get_arg (wrapcxt, 2);

    FILE *fp;
    if ((fp = fopen (OUT_FILE, "ab+")) == NULL) 
    {
        dr_fprintf (STDERR, "Couldn't open the output file %s\n", OUT_FILE);
        return;
    }

    fcd_arg args[] = {
        {.name="sockfd", .type=int32_t_ty, .value=(void *)(ssize_t)sockfd},
        {.name="msg", .type=STRUCT, .value=(void *)msg,
         .printer.print_innards=msg_printer},
        {.name="flags" , .type=int32_t_ty, .value=(void *)(ssize_t)flags}
    };

    fcd_call call = {
        .arg_count = sizeof (args) / sizeof (*args),
        .args = args,
        .name = "sendmsg",
        .return_value = NULL
    };

    fcd_call_dump (fp, &call);

    fclose (fp);
}

struct RecvRead
{
    int sockfd;
    void *buf;
    size_t len;
    int flags;
    struct sockaddr_in *src_addr;
    socklen_t *addrlen;
};

struct MsgRead
{
    int sockfd;
    struct msghdr *msg;
    int flags;
};

static void
wrap_pre_recv (void *wrapcxt, DR_PARAM_OUT void **user_data)
{
    struct RecvRead *rd = dr_global_alloc(sizeof(struct RecvRead));
    rd->sockfd = (int)(ssize_t) drwrap_get_arg (wrapcxt, 0);
    rd->buf    =                drwrap_get_arg (wrapcxt, 1);
    rd->len    = (size_t)       drwrap_get_arg (wrapcxt, 2);
    rd->flags  = (int)(ssize_t) drwrap_get_arg (wrapcxt, 3);
    
    *user_data = (void *) rd;
}

static void
wrap_pre_recvfrom (void *wrapcxt, DR_PARAM_OUT void **user_data)
{
    struct RecvRead *rd = dr_global_alloc(sizeof(struct RecvRead));
    rd->sockfd   = (int)(ssize_t)          drwrap_get_arg (wrapcxt, 0);
    rd->buf      =                         drwrap_get_arg (wrapcxt, 1);
    rd->len      = (size_t)                drwrap_get_arg (wrapcxt, 2);
    rd->flags    = (int)(ssize_t)          drwrap_get_arg (wrapcxt, 3);
    rd->src_addr = (struct sockaddr_in * ) drwrap_get_arg (wrapcxt, 4);
    rd->addrlen  = (socklen_t *)           drwrap_get_arg (wrapcxt, 5);

    *user_data = (void *) rd;
}

/* Used by ping */
static void
wrap_pre_recvmsg (void *wrapcxt, DR_PARAM_OUT void **user_data)
{
    struct MsgRead *mr = dr_global_alloc(sizeof(struct MsgRead));

    mr->sockfd = (int)(ssize_t) drwrap_get_arg (wrapcxt, 0);
    mr->msg = (struct msghdr *) drwrap_get_arg (wrapcxt, 1);
    mr->flags =  (int)(ssize_t) drwrap_get_arg (wrapcxt, 2);

    *user_data = (void *)mr;
}

static void
wrap_post_recv (void *wrapcxt, void *user_data)
{
    ssize_t actual_read = (size_t)(ptr_int_t)drwrap_get_retval(wrapcxt);
    int sockfd = (int)(ssize_t) drwrap_get_arg (wrapcxt, 0);


    struct RecvRead *rd = (struct RecvRead *) user_data;


    FILE *fp;
    if ((fp = fopen (OUT_FILE, "ab+")) == NULL) 
    {
        dr_fprintf(STDERR, "Couldn't open the output file %s\n", OUT_FILE);
        return;
    }

    fcd_arg args[] = {
        {.name="sockfd", .type=int32_t_ty , .value=(void *)(ssize_t)rd->sockfd},
        {.name="buf"   , .type=RAW_BYTES  , .value=rd->buf,
         .printer.size = rd->len},
        {.name="len"   , .type=uint64_t_ty, .value=(void *)rd->len},
        {.name="flags" , .type=int32_t_ty , .value=(void *)(ssize_t)rd->flags}
    };

    fcd_arg return_value =
        {.name = "", .type = int64_t_ty, .value=(void *)actual_read};

    fcd_call call = {
        .name = "recv",
        .return_value = &return_value,
        .args = args,
        .arg_count = sizeof (args) / sizeof (*args)
    };

    fcd_call_dump (fp, &call);
    
    fclose (fp);
    dr_global_free (user_data, sizeof (struct RecvRead));
}

static void
wrap_post_recvfrom (void *wrapcxt, void *user_data)
{
    void print_sockaddr (fcd_arg const *, FILE *);
    void print_ptr_to_socklen_t (fcd_arg const *, FILE *);

    ssize_t actual_read = (size_t)(ptr_int_t)drwrap_get_retval(wrapcxt);
    int sockfd = (int)(ssize_t) drwrap_get_arg (wrapcxt, 0);


    struct RecvRead *rd = (struct RecvRead *) user_data;


    FILE *fp;
    if ((fp = fopen (OUT_FILE, "ab+")) == NULL) 
    {
        dr_fprintf(STDERR, "Couldn't open the output file %s\n", OUT_FILE);
        return;
    }

    fcd_arg args[] = {
        {.name="sockfd"  , .type=int32_t_ty  , 
         .value=(void *)(ssize_t)rd->sockfd} ,
        {.name="buf"     , .type=RAW_BYTES   , .value=rd->buf,
         .printer.size=rd->len},
        {.name="len"     , .type=uint64_t_ty , .value=(void *)rd->len},
        {.name="flags"   , .type=int32_t_ty  , 
         .value=(void *)(ssize_t)rd->flags},
        {.name="src_addr", .type=STRUCT      , .value=(void *)rd->src_addr,
         .printer.print_innards=print_sockaddr},
        {.name="addrlen" , .type=STRUCT      , .value=(void*)rd->addrlen,
         .printer.print_innards=print_ptr_to_socklen_t}
    };

    fcd_arg return_value =
        {.name = "", .type = int64_t_ty, .value=(void *)actual_read};

    fcd_call call = {
        .name = "recvfrom",
        .return_value = &return_value,
        .args = args,
        .arg_count = sizeof (args) / sizeof (*args)
    };

    fcd_call_dump (fp, &call);
    
    fclose (fp);
    dr_global_free (user_data, sizeof (struct RecvRead));
}
static void
wrap_post_recvmsg (void *wrapcxt, void *user_data)
{
    void msg_printer (fcd_arg const *, FILE *);
    ssize_t actual_read = (ssize_t)drwrap_get_retval(wrapcxt);

    struct MsgRead *rd = (struct MsgRead *)user_data;

    
    FILE *fp = fopen (OUT_FILE, "ab+");
    if (!fp)
    {
        dr_fprintf(STDERR, "Couldn't open the output file %s\n", OUT_FILE);
        return;
    }

    fcd_arg args[] = {
        {.name="sockfd", .type=int32_t_ty, .value=(void *)(ssize_t)rd->sockfd},
        {.name="msg"   , .type=STRUCT    , .value=rd->msg,
         .printer.print_innards=msg_printer},
        {.name="flags" , .type=int32_t_ty, .value=(void *)(ssize_t)rd->flags}
    };

    fcd_arg return_value = {
        .name="", 
        .type=int64_t_ty, 
        .value=(void *)actual_read
    };

    fcd_call call = {
        .arg_count = sizeof (args) / sizeof (*args),
        .args = args,
        .name = "recvmsg",
        .return_value = &return_value
    };

    fcd_call_dump (fp, &call);

    fclose (fp);
    dr_global_free (user_data, sizeof (struct MsgRead));
}


void msg_printer (fcd_arg const *arg, FILE *file) {
    struct msghdr *msg = (struct msghdr *)arg->value;
    if (msg == NULL) {
        fprintf (file, "NULL");
        return;
    }

    for (size_t i = 0, end = msg->msg_iovlen; i < end; ++i)
    {
        struct iovec *io = msg->msg_iov + i;
        if (io->iov_len > 0)
        {
            fprintf (file, "{iov_base=\"");
            fcd_fwrite (io->iov_base, 1, io->iov_len, file);
            fprintf (file, "\", iov_len=%lu}", io->iov_len);
        }

        if (__glibc_unlikely(i == end))
            fprintf (file, ", ");

    }
}

void print_sockaddr (fcd_arg const *arg, FILE *file) {
    struct sockaddr *to_print = (struct sockaddr*)(arg->value);
    if (to_print == NULL) {
        fprintf (file, "NULL");
        return;
    }

    fprintf (file, "{.sa_data=\"");
    fcd_fwrite (to_print->sa_data, 1, sizeof(to_print->sa_data), file);
    fprintf (file, "\", .sa_family=%hu}", to_print->sa_family);
}

void print_ptr_to_socklen_t (fcd_arg const *arg, FILE *file) {
    fprintf (file, "%p", arg->value);
    if (arg->value)
        fprintf (file, " (*addrlen=%u)", *(socklen_t*)arg->value);
}
