#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

class ProblemType;
class Solution;

extern int seed;

extern std::default_random_engine generator;

extern ProblemType* problem_type;
extern Solution* solution;

extern int run_index;

extern int multi_counter;

#endif /* GLOBALS_H */