/*
 * hs_time.cpp
 *
 *  Created on: Dec 7, 2021
 *      Author: sgriffin
 */

#include "hs_time.h"
#include <sys/time.h>

namespace hitspool {

    u64 get_system_time(){
        struct timeval time_now = {};
        gettimeofday(&time_now, NULL);
        time_t microsec_time = time_now.tv_sec * 1000000 + time_now.tv_usec;
        return microsec_time;
    }



}


