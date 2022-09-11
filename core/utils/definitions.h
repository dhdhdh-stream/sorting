#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <random>

#include "action_dictionary.h"
#include "solution.h"

extern std::default_random_engine generator;

extern Solution* solution;
extern ActionDictionary* action_dictionary;

#endif /* DEFINITIONS_H */