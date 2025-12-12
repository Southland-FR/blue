/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        game_sa/CCoronasLimitSA.h
 *  PURPOSE:     Corona limit adjustment (ported from fastman92 limit adjuster)
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *  Corona limit code based on fastman92 limit adjuster
 *  Copyright (C) fastman92 <fastman92@gmail.com>
 *  Licensed under the MIT License
 *
 *****************************************************************************/

#pragma once

class CRegisteredCoronaSAInterface;

class CCoronasLimitSA
{
public:
    // Apply corona limit patch - must be called before CCoronasSA is created
    static bool SetLimit(int iCoronaLimit);

    // Get current limit
    static int GetLimit() { return ms_iCoronasLimit; }

    // Get the corona array (either default GTA array or our allocated one)
    static CRegisteredCoronaSAInterface* GetArray() { return ms_pCoronasArray; }

private:
    static int                          ms_iCoronasLimit;
    static CRegisteredCoronaSAInterface* ms_pCoronasArray;
    static bool                         ms_bAllocated;
};
