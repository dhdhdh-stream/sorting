/**
 * Notes:
 * - explore after 1 iteration
 *   - so LoopFold/Scope can still loop 0 times, but will effectively start from 1
 *     - if Scope is later reused though, can have 0 iterations
 * 
 * - assume all newly added state needed for continue/halt networks
 *   - so no need to remove or clear inner state
 * 
 * - keep looping until clear signal to halt
 *   - this is because may be difficult to train to look ahead, when initially forcing number of iterations
 *     - negative signals from continuing can cancel out the target positive signals?
 *   - if leads to issues, then loop will simply not be added
 *     - and otherwise, can find loops that would otherwise be difficult to find
 *   - though might also not matter too much?
 */

#ifndef LOOP_FOLD_H
#define LOOP_FOLD_H

#include <map>
#include <set>
#include <vector>

#include "run_helper.h"
#include "scope.h"
#include "state_network.h"

const int LOOP_FOLD_STATE_EXPERIMENT = 0;

const int LOOP_FOLD_STATE_EXPERIMENT_FAIL = 1;

const int LOOP_FOLD_STATE_ADD_OUTER_STATE = 2;
const int LOOP_FOLD_STATE_ADD_INNER_STATE = 3;

const int LOOP_FOLD_STATE_REMOVE_INNER_INPUT = 4;

// TODO: remove unneeded num_states

const int LOOP_FOLD_STATE_EXPERIMENT_DONE = 5;

const int LOOP_FOLD_STATE_REMOVE_OUTER_SCOPE = 6;
const int LOOP_FOLD_STATE_REMOVE_OUTER_SCOPE_NETWORK = 7;

const int LOOP_FOLD_STATE_REMOVE_INNER_SCOPE = 8;
const int LOOP_FOLD_STATE_REMOVE_INNER_SCOPE_NETWORK = 9;

const int LOOP_FOLD_STATE_REMOVE_INNER_NETWORK = 10;

const int LOOP_FOLD_STATE_DONE = 11;

const int LOOP_FOLD_SUB_STATE_LEARN = 0;
const int LOOP_FOLD_SUB_STATE_MEASURE = 1;

class LoopFoldHistory;
class ScopeHistory;
class LoopFold {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;
	std::vector<Action> actions;

	int num_states;

	int sum_inner_inputs;
	std::vector<int> inner_input_start_indexes;
	std::vector<int> num_inner_inputs;
	std::vector<Scale*> inner_scope_scale_mods;

	int state;
	int sub_state;
	int state_iter;
	int sub_iter;
	double sum_error;

	int curr_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> curr_outer_state_networks;

	std::vector<StateNetwork*> curr_starting_state_networks;
	// only for new_inner_state

	StateNetwork* curr_continue_score_network;
	StateNetwork* curr_continue_misguess_network;
	StateNetwork* curr_halt_score_network;
	StateNetwork* curr_halt_misguess_network;

	int curr_num_new_inner_states;
	std::vector<std::vector<StateNetwork*>> curr_state_networks;
	std::vector<StateNetwork*> curr_score_networks;
	std::map<int, std::vector<std::vector<StateNetwork*>>> curr_inner_state_networks;

	int test_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_outer_state_networks;

	std::vector<StateNetwork*> test_starting_state_networks;

	StateNetwork* test_continue_score_network;
	StateNetwork* test_continue_misguess_network;
	StateNetwork* test_halt_score_network;
	StateNetwork* test_halt_misguess_network;

	int test_num_new_inner_states;
	std::vector<std::vector<StateNetwork*>> test_state_networks;
	std::vector<StateNetwork*> test_score_networks;
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_inner_state_networks;

	std::vector<bool> curr_inner_inputs_needed;
	std::vector<bool> test_inner_inputs_needed;

	double* existing_average_score;
	double* existing_score_variance;
	double* existing_average_misguess;
	double* existing_misguess_variance;

	double curr_average_score;
	double curr_score_variance;
	double curr_average_misguess;
	double curr_misguess_variance;

	double test_average_score;
	double test_score_variance;
	double test_average_misguess;
	double test_misguess_variance;

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

	LoopFold(std::vector<int> scope_context,
			 std::vector<int> node_context,
			 int sequence_length,
			 std::vector<bool> is_inner_scope,
			 std::vector<int> existing_scope_ids,
			 std::vector<Action> actions,
			 double* existing_average_score,
			 double* existing_score_variance,
			 double* existing_average_misguess,
			 double* existing_misguess_variance);
	LoopFold(std::ifstream& input_file,
			 int scope_id,
			 int scope_index);
	~LoopFold();

	void activate(Problem& problem,
				  std::vector<double>& state_vals,
				  std::vector<bool>& states_initialized,
				  double& predicted_score,
				  double& scale_factor,
				  double& sum_impact,
				  std::vector<ScopeHistory*>& context_histories,
				  RunHelper& run_helper,
				  LoopFoldHistory* history);
	void backprop(std::vector<double>& state_errors,
				  std::vector<bool>& states_initialized,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  LoopFoldHistory* history);

	void experiment_increment();

	void experiment_learn_outer_scope_activate_helper(
		std::vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history,
		LoopFoldHistory* history);
	void experiment_learn_inner_scope_activate_helper(
		std::vector<double>& new_state_vals,
		ScopeHistory* scope_history,
		int iter_index,
		int step_index,
		LoopFoldHistory* history);
	void experiment_learn_activate(Problem& problem,
								   std::vector<double>& state_vals,
								   std::vector<bool>& states_initialized,
								   double& predicted_score,
								   double& scale_factor,
								   std::vector<ScopeHistory*>& context_histories,
								   RunHelper& run_helper,
								   LoopFoldHistory* history);
	void experiment_learn_backprop(std::vector<double>& state_errors,
								   std::vector<bool>& states_initialized,
								   double target_val,
								   double final_misguess,
								   double& predicted_score,
								   double& scale_factor,
								   RunHelper& run_helper,
								   LoopFoldHistory* history);

	void experiment_measure_outer_scope_activate_helper(
		std::vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history);
	void experiment_measure_inner_scope_activate_helper(
		std::vector<double>& new_state_vals,
		ScopeHistory* scope_history);
	void experiment_measure_activate(Problem& problem,
									 std::vector<double>& state_vals,
									 std::vector<bool>& states_initialized,
									 double& predicted_score,
									 double& scale_factor,
									 std::vector<ScopeHistory*>& context_histories,
									 RunHelper& run_helper,
									 LoopFoldHistory* history);
	void experiment_measure_backprop(double target_val,
									 double final_misguess);

	void experiment_end();
	void add_outer_state_end();
	void add_inner_state_end();

	void remove_inner_input_learn_outer_scope_activate_helper(
		std::vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history,
		LoopFoldHistory* history);
	void remove_inner_input_learn_inner_scope_activate_helper(
		std::vector<double>& new_state_vals,
		ScopeHistory* scope_history,
		int iter_index,
		int step_index,
		LoopFoldHistory* history);
	void remove_inner_input_learn_activate(Problem& problem,
										   std::vector<double>& state_vals,
										   std::vector<bool>& states_initialized,
										   double& predicted_score,
										   double& scale_factor,
										   std::vector<ScopeHistory*>& context_histories,
										   RunHelper& run_helper,
										   LoopFoldHistory* history);
	void remove_inner_input_learn_backprop(std::vector<double>& state_errors,
										   std::vector<bool>& states_initialized,
										   double target_val,
										   double final_misguess,
										   double& predicted_score,
										   double& scale_factor,
										   RunHelper& run_helper,
										   LoopFoldHistory* history);

	void remove_inner_input_measure_outer_scope_activate_helper(
		std::vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history);
	void remove_inner_input_measure_inner_scope_activate_helper(
		std::vector<double>& new_state_vals,
		ScopeHistory* scope_history);
	void remove_inner_input_measure_activate(Problem& problem,
											 std::vector<double>& state_vals,
											 std::vector<bool>& states_initialized,
											 double& predicted_score,
											 double& scale_factor,
											 std::vector<ScopeHistory*>& context_histories,
											 RunHelper& run_helper,
											 LoopFoldHistory* history);
	void remove_inner_input_measure_backprop(double target_val,
											 double final_misguess);

	void remove_inner_input_end();

	void experiment_to_clean();

	void clean_increment();

	void remove_outer_scope_outer_scope_activate_helper(
		std::vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history,
		std::vector<int>& curr_scope_context,
		std::vector<int>& curr_node_context,
		RunHelper& run_helper,
		LoopFoldHistory* history,
		std::vector<double>& test_new_outer_state_vals,
		std::vector<std::vector<StateNetworkHistory*>>& test_outer_state_network_histories);
	void remove_outer_scope_inner_scope_activate_helper(
		std::vector<double>& new_state_vals,
		ScopeHistory* scope_history,
		RunHelper& run_helper,
		int iter_index,
		int step_index,
		LoopFoldHistory* history,
		std::vector<double>& test_new_state_vals,
		std::vector<std::vector<std::vector<std::vector<StateNetworkHistory*>>>>& test_inner_state_network_histories);
	void remove_outer_scope_activate(Problem& problem,
									 std::vector<double>& state_vals,
									 std::vector<bool>& states_initialized,
									 double& predicted_score,
									 double& scale_factor,
									 double& sum_impact,
									 std::vector<ScopeHistory*>& context_histories,
									 RunHelper& run_helper,
									 LoopFoldHistory* history);

	void remove_outer_scope_end();

	void remove_outer_scope_network_outer_scope_activate_helper(
		std::vector<double>& new_outer_state_vals,
		ScopeHistory* scope_history,
		RunHelper& run_helper,
		LoopFoldHistory* history,
		std::vector<double>& test_new_outer_state_vals,
		std::vector<std::vector<StateNetworkHistory*>>& test_outer_state_network_histories);
	void remove_outer_scope_network_inner_scope_activate_helper(
		std::vector<double>& new_state_vals,
		ScopeHistory* scope_history,
		RunHelper& run_helper,
		int iter_index,
		int step_index,
		LoopFoldHistory* history);
	void remove_outer_scope_network_activate(Problem& problem,
											 std::vector<double>& state_vals,
											 std::vector<bool>& states_initialized,
											 double& predicted_score,
											 double& scale_factor,
											 double& sum_impact,
											 std::vector<ScopeHistory*>& context_histories,
											 RunHelper& run_helper,
											 LoopFoldHistory* history);

	void remove_outer_scope_network_end();

	void clean_outer_scope_activate_helper(std::vector<double>& new_outer_state_vals,
										   ScopeHistory* scope_history,
										   std::vector<int>& curr_scope_context,
										   std::vector<int>& curr_node_context,
										   RunHelper& run_helper,
										   LoopFoldHistory* history);
	void clean_inner_scope_activate_helper(std::vector<double>& new_state_vals,
										   ScopeHistory* scope_history,
										   std::vector<int>& curr_scope_context,
										   std::vector<int>& curr_node_context,
										   RunHelper& run_helper,
										   int iter_index,
										   int step_index,
										   LoopFoldHistory* history,
										   std::vector<double>& test_new_state_vals,
										   std::vector<std::vector<std::vector<std::vector<StateNetworkHistory*>>>>& test_inner_state_network_histories);
	void clean_activate(Problem& problem,
						std::vector<double>& state_vals,
						std::vector<bool>& states_initialized,
						double& predicted_score,
						double& scale_factor,
						double& sum_impact,
						std::vector<ScopeHistory*>& context_histories,
						RunHelper& run_helper,
						LoopFoldHistory* history);
	void clean_backprop(std::vector<double>& state_errors,
						std::vector<bool>& states_initialized,
						double target_val,
						double final_misguess,
						double final_sum_impact,
						double& predicted_score,
						double& scale_factor,
						double& scale_factor_error,
						RunHelper& run_helper,
						LoopFoldHistory* history);

	void remove_inner_scope_end();
	void remove_inner_scope_network_end();

	void remove_inner_network_transform_helper();
	void remove_inner_network_end();

	void remove_outer_scope_from_load();
	void remove_outer_scope_network_from_load();
	void remove_inner_scope_from_load();
	void remove_inner_scope_network_from_load();
	void remove_inner_network_from_load();

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);
};

class LoopFoldHistory {
public:
	LoopFold* loop_fold;

	std::vector<std::vector<StateNetworkHistory*>> outer_state_network_histories;

	std::vector<StateNetworkHistory*> starting_state_network_histories;

	int num_loop_iters;

	std::vector<double> continue_score_network_updates;
	std::vector<StateNetworkHistory*> continue_score_network_histories;
	std::vector<double> continue_misguess_vals;
	std::vector<StateNetworkHistory*> continue_misguess_network_histories;
	double halt_score_network_update;
	StateNetworkHistory* halt_score_network_history;
	double halt_misguess_val;
	StateNetworkHistory* halt_misguess_network_history;

	std::vector<std::vector<std::vector<StateNetworkHistory*>>> state_network_histories;
	std::vector<std::vector<ScopeHistory*>> inner_scope_histories;
	std::vector<std::vector<double>> score_network_updates;
	std::vector<std::vector<StateNetworkHistory*>> score_network_histories;
	std::vector<std::vector<std::vector<std::vector<StateNetworkHistory*>>>> inner_state_network_histories;

	int state_iter_snapshot;	// heuristic to try to catch if state change occurred

	LoopFoldHistory(LoopFold* loop_fold);
	~LoopFoldHistory();
};

#endif /* LOOP_FOLD_H */