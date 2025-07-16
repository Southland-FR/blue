-- Bones that make up the head/neck chain. These correspond to
-- BONE_NECK (5) up to BONE_HEAD (8).
local TARGET_BONES = {5, 6, 7, 8}

-- Current scale that should be applied to every bone in TARGET_BONES.
local currentScale = 1

-- Continuously apply the scale to the player's bones so it stays active
-- even when animations change.
addEventHandler("onClientPreRender", root, function()
    for _, bone in ipairs(TARGET_BONES) do
        setElementBoneScale(localPlayer, bone, currentScale, currentScale, currentScale)
    end
    updateElementRpHAnim(localPlayer)
end)

-- Command to change the scale of the head/neck bones.
addCommandHandler("bobble", function(_, value)
    local s = tonumber(value)
    if not s then
        outputChatBox("Usage: /bobble <scale>")
        return
    end
    currentScale = s
end)

-- Command to reset the bones to their original size.
addCommandHandler("nobobble", function()
    currentScale = 1
end)

