/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        game_sa/CCoronasLimitSA.cpp
 *  PURPOSE:     Corona limit adjustment (ported from fastman92 limit adjuster)
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *  Corona limit code based on fastman92 limit adjuster
 *  Copyright (C) fastman92 <fastman92@gmail.com>
 *  Licensed under the MIT License
 *
 *****************************************************************************/

#include "StdInc.h"
#include "CCoronasLimitSA.h"
#include "CRegisteredCoronaSA.h"

// Static member initialization
int                          CCoronasLimitSA::ms_iCoronasLimit = 64;  // Default GTA SA limit
CRegisteredCoronaSAInterface* CCoronasLimitSA::ms_pCoronasArray = nullptr;
bool                         CCoronasLimitSA::ms_bAllocated = false;

// Default GTA SA corona array address
#define ARRAY_CORONAS_DEFAULT 0xC3E058

bool CCoronasLimitSA::SetLimit(int iCoronaLimit)
{
    // Validate limit
    if (iCoronaLimit < 64 || iCoronaLimit > 10000)
        return false;

    // Only allow setting once
    if (ms_bAllocated)
        return false;

    // Allocate new array if limit is greater than default
    if (iCoronaLimit > 64)
    {
        ms_pCoronasArray = new CRegisteredCoronaSAInterface[iCoronaLimit];
        if (!ms_pCoronasArray)
            return false;

        // Zero initialize
        memset(ms_pCoronasArray, 0, sizeof(CRegisteredCoronaSAInterface) * iCoronaLimit);
        ms_bAllocated = true;
    }
    else
    {
        // Use default array
        ms_pCoronasArray = reinterpret_cast<CRegisteredCoronaSAInterface*>(ARRAY_CORONAS_DEFAULT);
    }

    ms_iCoronasLimit = iCoronaLimit;

    // Calculate structure offsets for patching
    // CRegisteredCoronaSAInterface is 0x3C (60) bytes
    // Offsets within the structure:
    //   m_vPosn (CVector):    0x00
    //   Identifier (DWORD):   0x0C
    //   pTex (pointer):       0x10
    //   Size (float):         0x14
    //   NormalAngle (float):  0x18
    //   Range (float):        0x1C
    //   PullTowardsCam:       0x20
    //   HeightAboveGround:    0x24
    //   FadeSpeed:            0x28
    //   Color (RGBA):         0x2C (Red=0x2C, Green=0x2D, Blue=0x2E, Alpha=0x2F)
    //   FadedIntensity:       0x30
    //   RegisteredThisFrame:  0x31
    //   FlareType:            0x32
    //   ReflectionType:       0x33
    //   Flags byte 1:         0x34
    //   JustCreated:          0x35
    //   Flags byte 2:         0x36
    //   pEntityAttachedTo:    0x38

    char* pArray = reinterpret_cast<char*>(ms_pCoronasArray);
    char* pArrayEnd = pArray + (iCoronaLimit * sizeof(CRegisteredCoronaSAInterface));

    // =====================================================
    // Patch array pointers (from fastman92 limit adjuster)
    // These redirect hardcoded array references to our new array
    // =====================================================

    // CCoronas::Update - mov eax, offset aCoronas.m_dwId
    MemPut<void*>(0x6FAACE + 1, pArray + 0x0C);

    // CCoronas::DoLOSCheck - mov esi, offset aCoronas
    MemPut<void*>(0x6FAE9F + 1, pArray);

    // CCoronas::DoLOSCheck - cmp esi, offset aCoronas[LIMIT]
    MemPut<void*>(0x6FAEB5 + 2, pArrayEnd);

    // CCoronas::LOScheck - mov edi, offset aCoronas.m_pAttachedTo
    MemPut<void*>(0x6FAF41 + 1, pArray + 0x38);

    // CCoronas::RenderReflections - mov eax, offset aCoronas.m_flags+2
    MemPut<void*>(0x6FB647 + 1, pArray + 0x36);

    // CCoronas::RenderReflections - cmp eax, offset aCoronas[LIMIT].m_flags+2
    MemPut<void*>(0x6FB656 + 1, pArrayEnd + 0x36);

    // CCoronas::RenderReflections - mov esi, offset aCoronas.m_nFadeState
    MemPut<void*>(0x6FB6CE + 1, pArray + 0x30);

    // CCoronas::RenderReflections - cmp esi, offset aCoronas[LIMIT].m_nFadeState
    MemPut<void*>(0x6FB9B6 + 2, pArrayEnd + 0x30);

    // CCoronas::RegisterCorona - cmp ds:aCoronas.m_dwId[edx], esi
    MemPut<void*>(0x6FC2E6 + 2, pArray + 0x0C);

    // CCoronas::RegisterCorona - mov edi, ds:aCoronas.m_dwId[eax]
    MemPut<void*>(0x6FC316 + 2, pArray + 0x0C);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_nFadeState[ecx], 0
    MemPut<void*>(0x6FC33F + 2, pArray + 0x30);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_nFadeState[ecx], 0FFh
    MemPut<void*>(0x6FC348 + 2, pArray + 0x30);

    // CCoronas::RegisterCorona - or ds:aCoronas.m_flags[ecx], 2
    MemPut<void*>(0x6FC34F + 2, pArray + 0x34);

    // CCoronas::RegisterCorona - and ds:aCoronas.m_flags+2[ecx], 0FBh
    MemPut<void*>(0x6FC356 + 2, pArray + 0x36);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_bJustCreated[ecx], dl
    MemPut<void*>(0x6FC363 + 2, pArray + 0x35);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_dwId[ecx], esi
    MemPut<void*>(0x6FC369 + 2, pArray + 0x0C);

    // CCoronas::RegisterCorona - mov byte ptr ds:aCoronas.m_Color.red[ecx], al
    MemPut<void*>(0x6FC378 + 2, pArray + 0x2C);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_Color.green[ecx], al
    MemPut<void*>(0x6FC382 + 2, pArray + 0x2D);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_Color.blue[ecx], al
    MemPut<void*>(0x6FC38C + 2, pArray + 0x2E);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_Color.alpha[ecx], al
    MemPut<void*>(0x6FC396 + 2, pArray + 0x2F);

    // CCoronas::RegisterCorona - lea eax, aCoronas.m_vPosn[ecx]
    MemPut<void*>(0x6FC39F + 2, pArray + 0x00);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_fSize[ecx], eax
    MemPut<void*>(0x6FC3B7 + 2, pArray + 0x14);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_fAngle[ecx], eax
    MemPut<void*>(0x6FC3C1 + 2, pArray + 0x18);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_fFarClip[ecx], eax
    MemPut<void*>(0x6FC3CB + 2, pArray + 0x1C);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_pTexture[ecx], eax
    MemPut<void*>(0x6FC3D5 + 2, pArray + 0x10);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_nFlareType[ecx], al
    MemPut<void*>(0x6FC3DF + 2, pArray + 0x32);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_bUsesReflection[ecx], al
    MemPut<void*>(0x6FC3E9 + 2, pArray + 0x33);

    // CCoronas::RegisterCorona - mov al, ds:aCoronas.m_flags[ecx]
    MemPut<void*>(0x6FC3EF + 2, pArray + 0x34);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_bRegisteredThisFrame[ecx], dl
    MemPut<void*>(0x6FC3F9 + 2, pArray + 0x31);

    // CCoronas::RegisterCorona - xor ds:aCoronas.m_flags[ecx], al
    MemPut<void*>(0x6FC401 + 2, pArray + 0x34);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_fNearClip[ecx], eax
    MemPut<void*>(0x6FC40B + 2, pArray + 0x20);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_fFadeSpeed[ecx], eax
    MemPut<void*>(0x6FC415 + 2, pArray + 0x28);

    // CCoronas::RegisterCorona - mov dl, ds:aCoronas.m_flags+2[ecx]
    MemPut<void*>(0x6FC430 + 2, pArray + 0x36);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_flags+2[ecx], al
    MemPut<void*>(0x6FC448 + 2, pArray + 0x36);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_flags+2[ecx], al
    MemPut<void*>(0x6FC452 + 2, pArray + 0x36);

    // CCoronas::RegisterCorona - lea eax, aCoronas.m_pAttachedTo[ecx]
    MemPut<void*>(0x6FC458 + 2, pArray + 0x38);

    // CCoronas::RegisterCorona - cmp ds:aCoronas.m_nFadeState[ecx], 0
    MemPut<void*>(0x6FC476 + 2, pArray + 0x30);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_dwId[ecx], 0
    MemPut<void*>(0x6FC494 + 2, pArray + 0x0C);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_flags+2[ecx], al
    MemPut<void*>(0x6FC4AA + 2, pArray + 0x36);

    // CCoronas::RegisterCorona - mov ds:aCoronas.m_pAttachedTo[ecx], 0
    MemPut<void*>(0x6FC4B0 + 2, pArray + 0x38);

    // CCoronas::UpdateCoronaCoors - cmp ds:aCoronas.m_dwId[esi], edx
    MemPut<void*>(0x6FC536 + 2, pArray + 0x0C);

    // CCoronas::UpdateCoronaCoors - lea edx, aCoronas.m_vPosn[eax]
    MemPut<void*>(0x6FC553 + 2, pArray + 0x00);

    // CCoronas::UpdateCoronaCoors - mov ds:aCoronas.m_fAngle[eax], edx
    MemPut<void*>(0x6FC56B + 2, pArray + 0x18);

    // =====================================================
    // Patch loop limit values
    // These change the hardcoded "64" (0x40) comparisons
    // =====================================================

    // CCoronas::Render - limit value at 0x6FAF46+4
    MemPut<DWORD>(0x6FAF46 + 4, iCoronaLimit);

    // CCoronas::Update - mov ecx, 40h -> mov ecx, newLimit
    // Original: B9 40 00 00 00 (mov ecx, 0x40)
    MemPut<DWORD>(0x6FAAD3 + 1, iCoronaLimit);

    // CCoronas::RegisterCorona - cmp cx, 40h -> cmp cx, newLimit
    // Multiple locations need 16-bit comparison patches
    // Original bytes: 66 81 F9 40 00 (cmp cx, 0x40)
    MemPut<WORD>(0x6FC2EF + 3, static_cast<WORD>(iCoronaLimit));
    MemPut<WORD>(0x6FC2F5 + 3, static_cast<WORD>(iCoronaLimit));
    MemPut<WORD>(0x6FC321 + 3, static_cast<WORD>(iCoronaLimit));
    MemPut<WORD>(0x6FC327 + 3, static_cast<WORD>(iCoronaLimit));

    // CCoronas::UpdateCoronaCoors - cmp ax, 40h -> cmp ax, newLimit
    // Original bytes: 66 3D 40 00 (cmp ax, 0x40)
    MemPut<WORD>(0x6FC53F + 2, static_cast<WORD>(iCoronaLimit));
    MemPut<WORD>(0x6FC545 + 2, static_cast<WORD>(iCoronaLimit));

    return true;
}
