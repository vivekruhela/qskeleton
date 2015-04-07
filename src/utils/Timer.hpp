#ifndef UTILS_TIMER_HPP
#define UTILS_TIMER_HPP


// In case OpenMP is available use high-precision OpenMP timer,
// otherwise go with rather low-precision clock(). Both are portable.
#ifdef USE_OPENMP

namespace Utils
{
#include <omp.h>
double getTimeSec()
{
    return omp_get_wtime();
}
}

#else

#include <ctime>
namespace Utils
{
double getTimeSec()
{
    return (double)clock() / CLOCKS_PER_SEC; 
}
}

#endif


#endif
