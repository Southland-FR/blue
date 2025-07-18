/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.x
 *               (Shared logic for modifications)
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        mods/shared_logic/luadefs/CLuaPedDefs.h
 *  PURPOSE:     Lua ped definitions class header
 *
 *****************************************************************************/

#pragma once
#include "CLuaDefs.h"
#include "CVector.h"
#include <lua/CLuaMultiReturn.h>

class CLuaPedDefs : public CLuaDefs
{
public:
    static void LoadFunctions();
    static void AddClass(lua_State* luaVM);

    LUA_DECLARE(CreatePed);
    LUA_DECLARE(KillPed);
    LUA_DECLARE(DetonateSatchels);

    static bool ResetPedVoice(CClientPed* ped);
    LUA_DECLARE(GetPedVoice);
    LUA_DECLARE(SetPedVoice);
    LUA_DECLARE(GetPedTarget);
    LUA_DECLARE_OOP(GetPedTargetStart);
    LUA_DECLARE_OOP(GetPedTargetEnd);
    LUA_DECLARE_OOP(GetPedTargetCollision);
    LUA_DECLARE(GetPedWeaponSlot);
    LUA_DECLARE(GetPedWeapon);
    LUA_DECLARE(GetPedAmmoInClip);
    LUA_DECLARE(GetPedTotalAmmo);
    LUA_DECLARE(GetPedWeaponMuzzlePosition);
    LUA_DECLARE(GetPedStat);
    LUA_DECLARE(GetPedOccupiedVehicle);
    LUA_DECLARE(GetPedOccupiedVehicleSeat);
    static float GetPedArmor(CClientPed* const ped) noexcept;
    LUA_DECLARE(IsPedChoking);
    LUA_DECLARE(IsPedDucked);
    LUA_DECLARE(IsPedInVehicle);
    LUA_DECLARE(DoesPedHaveJetPack);
    LUA_DECLARE(IsPedOnGround);
    LUA_DECLARE(GetPedTask);
    LUA_DECLARE(GetPedSimplestTask);
    LUA_DECLARE(IsPedDoingTask);
    LUA_DECLARE(GetPedContactElement);
    LUA_DECLARE(GetPedRotation);
    LUA_DECLARE(CanPedBeKnockedOffBike);

    static std::variant<bool, CLuaMultiReturn<float, float, float>> GetElementBonePosition(CClientPed* ped, const std::uint16_t bone);
    static std::variant<bool, CLuaMultiReturn<float, float, float>> GetElementBoneRotation(CClientPed* ped, const std::uint16_t bone);
    static std::variant<bool, CLuaMultiReturn<float, float, float>> GetElementBoneScale(CClientPed* ped, std::uint16_t bone);
    static std::variant<bool, CLuaMultiReturn<float, float, float, float>> GetElementBoneQuaternion(CClientPed* ped, const std::uint16_t bone);
    static std::variant<bool, std::array<std::array<float, 4>, 4>>         GetElementBoneMatrix(CClientPed* ped, const std::uint16_t bone);

    static bool SetElementBonePosition(CClientPed* ped, const std::uint16_t bone, const CVector position);
    static bool SetElementBoneRotation(CClientPed* ped, const std::uint16_t bone, const float yaw, const float pitch, const float roll);
    static bool SetElementBoneScale(CClientPed* ped, std::uint16_t bone, float scaleX, float scaleY, float scaleZ);
    static bool SetElementBoneQuaternion(CClientPed* ped, const std::uint16_t bone, const float x, const float y, const float z, const float w);
    static bool SetElementBoneMatrix(CClientPed* ped, const std::uint16_t bone, const CMatrix matrix);

    static bool UpdateElementRpHAnim(CClientPed* ped);

    LUA_DECLARE_OOP(GetPedBonePosition);
    static int OOP_GetPedBoneScale(lua_State* luaVM);
    LUA_DECLARE(GetPedClothes);
    static bool GetPedControlState(std::variant<CClientPed*, std::string> first, std::optional<std::string> maybeControl);
    LUA_DECLARE(GetPedAnalogControlState);
    LUA_DECLARE(IsPedSunbathing);
    LUA_DECLARE(IsPedDoingGangDriveby);
    LUA_DECLARE(GetPedFightingStyle);
    LUA_DECLARE(GetPedAnimation);
    static float GetPedAnimationProgress(CClientPed* ped);
    static float GetPedAnimationSpeed(CClientPed* ped);
    static float GetPedAnimationLength(CClientPed* ped);
    LUA_DECLARE(GetPedMoveState);
    LUA_DECLARE(GetPedMoveAnim);
    LUA_DECLARE(IsPedHeadless);
    LUA_DECLARE(IsPedFrozen);
    LUA_DECLARE(IsPedFootBloodEnabled);
    LUA_DECLARE(GetPedCameraRotation);
    LUA_DECLARE(IsPedOnFire);
    LUA_DECLARE(GetPedOxygenLevel);
    LUA_DECLARE(IsPedDead);

    LUA_DECLARE(SetPedRotation);
    LUA_DECLARE(SetPedCanBeKnockedOffBike);
    LUA_DECLARE(SetPedAnimation);
    LUA_DECLARE(SetPedAnimationProgress);
    LUA_DECLARE(SetPedAnimationSpeed);
    LUA_DECLARE(SetPedMoveAnim);
    static bool SetPedArmor(CClientPed* const ped, const float armor);
    LUA_DECLARE(SetPedWeaponSlot);
    LUA_DECLARE(GivePedWeapon);
    static bool IsPedReloadingWeapon(CClientPed* const ped) noexcept;
    LUA_DECLARE(AddPedClothes);
    LUA_DECLARE(RemovePedClothes);
    static bool SetPedControlState(std::variant<CClientPed*, std::string> first, std::variant<std::string, bool> second,
                                   std::optional<bool> maybeState);
    LUA_DECLARE(SetPedAnalogControlState);
    LUA_DECLARE(SetPedDoingGangDriveby);
    static bool SetPedFightingStyle(CClientEntity* const entity, const unsigned int style);
    LUA_DECLARE(SetPedLookAt);
    LUA_DECLARE(SetPedHeadless);
    LUA_DECLARE(SetPedFrozen);
    LUA_DECLARE(SetPedFootBloodEnabled);
    LUA_DECLARE(SetPedCameraRotation);
    LUA_DECLARE(SetPedAimTarget);
    LUA_DECLARE(SetPedOnFire);
    LUA_DECLARE_OOP(WarpPedIntoVehicle);
    LUA_DECLARE(RemovePedFromVehicle);
    LUA_DECLARE(SetPedOxygenLevel);
    LUA_DECLARE(SetPedStat);
    static bool SetPedEnterVehicle(CClientPed* pPed, std::optional<CClientVehicle*> pOptVehicle, std::optional<bool> bOptPassenger);
    static bool SetPedExitVehicle(CClientPed* pPed);
    static bool IsPedBleeding(CClientPed* ped);
    static bool SetPedBleeding(CClientPed* ped, bool bleeding);
    
    static bool killPedTask(CClientPed* ped, taskType taskType, std::uint8_t taskNumber, std::optional<bool> gracefully);

    static void PlayPedVoiceLine(CClientPed* ped, int speechId, std::optional<float> probability);
};
