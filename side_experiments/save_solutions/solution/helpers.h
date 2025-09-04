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
class SignalInstance;
class Solution;
class SolutionWrapper;

void set_explore_scope(SolutionWrapper* wrapper);
void create_experiment(SolutionWrapper* wrapper);

void fetch_input_helper(ScopeHistory* scope_history,
						Input& input,
						int l_index,
						double& obs,
						bool& is_on);

const double MIN_CONSIDER_HIT_PERCENT = 0.2;

const double UNIQUE_MAX_PCC = 0.7;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

const double FACTOR_IMPACT_THRESHOLD = 0.1;

const int INPUT_NUM_HIGHEST = 4;
const int INPUT_NUM_RANDOM = 6;

class InputData {
public:
	double hit_percent;
	double average;
	double standard_deviation;
};

void analyze_input(Input& input,
				   std::vector<ScopeHistory*>& scope_histories,
				   InputData& input_data);
void gather_t_scores_helper(ScopeHistory* scope_history,
							std::vector<Scope*>& scope_context,
							std::vector<int>& node_context,
							std::map<Input, double>& t_scores,
							std::vector<ScopeHistory*>& scope_histories,
							std::map<Input, InputData>& input_tracker);
bool is_unique(std::vector<std::vector<double>>& input_vals,
			   std::vector<double>& existing_averages,
			   std::vector<double>& existing_standard_deviations,
			   std::vector<double>& potential_input_vals,
			   double& potential_average,
			   double& potential_standard_deviation);
void train_network(std::vector<std::vector<double>>& inputs,
				   std::vector<std::vector<bool>>& input_is_on,
				   std::vector<double>& target_vals,
				   Network* network);
void measure_network(std::vector<std::vector<double>>& inputs,
					 std::vector<std::vector<bool>>& input_is_on,
					 std::vector<double>& target_vals,
					 Network* network,
					 double& average_misguess,
					 double& misguess_standard_deviation);
void optimize_network(std::vector<std::vector<double>>& inputs,
					  std::vector<std::vector<bool>>& input_is_on,
					  std::vector<double>& target_vals,
					  Network* network);
bool train_existing(std::vector<ScopeHistory*>& scope_histories,
					std::vector<double>& target_val_histories,
					double& average_score,
					std::vector<Input>& factor_inputs,
					std::vector<double>& factor_input_averages,
					std::vector<double>& factor_input_standard_deviations,
					std::vector<double>& factor_weights);
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

bool hit_helper(ScopeHistory* scope_history,
				Scope* scope_context,
				AbstractNode* node_context,
				bool is_branch);
void fetch_histories_helper(ScopeHistory* scope_history,
							double target_val,
							Scope* scope_context,
							AbstractNode* node_context,
							bool is_branch,
							std::vector<ScopeHistory*>& scope_histories,
							std::vector<double>& target_val_histories,
							SolutionWrapper* wrapper);
void fetch_signals_helper(ScopeHistory* scope_history,
						  std::vector<ScopeHistory*>& scope_histories,
						  Scope* scope_context,
						  AbstractNode* node_context,
						  bool is_branch,
						  std::map<Scope*, std::vector<double>>& signals,
						  SolutionWrapper* wrapper);
bool hit_helper(ScopeHistory* scope_history,
				Scope* scope_context);

Scope* create_new_scope(Scope* scope_context);

void clean_scope(Scope* scope,
				 SolutionWrapper* wrapper);

void update_counts(ScopeHistory* scope_history,
				   int h_index);

void calc_signal(std::vector<std::vector<double>>& pre_obs_histories,
				 std::vector<std::vector<double>>& post_obs_histories,
				 std::vector<SignalInstance*>& instances,
				 bool& is_match,
				 double& val);
double calc_signal(std::vector<std::vector<double>>& pre_obs_histories,
				   std::vector<std::vector<double>>& post_obs_histories,
				   std::vector<SignalInstance*>& potential_signals,
				   double potential_miss_average_guess);
double calc_signal(ScopeHistory* signal_needed_from,
				   SolutionWrapper* wrapper);
bool check_signal(std::vector<double>& obs,
				  int& action,
				  bool& is_next,
				  SolutionWrapper* wrapper);
bool signal_experiment_check_signal(std::vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper);

#endif /* HELPERS_H */