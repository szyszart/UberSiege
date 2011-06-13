layouts = {
    simple = {
        layout = {"II", ""},
        unit = "archer",
        minTech = 1
    },    
    archer = {
        layout = {"WI", "IW"}, -- { top, bottom }   
        unit = "archer",
        minTech = 1
    },    
    dragon = {
        layout = {"FMMF", "MFFM"},
        unit = "dragon",
        minTech = 4        
    }
}

units = {
    archer =     "archer.lua",
    infantry =   "infantry.lua"
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