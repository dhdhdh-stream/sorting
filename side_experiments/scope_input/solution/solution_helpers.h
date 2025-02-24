#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <utility>
#include <vector>

#include "action.h"
#include "input.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;

void create_branch_experiment(ScopeHistory* scope_history);
void create_new_scope_experiment(ScopeHistory* scope_history);

void gather_possible_helper(ScopeHistory* scope_history,
							std::vector<Scope*>& scope_context,
							std::vector<int>& node_context,
							int& node_count,
							Input& new_input);
void gather_possible_factor_helper(ScopeHistory* scope_history,
								   Input& new_factor);
void fetch_input(RunHelper& run_helper,
				 ScopeHistory* scope_history,
				 Input& input,
				 double& val);

void clean_scope(Scope* scope,
				 Solution* parent_solution);

#endif /* SOLUTION_HELPERS_H */