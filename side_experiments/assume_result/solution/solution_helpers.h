#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include "action.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeNode;
class Solution;

void create_experiment(RunHelper& run_helper);

ScopeNode* create_existing();

void clean_scope(Scope* scope);
void clean_branch_node(Scope* scope);
void clean_scope_node_helper(Scope* scope,
							 AbstractNode* original_node,
							 AbstractNode* new_node);
void clean_scope_node(Solution* parent_solution,
					  Scope* to_remove);
void clean_scope_node(Solution* parent_solution,
					  Scope* to_remove,
					  Action to_replace);
void clean_scope_node(Solution* parent_solution,
					  Scope* to_remove,
					  Scope* to_replace);

void get_existing_result(Problem* original_problem,
						 double& result,
						 bool& hit_subproblem);

#endif /* SOLUTION_HELPERS_H */