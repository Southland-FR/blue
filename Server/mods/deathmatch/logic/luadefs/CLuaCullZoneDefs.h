#pragma once
#include "CLuaDefs.h"

class CCullZone;

class CLuaCullZoneDefs : public CLuaDefs
{
public:
    static void LoadFunctions();
    static void AddClass(lua_State* luaVM);

    LUA_DECLARE(CreateCullZone);
    LUA_DECLARE(DestroyCullZone);
};
