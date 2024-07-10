#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include "run_helper.h"

class Problem;
class Scope;
class ScopeNode;

void create_experiment(RunHelper& run_helper);

ScopeNode* create_existing();

void clean_scope(Scope* scope);

double get_existing_result(Problem* original_problem);

#endif /* SOLUTION_HELPERS_H */