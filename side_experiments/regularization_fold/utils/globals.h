#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

#include "solution.h"

extern std::default_random_engine generator;

extern bool global_debug_flag;
extern double global_sum_error;

extern Solution* solution;

#endif /* GLOBALS_H */