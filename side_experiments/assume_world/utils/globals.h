#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

class ProblemType;
class SolutionSet;

extern int seed;

extern std::default_random_engine generator;

extern ProblemType* problem_type;
extern SolutionSet* solution_set;

extern int run_index;

#endif /* GLOBALS_H */