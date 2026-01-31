#include "hw.h"
#include "pin.h"
#include "in.h"

// put #define variables here

// put global variables here

// helper functions

// main functions

// add documentation here
void in_init(void)
{
    // implement here
    // i.e. starting state, etc.
}

// might not use this funciton
void in_deinit(void)
{
    // implement here
    // might not need this, depends on what is in init
}

// add documentation here
void in_tick(uint8_t START_FINISH)
{
    // if the cycle is ready to begin (it has finished)
    if (START_FINISH)
    {
        // if the start button has been pressed
        if (!HW_START_DEMO)
        {
            // There will be more input functionality implemented here
            // for now, just start the demo
            START_FINISH = false;
        }
    }
}
