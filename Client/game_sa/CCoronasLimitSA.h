/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        game_sa/CCoronasLimitSA.h
 *  PURPOSE:     Corona limit adjustment
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

class CRegisteredCoronaSAInterface;

class CCoronasLimitSA
{
public:
    static bool SetLimit(int iCoronaLimit);
    static int  GetLimit() { return ms_iCoronasLimit; }

    static CRegisteredCoronaSAInterface* GetArray() { return ms_pCoronasArray; }

private:
    static int                           ms_iCoronasLimit;
    static CRegisteredCoronaSAInterface* ms_pCoronasArray;
    static bool                          ms_bAllocated;
};
