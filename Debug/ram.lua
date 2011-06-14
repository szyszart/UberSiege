-- ram.lua
local ATTACK_DAMAGE          = 100.0
local MAX_ATTACK_DISTANCE    = 12.0
local MOVE_VELOCITY          = 0.05

function onTick(sim, unit, event)
    local foe = sim:getNearestEnemy(unit)       
    if foe ~=nil then
        print(sim:getDistance(foe, unit))
    end
    if foe ~= nil and sim:getDistance(foe, unit) <= MAX_ATTACK_DISTANCE then               
        sim:stopAnimations(unit)
        sim:queueAnimation(unit, 'Act: Attack')        
        sim:inflictDamage(unit, foe, ATTACK_DAMAGE)
    else
        sim:requestAnimation(unit, 'Act: Walk_upper')
        sim:requestAnimation(unit, 'Act: Walk_lower')
        sim:moveForwards(unit, MOVE_VELOCITY)         
    end
end

function onDie(sim, unit, event)
    print('Die called')
    sim:stopAnimations(unit)
    -- sim:queueAnimation(unit, 'Act: Death')        
end

events = {
    Tick = onTick,
    Die = onDie,
    Init = function(s, u, e) print('Init called') end,
    Leave = function(s, u, e) print('Leave called') end
}
registerEvents(events)