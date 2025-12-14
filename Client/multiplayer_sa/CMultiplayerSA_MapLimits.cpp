/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        multiplayer_sa/CMultiplayerSA_MapLimits.cpp
 *  PURPOSE:     Map limits expansion - extends world coordinates from +/-3000 to +/-6000
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *  Based on research from fastman92 limit adjuster and gta-reversed project
 *
 *****************************************************************************/

#include "StdInc.h"

//////////////////////////////////////////////////////////////////////////////////////////
//
// Map Limits Expansion Overview
//
// APPROACH: Use 100-unit sectors instead of 50-unit sectors to fit +/-6000 in 120x120 grid
//
// Original GTA:SA:
//   - World: -3000 to +3000 (6000 units)
//   - Sectors: 120x120 grid, 50 units each
//   - Formula: sectorX = (worldX * 0.02) + 60
//
// MTA Extended:
//   - World: -6000 to +6000 (12000 units)
//   - Sectors: 120x120 grid, 100 units each (same grid, larger sectors)
//   - Formula: sectorX = (worldX * 0.01) + 60
//
// This avoids complex IMUL stride patches and array reallocations.
// Original map (-3000 to +3000) occupies sectors 30-90.
//
//////////////////////////////////////////////////////////////////////////////////////////

// Extended world bounds
constexpr float NEW_WORLD_MIN = -6000.0f;
constexpr float NEW_WORLD_MAX = 6000.0f;
constexpr float NEW_WORLD_MAX_MINUS_ONE = 5999.0f;

// RenderWare world bounds (generous headroom)
constexpr float RENDERWARE_WORLD_BOUND = 20000.0f;

// Static float values for pointer patches (fcomp/fld instructions read from these)
static float g_fWorldMapMinCoord = NEW_WORLD_MIN;           // -6000
static float g_fWorldMapMaxCoord = NEW_WORLD_MAX;           // +6000
static float g_fWorldMapMaxCoordMinusOne = NEW_WORLD_MAX_MINUS_ONE;

// Sector calculation values (100-unit sectors)
// Formula: sectorX = (worldX * g_fWorldSectorSizeInv) + g_fHalfSectorsPerDim
static float g_fWorldSectorSizeInv = 0.01f;                 // 1/100 (100-unit sectors)
static float g_fWorldLodSectorSizeInv = 0.0025f;            // 1/400 (400-unit LOD sectors)
static float g_fHalfSectorsPerDim = 60.0f;                  // Grid half-dimension (120/2)
static float g_fHalfLodSectorsPerDim = 15.0f;               // LOD grid half-dimension (30/2)

//////////////////////////////////////////////////////////////////////////////////////////
//
// PatchCoordinateLimits
//
// Patch coordinate boundary checks from +/-3000 to +/-6000
// Uses pointer patches for fcomp instructions and direct float patches for mov instructions
//
//////////////////////////////////////////////////////////////////////////////////////////
static void PatchCoordinateLimits()
{
    // fWorldMapMinCoord pointer patches (-3000 -> -6000)
    MemPut<void*>(0x47F0E6 + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x47F17B + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x5615DF + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x5615EE + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x561627 + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x6C660F + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x6C6631 + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x7361B0 + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x7361DE + 2, &g_fWorldMapMinCoord);

    // fWorldMapMaxCoord pointer patches (3000 -> 6000)
    MemPut<void*>(0x561604 + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x56161A + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x56163B + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x561645 + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x561659 + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x6C6620 + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x6C6642 + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x70E416 + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x7361C9 + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x7361F3 + 2, &g_fWorldMapMaxCoord);

    // Direct float patches for mov instructions
    MemPut<float>(0x405EE2 + 4, NEW_WORLD_MIN);
    MemPut<float>(0x405EEA + 4, NEW_WORLD_MIN);
    MemPut<float>(0x41140F + 4, NEW_WORLD_MIN);
    MemPut<float>(0x411417 + 4, NEW_WORLD_MIN);
    MemPut<float>(0x405EF2 + 4, NEW_WORLD_MAX);
    MemPut<float>(0x405EFA + 4, NEW_WORLD_MAX);
    MemPut<float>(0x41141F + 4, NEW_WORLD_MAX);
    MemPut<float>(0x411427 + 4, NEW_WORLD_MAX);

    // CEntity::Add coordinate bounds (0x5347xx range)
    // These are BEFORE MTA hooks at 0x5348FB/0x534A10, so safe to patch
    MemPut<void*>(0x5347EB + 2, &g_fWorldMapMinCoord);       // fcomp rect.left < min
    MemPut<void*>(0x534832 + 2, &g_fWorldMapMinCoord);       // fcomp rect.bottom < min
    MemPut<float>(0x53480D + 4, NEW_WORLD_MIN);              // mov clamp to min
    MemPut<float>(0x53483F + 4, NEW_WORLD_MIN);              // mov clamp to min
    MemPut<void*>(0x534819 + 2, &g_fWorldMapMaxCoord);       // fcomp rect.right >= max
    MemPut<void*>(0x53484B + 2, &g_fWorldMapMaxCoord);       // fcomp rect.top >= max
    MemPut<float>(0x534826 + 4, NEW_WORLD_MAX_MINUS_ONE);    // mov clamp right to 5999
    MemPut<float>(0x534858 + 4, NEW_WORLD_MAX_MINUS_ONE);    // mov clamp top to 5999

    // CEntity::Remove coordinate bounds (0x534Bxx range)
    MemPut<void*>(0x534AFD + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x534B2A + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x534B53 + 2, &g_fWorldMapMinCoord);
    MemPut<void*>(0x534B3A + 2, &g_fWorldMapMaxCoord);
    MemPut<void*>(0x534B6C + 2, &g_fWorldMapMaxCoord);
    MemPut<float>(0x534B60 + 4, NEW_WORLD_MIN);
    MemPut<float>(0x534B47 + 4, NEW_WORLD_MAX_MINUS_ONE);
    MemPut<float>(0x534B79 + 4, NEW_WORLD_MAX_MINUS_ONE);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// PatchSectorSizeCalculations
//
// Patch sector size inverse (0.02 -> 0.01) and half-sectors (60) at ~200 addresses
// These patches redirect fmul/fadd instructions to use our values for 100-unit sectors
//
// CRITICAL: Missing patches cause sector index mismatches between entity storage
// and rendering, resulting in invisible buildings/objects.
//
//////////////////////////////////////////////////////////////////////////////////////////
static void PatchSectorSizeCalculations()
{
    // ========================================================================
    // fWorldSectorSizeInversed (0.02 -> 0.01)
    // Original value at 0x858B38, redirect to g_fWorldSectorSizeInv
    // ========================================================================

    // 0x40xxxx range - core sector functions
    MemPut<void*>(0x408536 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40854F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x408569 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x4085BF + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x408607 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x408658 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x4090C6 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x4090EB + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40921D + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40923C + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40D444 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40D451 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40D474 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40D495 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40D4E3 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40D527 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40D578 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40D7EA + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x40D82B + 2, &g_fWorldSectorSizeInv);

    // 0x41xxxx range - entity interaction
    MemPut<void*>(0x41ACED + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x41AD3C + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x41AD80 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x41ADD1 + 2, &g_fWorldSectorSizeInv);

    // 0x42xxxx range - ped spawning
    MemPut<void*>(0x42CECC + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x42CF1C + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x42CF60 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x42CFAC + 2, &g_fWorldSectorSizeInv);

    // 0x43xxxx range - vehicle spawning
    MemPut<void*>(0x432662 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x4326B2 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x4326FE + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x43274F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x434491 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x4344E1 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x434525 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x434576 + 2, &g_fWorldSectorSizeInv);

    // 0x45xxxx range
    MemPut<void*>(0x4538BB + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x45390B + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x453953 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x45399F + 2, &g_fWorldSectorSizeInv);

    // 0x50xxxx range - physics
    MemPut<void*>(0x50F722 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x50F746 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x50F765 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x50F786 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x50F7A9 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x50F7D6 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x50F7FD + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x50F826 + 2, &g_fWorldSectorSizeInv);

    // 0x53xxxx range - CWorld/CEntity functions
    MemPut<void*>(0x533068 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x533085 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x53309F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5330B9 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5330DD + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x533106 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x53491E + 2, &g_fWorldSectorSizeInv);     // CEntity::Add
    MemPut<void*>(0x53493E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x53495F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x53497E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x534C79 + 2, &g_fWorldSectorSizeInv);     // CEntity::Remove
    MemPut<void*>(0x534C99 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x534CBC + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x534CDB + 2, &g_fWorldSectorSizeInv);

    // 0x54xxxx range - streaming/loading
    MemPut<void*>(0x54259D + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5425BD + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5425E0 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5425FF + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x544A7A + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x544A9A + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x544ABD + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x544ADC + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54D9E8 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DA08 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DA29 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DA48 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DC12 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DC2F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DC50 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DC6F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DD0A + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DD2C + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DD4D + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x54DD6C + 2, &g_fWorldSectorSizeInv);

    // 0x55xxxx range - objects/effects
    MemPut<void*>(0x554DED + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x554E13 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x554E27 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x554E3B + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x554E4F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x554E63 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x55544E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x55547B + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x555495 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5554AF + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5554C9 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5554E3 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5554FD + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x555517 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x555531 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x55554B + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x555C00 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x555C26 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x555C3D + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x555C54 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x555C6E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x555C88 + 2, &g_fWorldSectorSizeInv);

    // 0x56xxxx range - CWorld rendering (CRITICAL FOR VISIBLE BUILDINGS)
    MemPut<void*>(0x564608 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x564656 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5646A4 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5646F0 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x564A34 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x564A5A + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x564A7B + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x564A9F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x564C84 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x564CAA + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x564CCB + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x564CEF + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x566AA5 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x566AC2 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x566AE5 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x566B04 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x567519 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x567538 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x567652 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56766E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x568B94 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x568BBA + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x568BDB + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x568BFF + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x568DE0 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x568DFC + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x568E22 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x568E3E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x568FF8 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56901A + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56903D + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56905C + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x569250 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56926C + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x569292 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5692AE + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x569404 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56942A + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56944B + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56946F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x569E2F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x569E5B + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x569E7E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x569EA3 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56A4BF + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56A4DE + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56A501 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56A51D + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56A99B + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56A9B6 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56A9D4 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56A9F4 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56AA2D + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56ABC1 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56AFB1 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56AFCC + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56AFEA + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56B00A + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56B043 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56B1E1 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56B7A3 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56B7C9 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56B7EA + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56B80E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56BA42 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56BA62 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56BA8A + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56BAA6 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56BFA3 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56BFBE + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56BFE3 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56C003 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56C03E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56C235 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56C697 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56C6B2 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56C6D7 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56C6F7 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56C732 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56C941 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56DC7E + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56DCCE + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56DD12 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x56DD63 + 2, &g_fWorldSectorSizeInv);

    // 0x57xxxx range
    MemPut<void*>(0x57067A + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x570697 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5706B8 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5706D7 + 2, &g_fWorldSectorSizeInv);

    // 0x5Dxxxx range - shadows/effects
    MemPut<void*>(0x5DC5F1 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5DC647 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5DC68F + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5DC6E0 + 2, &g_fWorldSectorSizeInv);

    // 0x5Fxxxx range
    MemPut<void*>(0x5FFACA + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5FFAF9 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5FFB2D + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5FFB57 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5FFB86 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5FFBB0 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5FFBE4 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x5FFC0F + 2, &g_fWorldSectorSizeInv);

    // 0x60xxxx range
    MemPut<void*>(0x605BA8 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x605BD7 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x605C10 + 2, &g_fWorldSectorSizeInv);
    MemPut<void*>(0x605C48 + 2, &g_fWorldSectorSizeInv);

    // ========================================================================
    // fNumberOfWorldSectorsPerDimensionHalf (60.0)
    // Original value at 0x858B34, redirect to g_fHalfSectorsPerDim
    // ========================================================================

    // 0x40xxxx range - core sector functions
    MemPut<void*>(0x40853C + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x408555 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40856F + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x4085C5 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40860D + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40865E + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x4090D7 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x4090F1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x409226 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x409246 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40D45E + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40D47E + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40D49D + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40D4E9 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40D52D + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40D57E + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40D7F3 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x40D831 + 2, &g_fHalfSectorsPerDim);

    // 0x41xxxx range
    MemPut<void*>(0x41ACF6 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x41AD42 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x41AD86 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x41ADD7 + 2, &g_fHalfSectorsPerDim);

    // 0x42xxxx range
    MemPut<void*>(0x42CED2 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x42CF22 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x42CF66 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x42CFB2 + 2, &g_fHalfSectorsPerDim);

    // 0x43xxxx range
    MemPut<void*>(0x432668 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x4326B8 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x432704 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x432755 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x434497 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x4344E7 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x43452B + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x43457C + 2, &g_fHalfSectorsPerDim);

    // 0x45xxxx range
    MemPut<void*>(0x4538C1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x453911 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x453959 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x4539A5 + 2, &g_fHalfSectorsPerDim);

    // 0x50xxxx range
    MemPut<void*>(0x50F728 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x50F74E + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x50F76B + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x50F78E + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x50F7AF + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x50F7DE + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x50F803 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x50F82E + 2, &g_fHalfSectorsPerDim);

    // 0x53xxxx range - CWorld/CEntity
    MemPut<void*>(0x533071 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x53308B + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5330A5 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5330BF + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5330E3 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x53310C + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x534927 + 2, &g_fHalfSectorsPerDim);      // CEntity::Add
    MemPut<void*>(0x534948 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x534967 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x534988 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x534C82 + 2, &g_fHalfSectorsPerDim);      // CEntity::Remove
    MemPut<void*>(0x534CA5 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x534CC4 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x534CE5 + 2, &g_fHalfSectorsPerDim);

    // 0x54xxxx range
    MemPut<void*>(0x5425A6 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5425C9 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5425E8 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x542609 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x544A83 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x544AA6 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x544AC5 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x544AE6 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54D9F1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DA12 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DA31 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DA50 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DC18 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DC39 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DC58 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DC77 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DD15 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DD36 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DD55 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x54DD74 + 2, &g_fHalfSectorsPerDim);

    // 0x55xxxx range
    MemPut<void*>(0x554DFF + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x554E19 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x554E2D + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x554E41 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x554E55 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x554E69 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555462 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555481 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x55549B + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5554B5 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5554CF + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5554E9 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555503 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x55551D + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555537 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555551 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555C14 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555C2C + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555C43 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555C5A + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555C74 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x555C8E + 2, &g_fHalfSectorsPerDim);

    // 0x56xxxx range - CWorld rendering (CRITICAL FOR VISIBLE BUILDINGS)
    MemPut<void*>(0x56460E + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56465F + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5646AA + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5646F6 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x564A3A + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x564A60 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x564A81 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x564AA5 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x564C8A + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x564CB0 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x564CD1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x564CF5 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x566AAB + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x566ACE + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x566AED + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x566B0E + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x567522 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x567540 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x567658 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x567676 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x568B9A + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x568BC0 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x568BE1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x568C05 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x568DE6 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x568E06 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x568E28 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x568E48 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569003 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569026 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569045 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569066 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569256 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569276 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569298 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5692B8 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56940A + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569430 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569451 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569475 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569E3A + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569E61 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569E84 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x569EA9 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56A4C8 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56A4E6 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56A507 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56A527 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56A9A1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56A9BE + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56A9DE + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56A9FC + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56AA33 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56ABC7 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56AFB7 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56AFD4 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56AFF4 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56B012 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56B049 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56B1E7 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56B7A9 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56B7CF + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56B7F0 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56B814 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56BA4C + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56BA6A + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56BA90 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56BAB0 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56BFA9 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56BFCD + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56BFED + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56C00D + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56C044 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56C23B + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56C69D + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56C6C1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56C6E1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56C701 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56C738 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56C947 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56DC84 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56DCD4 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56DD18 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x56DD69 + 2, &g_fHalfSectorsPerDim);

    // 0x57xxxx range
    MemPut<void*>(0x570680 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5706A1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5706C0 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5706DF + 2, &g_fHalfSectorsPerDim);

    // 0x5Dxxxx range
    MemPut<void*>(0x5DC5F7 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5DC64D + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5DC695 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5DC6E6 + 2, &g_fHalfSectorsPerDim);

    // 0x5Fxxxx range
    MemPut<void*>(0x5FFAD6 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5FFAFF + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5FFB33 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5FFB5D + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5FFB8C + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5FFBB6 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5FFBEA + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x5FFC15 + 2, &g_fHalfSectorsPerDim);

    // 0x60xxxx range
    MemPut<void*>(0x605BB1 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x605BE0 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x605C19 + 2, &g_fHalfSectorsPerDim);
    MemPut<void*>(0x605C51 + 2, &g_fHalfSectorsPerDim);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// PatchRenderWareBounds
//
// Expand RenderWare world bounding box to prevent culling at extended coordinates
//
//////////////////////////////////////////////////////////////////////////////////////////
static void PatchRenderWareBounds()
{
    // RwWorld bounding box - supremum (maximum corner)
    MemPut<float>(0x5BD6EC + 4, RENDERWARE_WORLD_BOUND);     // sup.z
    MemPut<float>(0x5BD6F4 + 4, RENDERWARE_WORLD_BOUND);     // sup.y
    MemPut<float>(0x5BD6FC + 4, RENDERWARE_WORLD_BOUND);     // sup.x

    // RwWorld bounding box - infimum (minimum corner)
    MemPut<float>(0x5BD704 + 4, -RENDERWARE_WORLD_BOUND);    // inf.z
    MemPut<float>(0x5BD70C + 4, -RENDERWARE_WORLD_BOUND);    // inf.y
    MemPut<float>(0x5BD714 + 4, -RENDERWARE_WORLD_BOUND);    // inf.x
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// InitHooks_MapLimits
//
// Main entry point - applies all map limit expansion patches
// Called from CMultiplayerSA::InitHooks()
//
//////////////////////////////////////////////////////////////////////////////////////////
void CMultiplayerSA::InitHooks_MapLimits()
{
    // Patch coordinate limits (expand world bounds to +/-6000)
    PatchCoordinateLimits();

    // Patch sector size calculations (100-unit sectors via 0.01 inverse)
    PatchSectorSizeCalculations();

    // Patch RenderWare world bounds
    PatchRenderWareBounds();
}
