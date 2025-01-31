#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

class ProblemType;
class Solution;

extern std::default_random_engine generator;

extern ProblemType* problem_type;
extern Solution* solution;

#endif /* GLOBALS_H */