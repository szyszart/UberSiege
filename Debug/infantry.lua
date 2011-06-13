-- infantry.lua
function onTick(simulation, unit, event)    
    if enemyInRange(unit, MELEE_RANGE) then
        simulation:rinflictDamage(unit)
    else
        simulation:moveForwards(unit)     
    end
end

attacks = {
    anim = "melee_anim",
    damage = 15
}

events = {
    Tick = onTick
}
registerEvents(events)
