#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <utility>
#include <vector>

#include "action.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;

void create_experiment(ScopeHistory* scope_history);

void gather_possible_helper(ScopeHistory* scope_history,
							std::vector<int>& scope_context,
							std::vector<int>& node_context,
							int& node_count,
							std::pair<std::pair<std::vector<int>,std::vector<int>>,int>& new_input);
void fetch_input_helper(ScopeHistory* scope_history,
						std::pair<std::pair<std::vector<int>,std::vector<int>>,int>& input,
						int l_index,
						double& obs);

void clean_scope(Scope* scope,
				 Solution* parent_solution);

double get_existing_result(Problem* original_problem);

void mimic(Problem* problem,
		   ScopeHistory* scope_history,
		   int num_steps,
		   std::vector<Action>& mimic_actions);

#endif /* SOLUTION_HELPERS_H */