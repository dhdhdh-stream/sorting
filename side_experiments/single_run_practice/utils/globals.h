#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

class ProblemType;
class World;

extern std::default_random_engine generator;

extern ProblemType* problem_type;
extern World* world;

#endif /* GLOBALS_H */