#ifndef SOLUTION_HELPERS_H
#define SOLUTION_HELPERS_H

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

const double MIN_CONSIDER_HIT_PERCENT = 0.2;

const int NUM_FACTORS = 10;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;
const double REGRESSION_FAIL_MULTIPLIER = 1000.0;

const double FACTOR_IMPACT_THRESHOLD = 0.1;

const int INPUT_NUM_HIGHEST = 4;
const int INPUT_NUM_RANDOM_PER = 3;

void create_experiment(SolutionWrapper* wrapper,
					   AbstractExperiment*& curr_experiment);

void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs,
						bool& is_on);

void analyze_input(Input& input,
				   std::vector<ScopeHistory*>& scope_histories,
				   InputData& input_data);
void existing_add_factor(std::vector<ScopeHistory*>& scope_histories,
						 std::vector<Input>& network_inputs,
						 Network* network,
						 Input& new_input,
						 AbstractExperiment* experiment);
bool train_existing(std::vector<ScopeHistory*>& scope_histories,
					std::vector<double>& target_val_histories,
					double& average_score,
					std::vector<Input>& factor_inputs,
					std::vector<double>& factor_input_averages,
					std::vector<double>& factor_input_standard_deviations,
					std::vector<double>& factor_weights,
					AbstractExperiment* experiment);
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
				   int h_index);

#endif /* SOLUTION_HELPERS_H */