-- archer.lua
local RANGE_DAMAGE          = 30.0
local MAX_ATTACK_DISTANCE   = 35.0
local WALK_VELOCITY         = 0.05

function onTick(sim, unit, event)
    local foe = sim:getNearestEnemy(unit)       
    if foe ~= nil and sim:getDistance(foe, unit) <= MAX_ATTACK_DISTANCE then               
        sim:stopAnimations(unit)
        sim:requestAnimation(unit, 'Act: Attack')        
		local dmg = math.random() * RANGE_DAMAGE	
        sim:inflictDamage(unit, foe, dmg)
        --sim:throwProjectile(unit, 0.1, 1.0, 20)
    else
        sim:stopAnimations(unit)	
        sim:requestAnimation(unit, 'Act: Walk_upper')
        sim:requestAnimation(unit, 'Act: Walk_lower')
        sim:moveForwards(unit, WALK_VELOCITY)         
    end
end

function onDie(sim, unit, event)
    print('Die called')
    sim:stopAnimations(unit)
    sim:requestAnimation(unit, 'Act: Death')        
end

events = {
    Tick = onTick,
    Die = onDie,
    Init = function(s, u, e) print('Init called') end,
    Leave = function(s, u, e) print('Leave called') end
}
registerEvents(events)