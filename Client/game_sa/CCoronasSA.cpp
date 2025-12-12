/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        game_sa/CCoronasSA.cpp
 *  PURPOSE:     Corona entity manager
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"
#include "CCoronasSA.h"
#include "CRegisteredCoronaSA.h"
#include "CCoronasLimitSA.h"

CCoronasSA::CCoronasSA()
{
    m_ucCoronaReflectionsEnabled = 1;
    m_iCoronasLimit = MAX_CORONAS_DEFAULT;  // Will be updated by Initialize()
}

CCoronasSA::~CCoronasSA()
{
    for (auto* corona : Coronas)
    {
        delete corona;
    }
    Coronas.clear();
}

void CCoronasSA::Initialize()
{
    // Get the current limit and array from CCoronasLimitSA
    m_iCoronasLimit = CCoronasLimitSA::GetLimit();
    CRegisteredCoronaSAInterface* pArray = CCoronasLimitSA::GetArray();

    // If array is null, use the default GTA SA array
    if (!pArray)
        pArray = reinterpret_cast<CRegisteredCoronaSAInterface*>(ARRAY_CORONAS);

    // Clear any existing coronas
    for (auto* corona : Coronas)
    {
        delete corona;
    }
    Coronas.clear();

    // Create wrapper objects for each corona slot
    Coronas.reserve(m_iCoronasLimit);
    for (int i = 0; i < m_iCoronasLimit; i++)
    {
        Coronas.push_back(new CRegisteredCoronaSA(&pArray[i]));
    }
}

CRegisteredCorona* CCoronasSA::GetCorona(DWORD ID)
{
    if (ID >= static_cast<DWORD>(m_iCoronasLimit))
        return nullptr;
    return (CRegisteredCorona*)Coronas[ID];
}

CRegisteredCorona* CCoronasSA::CreateCorona(DWORD Identifier, CVector* position)
{
    CRegisteredCoronaSA* corona;
    corona = (CRegisteredCoronaSA*)FindCorona(Identifier);

    if (!corona)
        corona = (CRegisteredCoronaSA*)FindFreeCorona();

    if (corona)
    {
        RwTexture* texture = GetTexture(CoronaType::CORONATYPE_SHINYSTAR);
        if (texture)
        {
            corona->Init(Identifier);
            corona->SetPosition(position);
            corona->SetTexture(texture);
            return (CRegisteredCorona*)corona;
        }
    }

    return (CRegisteredCorona*)NULL;
}

CRegisteredCorona* CCoronasSA::FindFreeCorona()
{
    // Start at 2 to skip sun/moon
    for (int i = 2; i < m_iCoronasLimit; i++)
    {
        if (Coronas[i]->GetIdentifier() == 0)
        {
            return Coronas[i];
        }
    }
    return (CRegisteredCorona*)NULL;
}

CRegisteredCorona* CCoronasSA::FindCorona(DWORD Identifier)
{
    for (int i = 0; i < m_iCoronasLimit; i++)
    {
        if (Coronas[i]->GetIdentifier() == Identifier)
        {
            return Coronas[i];
        }
    }
    return (CRegisteredCorona*)NULL;
}

RwTexture* CCoronasSA::GetTexture(CoronaType type)
{
    // Validate enum is within valid range
    if (static_cast<DWORD>(type) >= MAX_CORONA_TEXTURES) [[unlikely]]
        return nullptr;
    
    // Read texture pointer from array with validation
    DWORD* pTextureArray = reinterpret_cast<DWORD*>(ARRAY_CORONA_TEXTURES);
    if (!pTextureArray) [[unlikely]]
        return nullptr;
    
    DWORD textureAddr = pTextureArray[static_cast<DWORD>(type)];
    if (!textureAddr) [[unlikely]]
        return nullptr;
    
    RwTexture* pTexture = reinterpret_cast<RwTexture*>(textureAddr);
    
    return pTexture;
}

void CCoronasSA::DisableSunAndMoon(bool bDisabled)
{
    static BYTE byteOriginal = 0;
    if (bDisabled && !byteOriginal)
    {
        byteOriginal = *(BYTE*)FUNC_DoSunAndMoon;
        MemPut<BYTE>(FUNC_DoSunAndMoon, 0xC3);
    }
    else if (!bDisabled && byteOriginal)
    {
        MemPut<BYTE>(FUNC_DoSunAndMoon, byteOriginal);
        byteOriginal = 0;
    }
}

/*
    Enable or disable corona rain reflections.
    ucEnabled:
     0 - disabled
     1 - enabled
     2 - force enabled (render even if there is no rain)
*/
void CCoronasSA::SetCoronaReflectionsEnabled(unsigned char ucEnabled)
{
    m_ucCoronaReflectionsEnabled = ucEnabled;

    if (ucEnabled == 0)
    {
        // Disable corona rain reflections
        // Return out CCoronas::RenderReflections()
        MemPut<BYTE>(0x6FB630, 0xC3);
    }
    else
    {
        // Enable corona rain reflections
        // Re-enable CCoronas::RenderReflections()
        MemPut<BYTE>(0x6FB630, 0xD9);
    }

    if (ucEnabled == 2)
    {
        // Force enable corona reflections (render even if there is no rain)
        // Disable fWetGripScale check
        MemPut<BYTE>(0x6FB645, 0xEB);

        // Patch "fld fWetGripScale" to "fld fOne"
        MemCpy((void*)0x6FB906, "\x24\x86\x85\x00", 4);
    }
    else
    {
        // Restore patched code
        MemPut<BYTE>(0x6FB645, 0x7A);
        MemCpy((void*)0x6FB906, "\x08\x13\xC8\x00", 4);
    }
}

unsigned char CCoronasSA::GetCoronaReflectionsEnabled()
{
    return m_ucCoronaReflectionsEnabled;
}
