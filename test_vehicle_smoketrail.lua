-- Simple test to toggle and check smoke trail on a vehicle
local veh = createVehicle(512, 0, 0, 3)
setSmokeTrailEnabled(veh, true)
outputDebugString('Smoke trail enabled: '..tostring(isSmokeTrailEnabled(veh)))
setTimer(function()
    setSmokeTrailEnabled(veh, false)
    outputDebugString('Smoke trail enabled after toggle: '..tostring(isSmokeTrailEnabled(veh)))
end, 2000, 1)
