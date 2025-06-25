#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

#include <map>
#include <utility>
#include <vector>

#include "input.h"

class AbstractExperiment;
class AbstractNode;
class Network;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;
class SolutionWrapper;

void select_reward_signal(Scope* scope,
						  Input& reward_signal);
void create_experiment(ScopeHistory* scope_history,
					   AbstractExperiment*& curr_experiment,
					   SolutionWrapper* wrapper);
void create_confusion(ScopeHistory* scope_history,
					  SolutionWrapper* wrapper);

void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs,
						bool& is_on);

bool train_existing(std::vector<ScopeHistory*>& scope_histories,
					std::vector<double>& target_val_histories,
					std::map<Input, InputData>& input_tracker,
					double& average_score,
					std::vector<Input>& factor_inputs,
					std::vector<double>& factor_input_averages,
					std::vector<double>& factor_input_standard_deviations,
					std::vector<double>& factor_weights,
					AbstractExperiment* experiment,
					SolutionWrapper* wrapper);
bool train_new(std::vector<ScopeHistory*>& scope_histories,
			   std::vector<double>& target_val_histories,
			   double& average_score,
			   std::vector<Input>& factor_inputs,
			   std::vector<double>& factor_input_averages,
			   std::vector<double>& factor_input_standard_deviations,
			   std::vector<double>& factor_weights,
			   std::vector<Input>& network_inputs,
			   Network*& network,
			   double& select_percentage);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

void check_generalize(Scope* scope_to_generalize,
					  SolutionWrapper* wrapper);

void update_scores(ScopeHistory* scope_history,
				   double target_val,
				   SolutionWrapper* wrapper);
void attach_existing_histories(ScopeHistory* scope_history,
							   double target_val);

bool is_match(std::vector<double>& t_scores);

#endif /* SOLUTION_HELPERS_H */