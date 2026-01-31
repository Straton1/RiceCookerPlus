#include "hw.h"
#include "pin.h"
#include "out.h"

// put #define variables here

// put global variables here

// helper functions

// main functions

// add documentation here
void out_init(void)
{
    // implement here
}

// might not use this funciton
void out_deinit(void)
{
    // implement here
}

// add documentation here
void out_tick(uint8_t START_FINISH)
{
    // if start_finish has been tripped, go for it
    if (!START_FINISH)
    {
        // start demo

        // check to see if demo is done runnning (using counts from inputs in final product)
        START_FINISH = true;
    }
}
