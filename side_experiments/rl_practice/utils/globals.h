#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

class ProblemType;

extern int seed;

extern std::default_random_engine generator;

extern ProblemType* problem_type;

#endif /* GLOBALS_H */