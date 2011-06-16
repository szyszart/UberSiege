-- infantry.lua
local MELEE_DAMAGE          = 50.0
local MAX_ATTACK_DISTANCE   = 12.0
local RUN_VELOCITY          = 0.1

function onTick(sim, unit, event)
    local foe = sim:getNearestEnemy(unit)       
    if foe ~= nil and sim:getDistance(foe, unit) <= MAX_ATTACK_DISTANCE then               
        --sim:queueAnimation(unit, 'Act: Action')
		sim:stopAnimations(unit)
		sim:requestAnimation(unit, 'Act: Action')		
		local dmg = math.random() * MELEE_DAMAGE
        sim:inflictDamage(unit, foe, dmg)
    else		
		sim:stopAnimations(unit)	
        sim:requestAnimation(unit, 'Act: Run_upper')
        sim:requestAnimation(unit, 'Act: Run_lower')
        sim:moveForwards(unit, RUN_VELOCITY)
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
