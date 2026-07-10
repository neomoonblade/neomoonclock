#include "timeutil.h"

#include <time.h>

bool time_get_current_nanotime(uint64_t* _time) {
	struct timespec current_time;
	uint64_t current_time_ns;
	
	if(timespec_get(&current_time, TIME_UTC) == TIME_UTC) {
		current_time_ns = ((uint64_t) current_time.tv_sec * 1000000000ULL) + current_time.tv_nsec;
		*_time = current_time_ns;
	} else {
		return false;
	}
	
	return true;
}
