/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <gui/Main.h>

namespace LuaCore::Savestate
{
    static int SaveFileSavestate(lua_State* L)
    {
        const std::string path = lua_tostring(L, 1);

        core_st_wait_increment();
        AsyncExecutor::invoke_async([=]
        {
            core_st_wait_decrement();
            core_st_do_file(path, core_st_job_save, nullptr, false);
        });
        
        return 0;
    }

    static int LoadFileSavestate(lua_State* L)
    {
        const std::string path = lua_tostring(L, 1);

        core_st_wait_increment();
        AsyncExecutor::invoke_async([=]
        {
            core_st_wait_decrement();
            core_st_do_file(path, core_st_job_load, nullptr, false);
        });
        
        return 0;
    }
}
