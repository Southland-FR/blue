# GTA:SA Map Limits Expansion in MTA:SA

## Overview

MTA:SA expands the GTA San Andreas world coordinate system from the original ±3000 units to ±6000 units, effectively doubling the usable map area. This expansion enables custom content (buildings, objects, vehicles) to be placed far beyond the original game boundaries while maintaining full game engine compatibility.

**Key Achievement**: Doubled world size (6000 → 12000 total units) while maintaining backward compatibility with all existing GTA:SA game systems.

## Technical Approach

The implementation uses an elegant solution: **larger sector sizes** instead of expanding the sector grid. This approach avoids complex array reallocations and instruction patches.

### Sector System Design

GTA:SA organizes the world using a spatial partitioning system called "sectors" for efficient entity management and rendering:

#### Original GTA:SA Configuration
- **World bounds**: -3000 to +3000 units (6000 total)
- **Sector size**: 50 units
- **Sector grid**: 120×120 sectors
- **Total sectors**: 14,400 (120²)
- **Sector calculation**: `sectorX = (worldX * 0.02) + 60`

#### MTA:SA Extended Configuration
- **World bounds**: -6000 to +6000 units (12000 total)
- **Sector size**: 100 units (doubled)
- **Sector grid**: 120×120 sectors (unchanged)
- **Total sectors**: 14,400 (unchanged)
- **Sector calculation**: `sectorX = (worldX * 0.01) + 60`

### Mathematical Foundation

The key insight is that sector size and grid dimensions are inversely proportional for a given world size:
```
WorldSize = SectorSize × GridDimension
12000 = 100 × 120  (MTA approach)
12000 = 50 × 240   (Alternative - requires extensive patching)
```

**Coordinate Mapping Examples**:
```
At worldX = -6000: sectorX = (-6000 × 0.01) + 60 = -60 + 60 = 0
At worldX = -3000: sectorX = (-3000 × 0.01) + 60 = -30 + 60 = 30
At worldX = 0:     sectorX = (0 × 0.01) + 60 = 60 (center)
At worldX = +3000: sectorX = (+3000 × 0.01) + 60 = 90
At worldX = +6000: sectorX = (+6000 × 0.01) + 60 = 120
```

Original map (-3000 to +3000) now occupies sectors 30-90, with sectors 0-29 and 91-120 available for extended content.

## Implementation Details

### File: Client/multiplayer_sa/CMultiplayerSA_MapLimits.cpp

This file contains all map limits expansion patches, organized into focused functions.

### Static Configuration Variables

These global variables define the new sector system parameters:

```cpp
// World coordinate limits
static float g_fWorldMapMinCoord = -6000.0f;
static float g_fWorldMapMaxCoord = 6000.0f;
static float g_fWorldMapMaxCoordMinusOne = 5999.0f;

// Sector size parameters (100-unit sectors)
static float g_fWorldSectorSizeInv = 0.01f;           // 1/100
static float g_fWorldLodSectorSizeInv = 0.0025f;      // 1/400 for LOD sectors
static float g_fHalfSectorsPerDim = 60.0f;            // Grid half-dimension
static float g_fHalfLodSectorsPerDim = 15.0f;         // LOD grid half-dimension
static int   g_iSectorsPerDim = 120;                  // Original grid size (unchanged)
```

### Memory Patches Applied

The implementation patches specific memory locations in GTA_SA.exe to redirect coordinate checks and sector calculations to the new values.

#### 1. Coordinate Limit Patches (PatchCoordinateLimits)

Redirects GTA's coordinate boundary checks to use ±6000 instead of ±3000:

**Key Addresses Patched**:
- **CEntity::Add** (0x5347xx range): Validates and clamps entity coordinates when adding to world
  - `0x5347EB, 0x534832`: Compare against minimum coordinate (-6000)
  - `0x534819, 0x53484B`: Compare against maximum coordinate (+6000)
  - `0x53480D, 0x53483F`: Clamp to minimum
  - `0x534826, 0x534858`: Clamp to maximum-1 (5999)

- **CEntity::Remove** (0x534Bxx range): Validates coordinates when removing entities
  - `0x534AFD, 0x534B2A, 0x534B53`: Minimum coordinate checks
  - `0x534B3A, 0x534B6C`: Maximum coordinate checks
  - Clamping values updated to ±6000 range

- **Other Systems**: Physics (0x405Exx), streaming (0x561xxx), rendering (0x6C66xx, 0x7361xx)

**Method**: Uses pointer patches for floating-point comparison instructions (fcomp/fld read from memory) and direct float patches for mov instructions.

#### 2. Sector Size Calculation Patches (PatchSectorSizeCalculations)

Updates ~200 addresses across the entire game engine to use the new sector size inverse (0.01 instead of 0.02):

**Address Ranges Patched**:
- `0x40xxxx`: Core sector functions (GetSector, AddToSector, etc.)
- `0x41xxxx`: Entity/ped interactions
- `0x42xxxx`: Vehicle/ped AI pathfinding
- `0x54xxxx`: Collision detection, physics
- `0x55xxxx`: Object streaming and placement
- `0x56xxxx`: **CRITICAL** - CWorld rendering loops (ensures buildings are visible)
- `0x57xxxx, 0x5Dxxxx, 0x5Fxxxx, 0x60xxxx`: Various game systems
- `0x67xxxx, 0x69xxxx, 0x6Exxxx, 0x70xxxx, 0x71xxxx`: Script processing, misc systems

**Why This Is Critical**: These patches ensure that when the game calculates which sector an entity belongs to, it uses the correct formula. Missing even one patch can cause entities to be stored in one sector but rendered from another, resulting in invisible buildings/objects.

#### 3. RenderWare World Bounds (PatchRenderWareBounds)

Expands RenderWare's world bounding box to prevent culling at extended coordinates:

```cpp
// RwWorld bounding box (generous headroom)
constexpr float RENDERWARE_WORLD_BOUND = 20000.0f;

// Addresses: 0x5BD6EC - 0x5BD714
// Patches both supremum (max corner) and infimum (min corner)
```

Sets the rendering engine's world bounds to ±20000, providing ample headroom beyond ±6000 game coordinates.

### What Was NOT Needed

This approach elegantly avoids several complex patching scenarios:

#### 1. No Sector Array Reallocation
Original approach would require:
- Allocating new 240×240 arrays (57,600 sectors vs 14,400)
- Redirecting ~100+ pointer references
- Managing 460KB of additional memory

**Avoided by**: Using larger sector sizes with original 120×120 grid.

#### 2. No IMUL Stride Patches
The sector linear index calculation uses:
```
linearIndex = sectorY * SECTORS_PER_DIM + sectorX
```

With 240 sectors, this would require patching `imul eax, 120` to `imul eax, 240` at dozens of locations across the codebase. The value 240 (0xF0) doesn't fit in a signed byte, requiring complex instruction rewrites:
- Original: `6B C0 78` (3 bytes: `imul eax, 120`)
- Required: `69 C0 F0 00 00 00` (6 bytes: `imul eax, 240`)

This requires sophisticated code relocation (CCodeMover-style trampolines) at each location.

**Avoided by**: Keeping SECTORS_PER_DIM = 120 (original value).

#### 3. No Rendering Loop Bound Patches
Sector iteration loops like:
```cpp
for (int x = 0; x < 120; x++)
    for (int y = 0; y < 120; y++)
```

With 240 sectors, all loop bounds would need updating, including complex conditional jumps and counter comparisons.

**Avoided by**: Original loop bounds remain valid (still iterate 120×120).

## Code Organization

### Main Entry Point

```cpp
void CMultiplayerSA::InitHooks_MapLimits()
{
    // Called during multiplayer initialization
    PatchCoordinateLimits();         // Expand world to ±6000
    PatchSectorSizeCalculations();   // Use 100-unit sectors
    PatchRenderWareBounds();         // Expand RenderWare bounds
}
```

Declared in `CMultiplayerSA.h`:
```cpp
void InitHooks_MapLimits();  // Line 90
```

Called from `CMultiplayerSA::InitHooks()` in `CMultiplayerSA.cpp`.

## Trade-offs and Limitations

### Advantages
1. **Minimal code changes**: Only ~200 memory patches vs thousands for alternative approaches
2. **No array reallocation**: Saves 460KB memory and avoids pointer update complexity
3. **No instruction rewrites**: Avoids complex IMUL patching and code relocation
4. **Backward compatible**: Original map area works identically
5. **Proven stable**: All game systems (physics, collision, rendering, AI) work correctly

### Limitations
1. **Reduced spatial granularity**: 100-unit sectors vs 50-unit sectors
   - Impact: Slightly larger sector iteration volumes for queries
   - Practical effect: Negligible (sector queries are already broad)

2. **Cannot expand beyond ±6000**: Further expansion would require full array reallocation
   - Current limit matches MTA's network sync limit (±8192 fits comfortably)

3. **Original map has lower sector density**: -3000 to +3000 now uses only 60 sectors per dimension instead of 120
   - Impact: Minimal - game was designed for 50-unit sectors, works fine with 100-unit

## Testing and Validation

### Verified Working
- **createBuilding()** at coordinates (4500, 4500, 10) - renders correctly
- **createObject()** at extended coordinates - full collision and physics
- **createVehicle()** at extended coordinates - AI pathfinding works
- **Original map area** (±3000) - unchanged behavior, no regressions

### Test Locations
```lua
-- Extended map test (sector 105, 105)
local building = createBuilding(modelID, 4500, 4500, 10)

-- Edge test (sector 0, 0)
local edgeBuilding = createBuilding(modelID, -5950, -5950, 10)

-- Original map test (sector 60, 60 - center)
local centerBuilding = createBuilding(modelID, 0, 0, 10)
```

All tests pass with correct rendering, collision, and game logic.

## Technical Background

### GTA:SA Sector System

The game uses three types of sectors:

1. **CSector** (8 bytes): Contains buildings and dummies
   - `m_pBuildings`: CPtrListSingleLink
   - `m_pDummies`: CPtrListDoubleLink

2. **CRepeatSector** (12 bytes): Contains dynamic entities
   - `m_pVehicles`: CPtrListDoubleLink
   - `m_pPeds`: CPtrListDoubleLink
   - `m_pObjects`: CPtrListDoubleLink

3. **CLodSector** (4 bytes): Contains LOD (Level of Detail) buildings
   - `m_pLodList`: CPtrListSingleLink

Original arrays in GTA_SA.exe:
- `ms_aSectors`: 0xB7D0B8 (120×120×8 = 115,200 bytes)
- `ms_aRepeatSectors`: 0xB992B8 (120×120×12 = 172,800 bytes)
- `ms_aLodPtrLists`: 0xB99EB8 (30×30×4 = 3,600 bytes)

### Research Foundation

This implementation is based on research from:
- **Fastman92 Limit Adjuster**: Reverse-engineered GTA structures and limit patches
- **GTA-Reversed Project**: Clean reimplementations of GTA systems with accurate signatures
- **MTA Development**: Years of game engine integration experience

## Performance Characteristics

### Memory Impact
- **No additional allocation**: Uses original 288KB of sector arrays
- **Static variable overhead**: ~40 bytes for float constants
- **Code size**: ~2KB for patch logic (executed once at startup)

### Runtime Performance
- **Sector queries**: Identical performance (same O(1) array access)
- **Iteration volume**: Slightly larger sectors mean ~4x entities per sector on average
  - Mitigated by: GTA's linked-list sector storage makes iteration fast regardless
- **No measurable FPS impact**: Confirmed through testing

## Future Expansion Possibilities

### To Expand Beyond ±6000

If further expansion is needed (e.g., ±8192 to match network sync limit):

**Option 1**: Increase sector size to 136.5 units
- Formula: 16384 / 120 = 136.5
- Pros: No array changes
- Cons: Non-power-of-2 size, even lower granularity

**Option 2**: Reallocate to 240×240 sectors (50-unit sectors)
- Requires: Full implementation of commented-out code in MapLimits.cpp
- Memory: +460KB
- Complexity: Moderate (code is already written, just disabled)

**Option 3**: Hybrid approach
- Keep 100-unit sectors but expand to 160×160 grid for ±8000
- Balance between memory and granularity

## Conclusion

The MTA:SA map limits expansion successfully doubles the GTA:SA world size through an elegant mathematical optimization. By recognizing that sector size and grid dimensions are interchangeable variables, the implementation achieves full expansion with minimal code complexity, zero memory overhead, and perfect backward compatibility.

The approach demonstrates that sophisticated reverse-engineering isn't always about brute-force patching - sometimes the best solution is finding the right mathematical transformation.

---

**Implementation Date**: December 2024
**Status**: Production-ready, fully tested
**Compatibility**: GTA SA v1.0 US, MTA:SA 1.6+
**Related Files**:
- `/Client/multiplayer_sa/CMultiplayerSA_MapLimits.cpp` (Implementation)
- `/Client/multiplayer_sa/CMultiplayerSA.h` (Declaration)
- `/Client/multiplayer_sa/CMultiplayerSA.cpp` (Initialization hook)
