#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

class WorldModel;

extern int seed;

extern std::default_random_engine generator;

extern WorldModel* world_model;

#endif /* GLOBALS_H */