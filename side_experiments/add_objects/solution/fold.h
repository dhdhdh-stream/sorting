// TODO: zero train by tracking separate set of state, selecting 1 per run randomly forward

#ifndef FOLD_H
#define FOLD_H

#include <map>
#include <set>
#include <vector>

#include "run_helper.h"
#include "scope.h"
#include "state_network.h"

const int FOLD_STATE_EXPERIMENT = 0;	// with 1 new outer, 1 new local

const int FOLD_STATE_EXPERIMENT_FAIL = 1;

const int FOLD_STATE_ADD_OUTER_STATE = 2;
const int FOLD_STATE_ADD_INNER_STATE = 3;

// freeze external scopes/nodes

const int FOLD_STATE_REMOVE_INNER_INPUT = 4;

const int FOLD_STATE_EXPERIMENT_DONE = 5;

const int FOLD_STATE_REMOVE_OUTER_SCOPE = 6;
// don't worry about removing outer states individually as will have no impact on scopes
const int FOLD_STATE_REMOVE_OUTER_SCOPE_NETWORK = 7;

const int FOLD_STATE_REMOVE_INNER_SCOPE = 8;
const int FOLD_STATE_REMOVE_INNER_SCOPE_NETWORK = 9;

const int FOLD_STATE_REMOVE_INNER_NETWORK = 10;
const int FOLD_STATE_REMOVE_INNER_STATE = 11;
const int FOLD_STATE_CLEAR_INNER_STATE = 12;

const int FOLD_STATE_DONE = 13;

const int FOLD_RESULT_FAIL = 0;
const int FOLD_RESULT_BRANCH = 1;
const int FOLD_RESULT_REPLACE = 2;

class FoldHistory;
class ScopeHistory;
class Fold {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]
	int exit_depth;		// 0 is local

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;

	// keep fixed even if parent scope updates
	int num_score_states;
	int num_sequence_states;

	int sum_inner_inputs;
	std::vector<int> inner_input_start_indexes;
	std::vector<int> num_inner_inputs;	// keep track here fixed even if scope updates
	// TODO: inner and ending scale_mods

	int state;
	int state_iter;
	int sub_iter;
	double sum_error;

	int curr_num_new_outer_states;
	// for new outer state, for inner, use as input, but don't modify
	std::map<int, std::vector<std::vector<StateNetwork*>>> curr_outer_state_networks;
	StateNetwork* curr_starting_score_network;
	// no starting state networks as ideally, state should already capture everything relevant

	int curr_num_new_inner_states;	// in addition to sum_inner_inputs, starts at 1
	std::vector<std::vector<StateNetwork*>> curr_state_networks;
	std::vector<StateNetwork*> curr_score_networks;
	std::map<int, std::vector<std::vector<StateNetwork*>>> curr_inner_state_networks;

	int test_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_outer_state_networks;
	StateNetwork* test_starting_score_network;

	int test_num_new_inner_states;
	std::vector<std::vector<StateNetwork*>> test_state_networks;
	std::vector<StateNetwork*> test_score_networks;	// compare against curr_score_networks rather than score, as easier to measure
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_inner_state_networks;

	std::vector<bool> curr_inner_inputs_needed;
	std::vector<bool> test_inner_inputs_needed;
	// set state_networks_not_needed and state_not_needed_locally on experiment_to_clean

	int existing_sequence_length;
	double* existing_average_score;
	double* existing_score_variance;
	double* existing_average_misguess;
	double* existing_misguess_variance;

	double curr_branch_average_score;
	double curr_existing_average_improvement;
	double curr_replace_average_score;
	double curr_replace_average_misguess;
	double curr_replace_misguess_variance;

	double test_branch_average_score;
	double test_existing_average_improvement;
	double test_replace_average_score;
	double test_replace_average_misguess;
	double test_replace_misguess_variance;

	int is_recursive;

	int experiment_result;

	bool experiment_added_state;

	int remove_inner_input_index;

	int clean_outer_scope_index;
	std::set<int> curr_outer_scopes_needed;
	std::set<std::pair<int, int>> curr_outer_contexts_needed;
	std::set<int> reverse_test_outer_scopes_needed;
	std::set<std::pair<int, int>> reverse_test_outer_contexts_needed;

	int clean_outer_node_index;
	int clean_outer_state_index;
	std::map<int, std::vector<std::vector<bool>>> curr_outer_state_networks_not_needed;
	std::map<int, std::vector<std::vector<bool>>> test_outer_state_networks_not_needed;

	int clean_inner_scope_index;
	std::set<int> curr_inner_scopes_needed;
	std::set<std::pair<int, int>> curr_inner_contexts_needed;
	std::set<int> reverse_test_inner_scopes_needed;
	std::set<std::pair<int, int>> reverse_test_inner_contexts_needed;

	int clean_inner_node_index;
	int clean_inner_state_index;
	std::map<int, std::vector<std::vector<bool>>> curr_inner_state_networks_not_needed;
	std::map<int, std::vector<std::vector<bool>>> test_inner_state_networks_not_needed;

	int clean_inner_step_index;

	std::vector<std::vector<bool>> curr_state_networks_not_needed;
	std::vector<std::vector<bool>> test_state_networks_not_needed;

	std::vector<std::vector<bool>> curr_state_not_needed_locally;
	std::vector<std::vector<bool>> test_state_not_needed_locally;

	std::vector<int> curr_num_states_cleared;
	std::vector<int> test_num_states_cleared;

	// TODO: add seeding

	Fold(std::vector<int> scope_context,
		 std::vector<int> node_context,
		 int exit_depth,
		 int sequence_length,
		 std::vector<bool> is_inner_scope,
		 std::vector<int> existing_scope_ids,
		 int existing_sequence_length,
		 double* existing_average_score,
		 double* existing_score_variance,
		 double* existing_average_misguess,
		 double* existing_misguess_variance);
	Fold(std::ifstream& input_file,
		 int scope_id,
		 int scope_index);
	~Fold();

	void score_activate(std::vector<double>& state_vals,
						double& predicted_score,
						double& scale_factor,
						std::vector<ScopeHistory*>& context_histories,
						RunHelper& run_helper,
						FoldHistory* history);
	void sequence_activate(std::vector<double>& state_vals,
						   std::vector<bool>& states_initialized,
						   std::vector<std::vector<double>>& flat_vals,
						   double& predicted_score,
						   double& scale_factor,
						   double& sum_impact,
						   RunHelper& run_helper,
						   FoldHistory* history);
	void backprop(std::vector<double>& state_errors,
				  std::vector<bool>& states_initialized,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  FoldHistory* history);

	void experiment_increment();

	void experiment_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
												ScopeHistory* scope_history,
												std::vector<int>& curr_scope_context,
												std::vector<int>& curr_node_context,
												RunHelper& run_helper,
												FoldHistory* history);
	void experiment_score_activate(std::vector<double>& state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   std::vector<ScopeHistory*>& context_histories,
								   RunHelper& run_helper,
								   FoldHistory* history);
	void experiment_inner_scope_activate_helper(std::vector<double>& new_state_vals,
												ScopeHistory* scope_history,
												std::vector<int>& curr_scope_context,
												std::vector<int>& curr_node_context,
												RunHelper& run_helper,
												int step_index,
												FoldHistory* history);
	void experiment_sequence_activate(std::vector<double>& state_vals,
									  std::vector<bool>& states_initialized,
									  std::vector<std::vector<double>>& flat_vals,
									  double& predicted_score,
									  double& scale_factor,
									  RunHelper& run_helper,
									  FoldHistory* history);
	void experiment_backprop(std::vector<double>& state_errors,
							 std::vector<bool>& states_initialized,
							 double target_val,
							 double final_misguess,
							 double& predicted_score,
							 double& scale_factor,
							 RunHelper& run_helper,
							 FoldHistory* history);

	void experiment_end();
	void add_inner_state_end();
	void add_outer_state_end();

	void remove_inner_input_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
														ScopeHistory* scope_history,
														std::vector<int>& curr_scope_context,
														std::vector<int>& curr_node_context,
														RunHelper& run_helper,
														FoldHistory* history);
	void remove_inner_input_score_activate(std::vector<double>& state_vals,
										   double& predicted_score,
										   double& scale_factor,
										   std::vector<ScopeHistory*>& context_histories,
										   RunHelper& run_helper,
										   FoldHistory* history);
	void remove_inner_input_inner_scope_activate_helper(std::vector<double>& new_state_vals,
														ScopeHistory* scope_history,
														std::vector<int>& curr_scope_context,
														std::vector<int>& curr_node_context,
														RunHelper& run_helper,
														int step_index,
														FoldHistory* history);
	void remove_inner_input_sequence_activate(std::vector<double>& state_vals,
											  std::vector<bool>& states_initialized,
											  std::vector<std::vector<double>>& flat_vals,
											  double& predicted_score,
											  double& scale_factor,
											  RunHelper& run_helper,
											  FoldHistory* history);
	void remove_inner_input_backprop(std::vector<double>& state_errors,
									 std::vector<bool>& states_initialized,
									 double target_val,
									 double final_misguess,
									 double& predicted_score,
									 double& scale_factor,
									 RunHelper& run_helper,
									 FoldHistory* history);

	void remove_inner_input_end();

	void experiment_to_clean();

	void clean_increment();

	void remove_outer_scope_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
														ScopeHistory* scope_history,
														std::vector<int>& curr_scope_context,
														std::vector<int>& curr_node_context,
														RunHelper& run_helper,
														FoldHistory* history,
														std::vector<double>& test_new_outer_state_vals);
	void remove_outer_scope_score_activate(std::vector<double>& state_vals,
										   std::vector<ScopeHistory*>& context_histories,
										   RunHelper& run_helper,
										   FoldHistory* history);
	void remove_outer_scope_inner_scope_activate_helper(std::vector<double>& new_state_vals,
														ScopeHistory* scope_history,
														std::vector<int>& curr_scope_context,
														std::vector<int>& curr_node_context,
														RunHelper& run_helper,
														int step_index,
														FoldHistory* history,
														std::vector<double>& test_new_state_vals,
														std::vector<std::vector<std::vector<StateNetworkHistory*>>>& test_inner_state_network_histories);
	void remove_outer_scope_sequence_activate(std::vector<double>& state_vals,
											  std::vector<bool>& states_initialized,
											  std::vector<std::vector<double>>& flat_vals,
											  double& predicted_score,
											  double& scale_factor,
											  double& sum_impact,
											  RunHelper& run_helper,
											  FoldHistory* history);

	void remove_outer_scope_end();

	void remove_outer_scope_network_outer_scope_activate_helper(
		std::vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history,
		std::vector<int>& curr_scope_context,
		std::vector<int>& curr_node_context,
		RunHelper& run_helper,
		FoldHistory* history,
		std::vector<double>& test_new_outer_state_vals);
	void remove_outer_scope_network_score_activate(
		std::vector<double>& state_vals,
		std::vector<ScopeHistory*>& context_histories,
		RunHelper& run_helper,
		FoldHistory* history);
	void remove_outer_scope_network_inner_scope_activate_helper(
		std::vector<double>& new_state_vals,
		ScopeHistory* scope_history,
		std::vector<int>& curr_scope_context,
		std::vector<int>& curr_node_context,
		RunHelper& run_helper,
		int step_index,
		FoldHistory* history);
	void remove_outer_scope_network_sequence_activate(
		std::vector<double>& state_vals,
		std::vector<bool>& states_initialized,
		std::vector<std::vector<double>>& flat_vals,
		double& predicted_score,
		double& scale_factor,
		double& sum_impact,
		RunHelper& run_helper,
		FoldHistory* history);

	void remove_outer_scope_network_end();

	void clean_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
										   ScopeHistory* scope_history,
										   std::vector<int>& curr_scope_context,
										   std::vector<int>& curr_node_context,
										   RunHelper& run_helper,
										   FoldHistory* history);
	void clean_score_activate(std::vector<double>& state_vals,
							  std::vector<ScopeHistory*> context_histories,
							  RunHelper& run_helper,
							  FoldHistory* history);
	void clean_inner_scope_activate_helper(std::vector<double>& new_state_vals,
										   std::vector<bool>& new_state_vals_initialized,
										   ScopeHistory* scope_history,
										   std::vector<int>& curr_scope_context,
										   std::vector<int>& curr_node_context,
										   RunHelper& run_helper,
										   int step_index,
										   FoldHistory* history,
										   std::vector<double>& test_new_state_vals,
										   std::vector<bool>& test_new_state_vals_initialized,
										   std::vector<std::vector<std::vector<StateNetworkHistory*>>>& test_inner_state_network_histories);
	void clean_sequence_activate(std::vector<double>& state_vals,
								 std::vector<bool>& states_initialized,
								 std::vector<std::vector<double>>& flat_vals,
								 double& predicted_score,
								 double& scale_factor,
								 double& sum_impact,
								 RunHelper& run_helper,
								 FoldHistory* history);
	void clean_backprop(std::vector<double>& state_errors,
						std::vector<bool>& states_initialized,
						double target_val,
						double final_misguess,
						double final_sum_impact,
						double& predicted_score,
						double& scale_factor,
						RunHelper& run_helper,
						FoldHistory* history);

	void remove_inner_scope_end();
	void remove_inner_scope_network_end();
	
	void clean_transform_helper();
	bool remove_inner_network_transform_helper();
	bool remove_inner_state_transform_helper();
	bool clear_inner_state_transform_helper();

	void remove_inner_network_end();
	void remove_inner_state_end();
	void clear_inner_state_end();

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);

	void remove_outer_scope_from_load();
	void remove_outer_scope_network_from_load();
	void remove_inner_scope_from_load();
	void remove_inner_scope_network_from_load();
	void remove_inner_network_from_load();
	void remove_inner_state_from_load();
	void clear_inner_state_from_load();
};

class FoldHistory {
public:
	Fold* fold;

	// to track between score and sequence
	std::vector<double> new_outer_state_vals;
	std::vector<double> test_new_outer_state_vals;

	std::vector<std::vector<StateNetworkHistory*>> outer_state_network_histories;
	double starting_score_update;
	StateNetworkHistory* starting_score_network_history;

	std::vector<std::vector<StateNetworkHistory*>> test_outer_state_network_histories;
	double test_starting_score_update;
	StateNetworkHistory* test_starting_score_network_history;

	std::vector<std::vector<StateNetworkHistory*>> state_network_histories;
	std::vector<ScopeHistory*> inner_scope_histories;
	std::vector<double> score_network_updates;
	std::vector<StateNetworkHistory*> score_network_histories;
	std::vector<std::vector<std::vector<StateNetworkHistory*>>> inner_state_network_histories;

	int state_iter_snapshot;	// heuristic to try to catch if state change occurred

	FoldHistory(Fold* fold);
	~FoldHistory();
};

#endif /* FOLD_H */