-- infantry.lua
local MELEE_DAMAGE          = 1.0
local MAX_ATTACK_DISTANCE   = 0.1

function onTick(sim, unit, event)
    local foe = sim:getNearestEnemy(unit)

    if foe ~= nil and math.abs(unit:getPos() - foe:getPos()) <= MAX_ATTACK_DISTANCE then        
        sim:stopAnimations(unit)
        --sim:requestAnimation(unit, 'Act: Attack')
        sim:inflictDamage(unit, foe, 15)
    else
        sim:requestAnimation(unit, 'Act: Run_upper')
        sim:requestAnimation(unit, 'Act: Run_lower')
        sim:moveForwards(unit, 0.1)         
    end
end

function onDie(sim, unit, event)
    print('onDie called')
    sim:stopAnimations(unit)
    sim:queueAnimation(unit, 'Act: Death')    
end

events = {
    Tick = onTick,
    Die = onDie,
    Init = function(s, u, e) print('Init called') end,
    Leave = function(s, u, e) print('Leave called') end
}
registerEvents(events)