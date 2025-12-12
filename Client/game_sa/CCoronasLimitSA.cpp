/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        game_sa/CCoronasLimitSA.cpp
 *  PURPOSE:     Corona limit adjustment
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"
#include "CCoronasLimitSA.h"
#include "CRegisteredCoronaSA.h"

int                           CCoronasLimitSA::ms_iCoronasLimit = 64;
CRegisteredCoronaSAInterface* CCoronasLimitSA::ms_pCoronasArray = nullptr;
bool                          CCoronasLimitSA::ms_bAllocated = false;

#define ARRAY_CORONAS_DEFAULT 0xC3E058

bool CCoronasLimitSA::SetLimit(int iCoronaLimit)
{
    if (iCoronaLimit < 64 || iCoronaLimit > 10000)
        return false;

    if (ms_bAllocated)
        return false;

    if (iCoronaLimit > 64)
    {
        ms_pCoronasArray = new CRegisteredCoronaSAInterface[iCoronaLimit];
        if (!ms_pCoronasArray)
            return false;

        memset(ms_pCoronasArray, 0, sizeof(CRegisteredCoronaSAInterface) * iCoronaLimit);
        ms_bAllocated = true;
    }
    else
    {
        ms_pCoronasArray = reinterpret_cast<CRegisteredCoronaSAInterface*>(ARRAY_CORONAS_DEFAULT);
    }

    ms_iCoronasLimit = iCoronaLimit;

    char* pArray = reinterpret_cast<char*>(ms_pCoronasArray);
    char* pArrayEnd = pArray + (iCoronaLimit * sizeof(CRegisteredCoronaSAInterface));

    // Patch array pointers
    MemPut<void*>(0x6FAACE + 1, pArray + 0x0C);
    MemPut<void*>(0x6FAE9F + 1, pArray);
    MemPut<void*>(0x6FAEB5 + 2, pArrayEnd);
    MemPut<void*>(0x6FAF41 + 1, pArray + 0x38);
    MemPut<void*>(0x6FB647 + 1, pArray + 0x36);
    MemPut<void*>(0x6FB656 + 1, pArrayEnd + 0x36);
    MemPut<void*>(0x6FB6CE + 1, pArray + 0x30);
    MemPut<void*>(0x6FB9B6 + 2, pArrayEnd + 0x30);
    MemPut<void*>(0x6FC2E6 + 2, pArray + 0x0C);
    MemPut<void*>(0x6FC316 + 2, pArray + 0x0C);
    MemPut<void*>(0x6FC33F + 2, pArray + 0x30);
    MemPut<void*>(0x6FC348 + 2, pArray + 0x30);
    MemPut<void*>(0x6FC34F + 2, pArray + 0x34);
    MemPut<void*>(0x6FC356 + 2, pArray + 0x36);
    MemPut<void*>(0x6FC363 + 2, pArray + 0x35);
    MemPut<void*>(0x6FC369 + 2, pArray + 0x0C);
    MemPut<void*>(0x6FC378 + 2, pArray + 0x2C);
    MemPut<void*>(0x6FC382 + 2, pArray + 0x2D);
    MemPut<void*>(0x6FC38C + 2, pArray + 0x2E);
    MemPut<void*>(0x6FC396 + 2, pArray + 0x2F);
    MemPut<void*>(0x6FC39F + 2, pArray + 0x00);
    MemPut<void*>(0x6FC3B7 + 2, pArray + 0x14);
    MemPut<void*>(0x6FC3C1 + 2, pArray + 0x18);
    MemPut<void*>(0x6FC3CB + 2, pArray + 0x1C);
    MemPut<void*>(0x6FC3D5 + 2, pArray + 0x10);
    MemPut<void*>(0x6FC3DF + 2, pArray + 0x32);
    MemPut<void*>(0x6FC3E9 + 2, pArray + 0x33);
    MemPut<void*>(0x6FC3EF + 2, pArray + 0x34);
    MemPut<void*>(0x6FC3F9 + 2, pArray + 0x31);
    MemPut<void*>(0x6FC401 + 2, pArray + 0x34);
    MemPut<void*>(0x6FC40B + 2, pArray + 0x20);
    MemPut<void*>(0x6FC415 + 2, pArray + 0x28);
    MemPut<void*>(0x6FC430 + 2, pArray + 0x36);
    MemPut<void*>(0x6FC448 + 2, pArray + 0x36);
    MemPut<void*>(0x6FC452 + 2, pArray + 0x36);
    MemPut<void*>(0x6FC458 + 2, pArray + 0x38);
    MemPut<void*>(0x6FC476 + 2, pArray + 0x30);
    MemPut<void*>(0x6FC494 + 2, pArray + 0x0C);
    MemPut<void*>(0x6FC4AA + 2, pArray + 0x36);
    MemPut<void*>(0x6FC4B0 + 2, pArray + 0x38);
    MemPut<void*>(0x6FC536 + 2, pArray + 0x0C);
    MemPut<void*>(0x6FC553 + 2, pArray + 0x00);
    MemPut<void*>(0x6FC56B + 2, pArray + 0x18);

    // Patch loop limits
    MemPut<DWORD>(0x6FAF46 + 4, iCoronaLimit);
    MemPut<DWORD>(0x6FAAD3 + 1, iCoronaLimit);
    MemPut<WORD>(0x6FC2EF + 3, static_cast<WORD>(iCoronaLimit));
    MemPut<WORD>(0x6FC2F5 + 3, static_cast<WORD>(iCoronaLimit));
    MemPut<WORD>(0x6FC321 + 3, static_cast<WORD>(iCoronaLimit));
    MemPut<WORD>(0x6FC327 + 3, static_cast<WORD>(iCoronaLimit));
    MemPut<WORD>(0x6FC53F + 2, static_cast<WORD>(iCoronaLimit));
    MemPut<WORD>(0x6FC545 + 2, static_cast<WORD>(iCoronaLimit));

    return true;
}
