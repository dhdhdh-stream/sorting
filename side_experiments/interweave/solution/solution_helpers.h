#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <vector>

#include "run_helper.h"

class Scope;
class ScopeHistory;
class Solution;

void create_experiment(ScopeHistory* scope_history);

void gather_possible_helper(ScopeHistory* scope_history,
							std::vector<Scope*>& scope_context,
							std::vector<int>& node_context,
							int& node_count,
							std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,std::pair<int,int>>& new_input);
void gather_possible_factor_helper(ScopeHistory* scope_history,
								   int& factor_count,
								   std::pair<int,int>& new_factor);
void fetch_factor_helper(ScopeHistory* scope_history,
						 std::pair<int,int> factor,
						 double& val);
void fetch_input_helper(ScopeHistory* scope_history,
						std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,std::pair<int,int>>& input,
						int l_index,
						double& obs);

#endif /* SOLUTION_HELPERS_H */