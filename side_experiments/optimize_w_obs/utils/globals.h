#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

class ActionNetwork;
class LSTM;
class ProblemType;
class Solution;

extern int seed;

extern std::default_random_engine generator;

extern ProblemType* problem_type;
extern Solution* solution;

extern std::vector<LSTM*> mimic_memory_cells;
extern std::vector<ActionNetwork*> mimic_action_networks;

extern int run_index;

#endif /* GLOBALS_H */