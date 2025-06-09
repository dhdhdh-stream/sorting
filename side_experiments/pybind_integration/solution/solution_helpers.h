#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <utility>
#include <vector>

#include "input.h"

class AbstractExperiment;
class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

void create_experiment(ScopeHistory* scope_history,
					   AbstractExperiment*& curr_experiment,
					   SolutionWrapper* wrapper);
void create_confusion(ScopeHistory* scope_history,
					  SolutionWrapper* wrapper);

void gather_possible_helper(ScopeHistory* scope_history,
							std::vector<Scope*>& scope_context,
							std::vector<int>& node_context,
							int& node_count,
							Input& new_input,
							SolutionWrapper* wrapper);
void gather_possible_factor_helper(ScopeHistory* scope_history,
								   std::pair<int,int>& new_factor);
void fetch_factor_helper(ScopeHistory* scope_history,
						 std::pair<int,int> factor,
						 double& val);
void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs);
void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						bool& hit,
						double& obs);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

void check_generalize(Scope* scope_to_generalize,
					  SolutionWrapper* wrapper);

#endif /* SOLUTION_HELPERS_H */