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

void update_scores(ScopeHistory* scope_history,
				   double target_val);

void create_experiment(ScopeHistory* scope_history,
					   int improvement_iter);
void create_multi_experiment(ScopeHistory* scope_history);
void create_multi_branch_experiment(ScopeHistory* scope_history);
void create_multi_commit_experiment(ScopeHistory* scope_history);

void gather_possible_helper(ScopeHistory* scope_history,
							std::vector<Scope*>& scope_context,
							std::vector<int>& node_context,
							int& node_count,
							std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,std::pair<int,int>>& new_input);
void gather_possible_factor_helper(ScopeHistory* scope_history,
								   std::pair<int,int>& new_factor);
void fetch_factor_helper(RunHelper& run_helper,
						 ScopeHistory* scope_history,
						 std::pair<int,int> factor,
						 double& val);
void fetch_input_helper(RunHelper& run_helper,
						ScopeHistory* scope_history,
						std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,std::pair<int,int>>& input,
						int l_index,
						double& obs);

void clean_scope(Scope* scope,
				 Solution* parent_solution);

void multi_iter();
void multi_branch_iter();
void multi_commit_iter();

#endif /* SOLUTION_HELPERS_H */