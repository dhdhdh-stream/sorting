#ifndef HELPERS_H
#define HELPERS_H

#include <map>
#include <utility>
#include <vector>

#include "input.h"

class AbstractNode;
class BranchExperiment;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Signal;
class Solution;
class SolutionWrapper;

void create_experiment(SolutionWrapper* wrapper);

void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs,
						bool& is_on);
void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs,
						bool& is_on,
						int& num_actions_snapshot);
void fetch_histories_helper(ScopeHistory* scope_history,
							std::vector<ScopeHistory*>& scope_context_histories,
							double true_diff,
							AbstractNode* node_context,
							bool is_branch,
							std::vector<ScopeHistory*>& scope_histories,
							std::vector<double>& target_val_histories);

double calc_signal(ScopeHistory* signal_needed_from);
bool check_signal_activate(std::vector<double>& obs,
						   int& action,
						   bool& is_next,
						   SolutionWrapper* wrapper);
bool experiment_check_signal_activate(std::vector<double>& obs,
									  int& action,
									  bool& is_next,
									  bool& fetch_action,
									  SolutionWrapper* wrapper);

Scope* create_new_scope(Scope* scope_context);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

#endif /* HELPERS_H */