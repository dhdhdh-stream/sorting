#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

class Problem;
class Solution;

extern int seed;

extern std::default_random_engine generator;

extern Problem* problem_type;
extern Solution* solution;

#endif /* GLOBALS_H */