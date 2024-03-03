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

#include "dr_api.h"
#include "drwrap.h"
#include "drmgr.h"
#include <utility>

#include "sample_instrumentation_algo.cpp"


static void
wrap_pre_Send_sendto(void *wrapcxt, DR_PARAM_OUT void **user_data);
static void
wrap_post_Send_sendto(void *wrapcxt, void *user_data);


static void
module_load_event(void *drcontext, const module_data_t *mod, bool loaded)
{
    app_pc towrap = (app_pc)dr_get_proc_address(mod->handle, "sendto");
    if (towrap != NULL) {
        bool ok = drwrap_wrap(towrap, wrap_pre_Send_sendto, 
                                wrap_post_Send_sendto);
        if (!ok) {
            dr_fprintf(STDERR, "Couldn't wrap sendto\n");
            DR_ASSERT(ok);
        }
    }
}

static void
event_exit (void)
{
    drwrap_exit ();
    drmgr_exit ();
}

DR_EXPORT void
dr_init (client_id_t id)
{
    drmgr_init ();
    drwrap_init ();
    dr_register_exit_event (event_exit);
    drmgr_register_module_load_event (module_load_event);
}

static void
wrap_pre_Send_sendto (void *wrapcxt, DR_PARAM_OUT void **user_data)
{
    Send *func = (Send *) dr_global_alloc (sizeof (Send));
    new (func) Send ("sendto", mywrap::drService(wrapcxt));
    func->wrap_pre ();
    *user_data = func;
}


static void
wrap_post_Send_sendto (void *wrapcxt, void *user_data)
{
    Send *func = (Send *) user_data;
    func->wrapcxt = wrapcxt;
    func->wrap_post ();
    func->~Send();
    dr_global_free (func, sizeof (Send));
}
