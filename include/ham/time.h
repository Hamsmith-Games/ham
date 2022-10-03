/*
 * Ham Runtime
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_TIME_H
#define HAM_TIME_H 1

/**
 * @defgroup HAM_TIME Time
 * @ingroup HAM
 * @{
 */

#include "ham/typedefs.h"

#include <time.h>
#include <math.h>

typedef struct timespec ham_timepoint;
typedef struct timespec ham_duration;

ham_nonnull_args(1)
ham_nothrow static inline bool ham_timepoint_now(ham_timepoint *ret, clockid_t clk_id){
	const int res = clock_gettime(clk_id, ret);
	if(res != 0){
		// TODO: signal error
		return false;
	}

	return true;
}

ham_nothrow static inline ham_duration ham_sleep(ham_duration dur){
	ham_duration rem = (ham_duration){ 0, 0 };
	const int res = nanosleep(&dur, &rem);
	if(res != 0 && res != EINTR){
		// TODO: signal error
	}

	return rem;
}

ham_constexpr ham_nothrow static inline ham_duration ham_timepoint_diff(ham_timepoint t0, ham_timepoint t1){
	ham_duration dt = {
		.tv_sec = t1.tv_sec - t0.tv_sec,
		.tv_nsec = t1.tv_nsec - t0.tv_nsec,
	};

	if(dt.tv_nsec < 0){
		dt.tv_nsec += 1000000000;
		dt.tv_sec--;
	}

	return dt;
}

ham_constexpr ham_nothrow static inline ham_f64 ham_duration_to_seconds64(ham_duration dur){
	return (ham_f64)dur.tv_sec + ((ham_f64)dur.tv_nsec * (1.0 / 1000000000.0));
}

ham_constexpr ham_nothrow static inline ham_duration ham_duration_from_seconds64(ham_f64 dt){
	ham_f64 int_part;
	ham_f64 frac_part = modf(ham_max(dt, 0.0), &int_part);
	return (ham_duration){
		.tv_sec = (time_t)int_part,
		.tv_nsec = (long)(frac_part * 1000000000),
	};
}

typedef struct ham_ticker{
	ham_timepoint loop, end;
	ham_f64 avg_clock_dt;
} ham_ticker;

ham_nonnull_args(1)
ham_nothrow static inline void ham_ticker_reset(ham_ticker *ticker){
	ham_timepoint_now(&ticker->loop, CLOCK_MONOTONIC);

	const int num_clocks = 2000;

	for(int i = 0; i < num_clocks; i++){
		ham_sleep(ham_duration{ 0, 0 });
		ham_timepoint_now(&ticker->end, CLOCK_MONOTONIC);
	}

	const ham_duration clock_dur = ham_timepoint_diff(ticker->loop, ticker->end);
	ticker->avg_clock_dt = ham_duration_to_seconds64(clock_dur) / ((ham_f64)num_clocks);

	ham_timepoint_now(&ticker->loop, CLOCK_MONOTONIC);
	ticker->end = ticker->loop;
}

ham_nonnull_args(1)
ham_nothrow static inline ham_f64 ham_ticker_tick(ham_ticker *ticker, ham_f64 target_dt){
	const int clk_id = CLOCK_MONOTONIC;

	target_dt = ham_max(target_dt - ticker->avg_clock_dt, 0.0);

	ham_timepoint_now(&ticker->end, clk_id);
	ham_duration dur = ham_timepoint_diff(ticker->loop, ticker->end);
	ham_f64 dt = ham_duration_to_seconds64(dur);

	const ham_f64 sleep_dt = ham_max(target_dt - dt, 0.0);
	ham_duration sleep_dur = ham_duration_from_seconds64(sleep_dt);
	while((sleep_dur.tv_nsec + sleep_dur.tv_sec) != 0){
		sleep_dur = ham_sleep(sleep_dur);
	}

	ham_timepoint_now(&ticker->end, clk_id);
	dur = ham_timepoint_diff(ticker->loop, ticker->end);
	dt = ham_duration_to_seconds64(dur);

	ticker->loop = ticker->end;
	return dt;
}

/**
 * @}
 */

#endif // !HAM_TIME_H
