#ifndef ROUTINES_H
#define ROUTINES_H

#include <stdint.h>

extern "C" uint64_t arm_kvtophys(uint64_t va);

extern "C" uint64_t arbitrary_call(uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4, uint64_t x5, uint64_t x6, uint64_t x7, uint64_t func, uint64_t *retval);

#endif // ROUTINES_H