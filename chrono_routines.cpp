#include "chrono_routines.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(__APPLE__)
#include <stdint.h>
#include <mach/mach_time.h>
#endif

#if defined(__ANDROID__) || defined(__gnu_linux__)
#define USE_TIME
#include <time.h>
#include <sys/time.h>
#endif

namespace chronoroutines {
  steady_clock::time_point steady_clock::now() {
#if defined(_WIN32)
    __int64 freq = 0;
    if (!QueryPerformanceFrequency((LARGE_INTEGER *) &freq))
      return time_point();
    __int64 cur = 0;
    QueryPerformanceCounter((LARGE_INTEGER *) &cur);
    return time_point((cur / freq) * 1000);
#endif

#if defined(__APPLE__)
    mach_timebase_info_data_t time_info;
    mach_timebase_info(&time_info);

    uint64_t cur = mach_absolute_time();
    return time_point((cur / (time_info.denom * 1000000)) * time_info.numer);
#endif

#if defined(USE_TIME)
    // struct timeval te;
    // gettimeofday(&te, 0);
    // return time_point(te.tv_sec * 1000 + te.tv_usec / 1000);
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return time_point(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif

    return time_point();
  }
}
