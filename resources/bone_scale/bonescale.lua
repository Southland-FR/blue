local HEAD_BONE = 8 -- BONE_HEAD

local function setHeadScale(scale)
    setElementBoneScale(localPlayer, HEAD_BONE, scale, scale, scale)
end

addCommandHandler("bobble", function(_, value)
    local s = tonumber(value)
    if not s then
        outputChatBox("Usage: /bobble <scale>")
        return
    end
    setHeadScale(s)
end)

addCommandHandler("nobobble", function()
    setHeadScale(1)
end)

