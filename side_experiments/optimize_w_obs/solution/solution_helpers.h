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

void create_experiment(RunHelper& run_helper);

void gather_possible_helper(ScopeHistory* scope_history,
							std::vector<Scope*>& scope_context,
							std::vector<int>& node_context,
							int& node_count,
							std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,int>& new_input);
void fetch_input_helper(ScopeHistory* scope_history,
						std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,int>& input,
						int l_index,
						double& obs);

void clean_scope(Scope* scope,
				 Solution* parent_solution);

void get_existing_result(Problem* original_problem,
						 RunHelper& run_helper);

#endif /* SOLUTION_HELPERS_H */