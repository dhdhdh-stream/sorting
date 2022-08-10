#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <random>

#include "action_dictionary.h"
#include "loop_dictionary.h"

extern std::default_random_engine generator;

extern ActionDictionary* action_dictionary;
extern LoopDictionary* loop_dictionary;

#endif /* DEFINITIONS_H */