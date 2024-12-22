#pragma once
#include "pin_config.h"


#define DEBUG true


#if DEBUG
    #define debug(x) Serial.print(x)
    #define debugln(x) Serial.println(x)
#else
    #define debug(x)
    #define debugln(x)
#endif

#define LED_COUNT 85
