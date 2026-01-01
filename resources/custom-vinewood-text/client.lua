--[[
    Custom Vinewood Text - MTA:SA Resource
    Original CLEO mod by Junior_Djjr (MixMods.com.br)
    MTA:SA port - Exact 1:1 positioning logic from CLEO script

    Allows customizing the Vinewood sign text with any A-Z, 0-9 characters.
]]

-- Configuration
local config = {
    text = "VINEWOOD",
    position = { x = 1413.0, y = -804.0, z = 100.0 },
    spacing = 1.2,  -- Multiplier for letter width (CLEO default: 1.2)
    drawDistance = 500
}

-- Original Vinewood sign object IDs to remove
local originalSignModels = {
    13722,  -- VineSign1_cunte (main sign)
    13831,  -- VineSign1_cunte01 (secondary)
    13759   -- LODVineSign1_cunte (LOD)
}

-- Character to file mapping
local charToFiles = {}
for i = 0, 9 do
    local char = tostring(i)
    charToFiles[char] = {
        dff = "assets/dff/cust_vin_text_" .. i .. ".dff",
        col = "assets/col/cust_vin_text_" .. i .. ".col"
    }
end
for i = 0, 25 do
    local char = string.char(65 + i)  -- A=65
    charToFiles[char] = {
        dff = "assets/dff/cust_vin_text_" .. char .. ".dff",
        col = "assets/col/cust_vin_text_" .. char .. ".col"
    }
end

-- State tracking
local allocatedModels = {}      -- model ID -> true (for cleanup)
local charModelCache = {}       -- character -> allocated model ID
local charWidthCache = {}       -- character -> width (from model dimensions)
local createdObjects = {}       -- list of created letter objects
local loadedDFFs = {}           -- character -> DFF element
local loadedCOLs = {}           -- character -> COL element
local loadedTXD = nil           -- shared TXD element for all letters
local isInitialized = false

-- Space width constant (same as CLEO: 2.0)
local SPACE_WIDTH = 2.0

-- Load configuration from XML
local function loadConfig()
    local configFile = xmlLoadFile("config.xml")
    if configFile then
        local textNode = xmlFindChild(configFile, "text", 0)
        if textNode then
            config.text = xmlNodeGetValue(textNode) or config.text
        end

        local posNode = xmlFindChild(configFile, "position", 0)
        if posNode then
            config.position.x = tonumber(xmlNodeGetAttribute(posNode, "x")) or config.position.x
            config.position.y = tonumber(xmlNodeGetAttribute(posNode, "y")) or config.position.y
            config.position.z = tonumber(xmlNodeGetAttribute(posNode, "z")) or config.position.z
        end

        local drawDistNode = xmlFindChild(configFile, "drawDistance", 0)
        if drawDistNode then
            config.drawDistance = tonumber(xmlNodeGetValue(drawDistNode)) or config.drawDistance
        end

        local spacingNode = xmlFindChild(configFile, "spacing", 0)
        if spacingNode then
            config.spacing = tonumber(xmlNodeGetValue(spacingNode)) or config.spacing
        end

        xmlUnloadFile(configFile)
    end

    config.text = string.upper(config.text)
    outputDebugString("[CustomVinewood] Loaded config: text='" .. config.text .. "', spacing=" .. config.spacing)
end

-- Remove original Vinewood sign objects
local function removeOriginalSigns()
    for _, modelID in ipairs(originalSignModels) do
        removeWorldModel(modelID, 10000, 0, 0, 0)
        outputDebugString("[CustomVinewood] Removed original sign model: " .. modelID)
    end
end

-- Allocate a model ID and load DFF + COL for a character
local function loadCharacterModel(char)
    if charModelCache[char] then
        return charModelCache[char]
    end

    if char == " " then
        return nil
    end

    local files = charToFiles[char]
    if not files then
        outputDebugString("[CustomVinewood] No files for character: " .. char, 2)
        return nil
    end

    local modelID = engineRequestModel("object")
    if not modelID then
        outputDebugString("[CustomVinewood] Failed to request model ID for: " .. char, 1)
        return nil
    end

    allocatedModels[modelID] = true

    local dff = engineLoadDFF(files.dff)
    if not dff then
        outputDebugString("[CustomVinewood] Failed to load DFF: " .. files.dff, 1)
        engineFreeModel(modelID)
        allocatedModels[modelID] = nil
        return nil
    end
    loadedDFFs[char] = dff

    if not engineReplaceModel(dff, modelID) then
        outputDebugString("[CustomVinewood] Failed to replace model: " .. modelID, 1)
        engineFreeModel(modelID)
        allocatedModels[modelID] = nil
        return nil
    end

    -- Load and apply TXD texture (shared across all letters)
    if not loadedTXD then
        loadedTXD = engineLoadTXD("assets/mulhouslahills.txd")
        if loadedTXD then
            outputDebugString("[CustomVinewood] Loaded TXD: mulhouslahills.txd")
        else
            outputDebugString("[CustomVinewood] Failed to load TXD: mulhouslahills.txd", 2)
        end
    end

    if loadedTXD then
        engineImportTXD(loadedTXD, modelID)
    end

    local col = engineLoadCOL(files.col)
    if col then
        loadedCOLs[char] = col
        engineReplaceCOL(col, modelID)
    end

    engineSetModelLODDistance(modelID, config.drawDistance)

    -- Get model dimensions for width calculation (CLEO uses get_model_dimensions)
    -- MTA doesn't have getModelBoundingBox, so we create a temp object and use getElementBoundingBox
    local tempObj = createObject(modelID, 0, 0, -1000)  -- Create far below ground
    if tempObj then
        local minX, minY, minZ, maxX, maxY, maxZ = getElementBoundingBox(tempObj)
        destroyElement(tempObj)
        if minX and maxX then
            -- CLEO uses: 19@ = abs(width) * 1.2  (width is the X dimension)
            -- config.spacing replaces the hardcoded 1.2 multiplier
            local width = math.abs(maxX - minX) * config.spacing
            charWidthCache[char] = width
            outputDebugString("[CustomVinewood] Character '" .. char .. "' width: " .. width)
        else
            charWidthCache[char] = SPACE_WIDTH
        end
    else
        charWidthCache[char] = SPACE_WIDTH
    end

    charModelCache[char] = modelID
    outputDebugString("[CustomVinewood] Loaded character '" .. char .. "' as model " .. modelID)

    return modelID
end

-- Load all unique characters in the text
local function loadAllCharacters()
    local uniqueChars = {}
    for i = 1, #config.text do
        local char = string.sub(config.text, i, i)
        if char ~= " " and not uniqueChars[char] then
            uniqueChars[char] = true
        end
    end

    for char in pairs(uniqueChars) do
        loadCharacterModel(char)
    end
end

-- Get character width (from cache or default)
local function getCharWidth(char)
    if char == " " then
        return SPACE_WIDTH
    end
    return charWidthCache[char] or SPACE_WIDTH
end

-- Create a single letter object at position
local function createLetterAt(char, x, y, z)
    local modelID = charModelCache[char]
    if not modelID then
        return nil
    end

    -- Apply Y and Z offsets exactly like CLEO (lines 149-150)
    -- Using fixed offsets - ground detection causes issues with letter collision
    local finalY = y + 2.0
    local finalZ = z - 1.0

    local obj = createObject(modelID, x, finalY, finalZ, 0, 0, 0)
    if obj then
        setElementDoubleSided(obj, true)
        setElementCollisionsEnabled(obj, true)

        -- Create LOD object (CLEO lines 151-153)
        local lodObj = createObject(modelID, x, finalY, finalZ, 0, 0, 0, true)
        if lodObj then
            setLowLODElement(obj, lodObj)
            setElementDoubleSided(lodObj, true)
            table.insert(createdObjects, lodObj)
        end

        table.insert(createdObjects, obj)
        return obj
    end
    return nil
end

--[[
    EXACT CLEO POSITIONING LOGIC (from @Noname_255 and @Noname_535):

    The algorithm places letters from CENTER outward:
    1. Place center character at base X position
    2. Place left characters (indices center-1 down to 0) moving left
    3. Place right characters (indices center+1 up to length-1) moving right

    Offset accumulation:
    - For left side: offset decreases by character width BEFORE and AFTER placing
    - For right side: offset increases by character width BEFORE and AFTER placing
    - Base position is RESET for each character, then offset is applied
]]
local function createLetterObjects()
    local text = config.text
    local textLen = #text

    if textLen == 0 then
        outputDebugString("[CustomVinewood] No text to display", 2)
        return
    end

    local baseX = config.position.x  -- 9@ in CLEO = 1413.0
    local baseY = config.position.y  -- 10@ in CLEO = -804.0
    local baseZ = config.position.z  -- 11@ in CLEO = 100.0

    -- 5@ = 7@ / 2 (center index, integer division)
    local centerIndex = math.floor(textLen / 2)  -- 0-indexed in CLEO, we'll adjust

    -- In CLEO, string indices are 0-based. In Lua, they're 1-based.
    -- CLEO center for "VINEWOOD" (len=8): index 4 = 'W' (0-indexed)
    -- Lua equivalent: index 5 = 'W' (1-indexed)
    local luaCenterIndex = centerIndex + 1

    outputDebugString("[CustomVinewood] Text: '" .. text .. "', length: " .. textLen .. ", center index: " .. luaCenterIndex)

    -- Variables matching CLEO:
    -- 14@ = current offset
    -- 15@ = first character width (stored after placing center)
    -- 19@ = current character width
    local offset = 0      -- 14@
    local firstWidth = 0  -- 15@

    -- ============================================
    -- PASS 0: Place center character (10@ = 0)
    -- ============================================
    local centerChar = string.sub(text, luaCenterIndex, luaCenterIndex)
    local centerWidth = getCharWidth(centerChar)

    -- CLEO: 16@ = 2@ (reset to base X), then 16@ += 14@ (offset is 0)
    local posX = baseX + offset  -- offset is 0, so posX = baseX

    if centerChar ~= " " then
        createLetterAt(centerChar, posX, baseY, baseZ)
        outputDebugString("[CustomVinewood] Center '" .. centerChar .. "' at X=" .. posX)
    end

    -- After placing center: 15@ = 19@ (store first width)
    firstWidth = centerWidth

    -- ============================================
    -- PASS 1: Left side (10@ = 1)
    -- Process indices from (centerIndex - 1) down to 0
    -- ============================================
    -- 14@ = 15@ * -1.0 (offset starts as negative of first width)
    offset = -firstWidth

    -- CLEO loop: while 5@ > 0, then 5@ -= 1, process char at 5@
    -- This processes indices: centerIndex-1, centerIndex-2, ..., 0
    local leftIndex = centerIndex  -- Will be decremented before use (0-indexed)

    while leftIndex > 0 do
        leftIndex = leftIndex - 1  -- Decrement first (like CLEO)
        local luaIndex = leftIndex + 1  -- Convert to 1-indexed
        local char = string.sub(text, luaIndex, luaIndex)
        local charWidth = getCharWidth(char)

        -- CLEO @Noname_535: if 10@ == 1, then 14@ -= 19@ (before placing)
        offset = offset - charWidth

        -- CLEO: 16@ = 2@ (reset to base), then 16@ += 14@
        posX = baseX + offset

        if char ~= " " then
            createLetterAt(char, posX, baseY, baseZ)
            outputDebugString("[CustomVinewood] Left '" .. char .. "' at X=" .. posX .. " (index " .. luaIndex .. ")")
        end

        -- CLEO: after placing, if 10@ == 1, then 14@ -= 19@ (again)
        offset = offset - charWidth
    end

    -- ============================================
    -- PASS 2: Right side (10@ = 2)
    -- Process indices from (centerIndex + 1) up to (textLen - 1)
    -- ============================================
    -- 14@ = 15@ (offset starts as positive first width)
    offset = firstWidth

    -- 5@ = 7@ / 2 (reset to center)
    local rightIndex = centerIndex  -- 0-indexed

    -- CLEO loop: while 7@ > 5@, then 5@ += 1, process char at 5@
    -- This processes indices: centerIndex+1, centerIndex+2, ..., textLen-1
    while textLen > rightIndex + 1 do  -- +1 because we increment before processing
        rightIndex = rightIndex + 1  -- Increment first (like CLEO)
        local luaIndex = rightIndex + 1  -- Convert to 1-indexed
        local char = string.sub(text, luaIndex, luaIndex)
        local charWidth = getCharWidth(char)

        -- CLEO @Noname_535: if 10@ == 2, then 14@ += 19@ (before placing)
        offset = offset + charWidth

        -- CLEO: 16@ = 2@ (reset to base), then 16@ += 14@
        posX = baseX + offset

        if char ~= " " then
            createLetterAt(char, posX, baseY, baseZ)
            outputDebugString("[CustomVinewood] Right '" .. char .. "' at X=" .. posX .. " (index " .. luaIndex .. ")")
        end

        -- CLEO: after placing, if 10@ == 2, then 14@ += 19@ (again)
        offset = offset + charWidth
    end

    outputDebugString("[CustomVinewood] Created " .. #createdObjects .. " letter objects")
end

-- Clean up all resources
local function cleanup()
    outputDebugString("[CustomVinewood] Cleaning up...")

    for _, obj in ipairs(createdObjects) do
        if isElement(obj) then
            destroyElement(obj)
        end
    end
    createdObjects = {}

    for _, dff in pairs(loadedDFFs) do
        if isElement(dff) then
            destroyElement(dff)
        end
    end
    loadedDFFs = {}

    for _, col in pairs(loadedCOLs) do
        if isElement(col) then
            destroyElement(col)
        end
    end
    loadedCOLs = {}

    if loadedTXD and isElement(loadedTXD) then
        destroyElement(loadedTXD)
        loadedTXD = nil
    end

    for modelID in pairs(allocatedModels) do
        engineFreeModel(modelID)
    end
    allocatedModels = {}
    charModelCache = {}
    charWidthCache = {}

    for _, modelID in ipairs(originalSignModels) do
        restoreWorldModel(modelID, 10000, 0, 0, 0)
    end

    isInitialized = false
    outputDebugString("[CustomVinewood] Cleanup complete")
end

-- Initialize the resource
local function initialize()
    if isInitialized then
        return
    end

    outputDebugString("[CustomVinewood] Initializing...")

    loadConfig()
    removeOriginalSigns()
    loadAllCharacters()
    createLetterObjects()

    isInitialized = true
    outputDebugString("[CustomVinewood] Initialization complete")
end

-- Event handlers
addEventHandler("onClientResourceStart", resourceRoot, initialize)
addEventHandler("onClientResourceStop", resourceRoot, cleanup)

-- Export function to change text dynamically
function setVinewoodText(newText)
    if type(newText) ~= "string" or #newText == 0 then
        return false
    end

    for _, obj in ipairs(createdObjects) do
        if isElement(obj) then
            destroyElement(obj)
        end
    end
    createdObjects = {}

    config.text = string.upper(newText)
    loadAllCharacters()
    createLetterObjects()

    return true
end

-- Command to change text (for testing)
addCommandHandler("vinewood", function(cmd, ...)
    local newText = table.concat({...}, " ")
    if #newText > 0 then
        if setVinewoodText(newText) then
            outputChatBox("[CustomVinewood] Text changed to: " .. newText)
        else
            outputChatBox("[CustomVinewood] Failed to change text", 255, 0, 0)
        end
    else
        outputChatBox("[CustomVinewood] Usage: /vinewood <text>")
    end
end)
