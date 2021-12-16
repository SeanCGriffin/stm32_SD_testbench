/*
 * hs_time.cpp
 *
 *  Created on: Dec 7, 2021
 *      Author: sgriffin
 */

#include "stm32h7xx.h"
#include "hs_time.h"
#include <sys/time.h>

namespace hitspool {

    u64 get_system_time(){

        return HAL_GetTick();
    }



}


