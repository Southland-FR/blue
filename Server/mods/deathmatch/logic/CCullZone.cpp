#include "StdInc.h"
#include "CCullZone.h"
#include "CGroups.h"

// These addresses are based on gta-reversed
static constexpr DWORD FUNC_AddCullZone = 0x72D140;               // guessed
static constexpr DWORD FUNC_RemoveCullZone = 0x72D1B0;            // guessed

typedef int(__cdecl* AddCullZone_t)(float x, float y, float z, float size, float height, unsigned char flags);
typedef void(__cdecl* RemoveCullZone_t)(int id);

CCullZone::CCullZone(CGroups* pGroups, CElement* pParent, const CVector& position, float size, float height, unsigned char flags)
    : CDummy(pGroups, pParent), m_vecPos(position), m_fSize(size), m_fHeight(height), m_ucFlags(flags), m_iZoneId(-1)
{
    m_iType = CElement::UNKNOWN;
    SetTypeName("cullzone");
}

CCullZone::~CCullZone()
{
    RemoveFromGame();
}

bool CCullZone::AddToGame()
{
    AddCullZone_t func = reinterpret_cast<AddCullZone_t>(FUNC_AddCullZone);
    m_iZoneId = func(m_vecPos.fX, m_vecPos.fY, m_vecPos.fZ, m_fSize, m_fHeight, m_ucFlags);
    return m_iZoneId != -1;
}

void CCullZone::RemoveFromGame()
{
    if (m_iZoneId != -1)
    {
        RemoveCullZone_t func = reinterpret_cast<RemoveCullZone_t>(FUNC_RemoveCullZone);
        func(m_iZoneId);
        m_iZoneId = -1;
    }
}
