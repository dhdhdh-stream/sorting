#ifndef GLOBALS_H
#define GLOBALS_H

#include <random>

class Problem;
class Solution;

extern int seed;

extern std::default_random_engine generator;

extern Problem* problem_type;
extern Solution* solution;

extern int num_actions_until_experiment;
extern int num_actions_after_experiment_to_skip;
extern bool eval_experiment;

#endif /* GLOBALS_H */