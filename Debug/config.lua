layouts = {
    infantry = {
        layout = {"II", ""},
        unit = "infantry"
    },    
    ram = {
        layout = {"WI", "IW"}, -- { top, bottom }   
        unit = "ram"
    },    
    archer = {
        layout = {"MM", ""},
        unit = "archer"
    }
}

units = {
    archer =     "archer.lua",
    infantry =   "infantry.lua",
    ram =        "ram.lua"
}

bindings = {    
    P1_MOVE_UP     = "KC_W",
    P1_MOVE_DOWN   = "KC_S",
    P1_MOVE_LEFT   = "KC_A",    
    P1_MOVE_RIGHT  = "KC_D",
    P1_SELECT      = "KC_Q",
    P1_CLEAR       = "KC_Z",
    P1_CONFIRM     = "KC_E",

    P2_MOVE_UP     = "KC_I",
    P2_MOVE_DOWN   = "KC_K",
    P2_MOVE_LEFT   = "KC_J",    
    P2_MOVE_RIGHT  = "KC_L",
    P2_SELECT      = "KC_U",
    P2_CLEAR       = "KC_M",
    P2_CONFIRM     = "KC_P"    
}