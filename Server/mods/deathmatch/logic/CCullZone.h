#pragma once

#include "CDummy.h"
#include <CVector.h>

class CCullZone : public CDummy
{
public:
    CCullZone(CGroups* pGroups, CElement* pParent, const CVector& position, float size, float height, unsigned char flags);
    ~CCullZone();

    void RemoveFromGame();
    bool AddToGame();

private:
    CVector       m_vecPos;
    float         m_fSize;
    float         m_fHeight;
    unsigned char m_ucFlags;
    int           m_iZoneId;
};
