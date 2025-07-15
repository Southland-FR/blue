#include "StdInc.h"
#include "CLuaCullZoneDefs.h"
#include "CStaticFunctionDefinitions.h"
#include "CScriptArgReader.h"

void CLuaCullZoneDefs::LoadFunctions()
{
    constexpr static const std::pair<const char*, lua_CFunction> functions[]{
        {"createCullZone", CreateCullZone},
        {"destroyCullZone", DestroyCullZone},
    };

    for (const auto& [name, func] : functions)
        CLuaCFunctions::AddFunction(name, func);
}

void CLuaCullZoneDefs::AddClass(lua_State* luaVM)
{
    lua_newclass(luaVM);

    lua_classfunction(luaVM, "create", "createCullZone");
    lua_classfunction(luaVM, "destroy", "destroyCullZone");

    lua_registerclass(luaVM, "CullZone", "Element");
}

int CLuaCullZoneDefs::CreateCullZone(lua_State* luaVM)
{
    CVector       pos;
    float         size;
    float         height;
    unsigned char flags = 0;

    CScriptArgReader argStream(luaVM);
    argStream.ReadVector3D(pos);
    argStream.ReadNumber(size);
    argStream.ReadNumber(height);
    argStream.ReadNumber(flags, flags);

    if (!argStream.HasErrors())
    {
        CLuaMain* pLuaMain = g_pGame->GetLuaManager()->GetVirtualMachine(luaVM);
        if (pLuaMain)
        {
            CResource* pResource = pLuaMain->GetResource();
            if (pResource)
            {
                CCullZone* pZone = CStaticFunctionDefinitions::CreateCullZone(pResource, pos, size, height, flags);
                if (pZone)
                {
                    CElementGroup* pGroup = pResource->GetElementGroup();
                    if (pGroup)
                        pGroup->Add(pZone);
                    lua_pushelement(luaVM, pZone);
                    return 1;
                }
            }
        }
    }
    else
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());

    lua_pushboolean(luaVM, false);
    return 1;
}

int CLuaCullZoneDefs::DestroyCullZone(lua_State* luaVM)
{
    CCullZone* pZone;

    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(pZone);

    if (!argStream.HasErrors())
    {
        if (CStaticFunctionDefinitions::DestroyCullZone(pZone))
        {
            lua_pushboolean(luaVM, true);
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());

    lua_pushboolean(luaVM, false);
    return 1;
}
