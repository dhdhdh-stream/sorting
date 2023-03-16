// TODO: zero train by tracking separate set of state, selecting 1 per run randomly forward

// TODO: for loops, because needs lots of 0, 1, 2, so only loop from existing?
// probably add like loop fold that can still add state
// state used to setup halt and continue networks

#ifndef FOLD_H
#define FOLD_H

#include <map>
#include <set>
#include <vector>

#include "run_helper.h"
#include "scope.h"
#include "state_network.h"

const int FOLD_STATE_EXPLORE = 0;	// with 1 new outer, 1 new local

const int FOLD_STATE_EXPLORE_FAIL = 1;

// TODO: consider adding input even if explore fail

const int FOLD_STATE_ADD_INNER_STATE = 2;
const int FOLD_STATE_ADD_OUTER_STATE = 3;

const int FOLD_STATE_EXPLORE_DONE = 4;

// freeze outer states scopes/nodes, use only if in outer_state_networks

const int FOLD_STATE_REMOVE_OUTER_SCOPE = 5;
// don't worry about removing outer states individually as will have no impact on scopes
const int FOLD_STATE_REMOVE_OUTER_NETWORK = 6;

const int FOLD_STATE_REMOVE_INNER_NETWORK = 7;
const int FOLD_STATE_REMOVE_INNER_STATE = 8;
const int FOLD_STATE_CLEAR_INNER_STATE = 9;

const int FOLD_STATE_DONE = 10;

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
	int num_score_local_states;
	int num_score_input_states;
	int num_sequence_local_states;
	int num_sequence_input_states;

	int sum_inner_inputs;
	std::vector<int> inner_input_start_indexes;
	std::vector<int> num_inner_inputs;	// keep track here fixed even if scope updates
	// TODO: inner and ending scale_mods

	int state;
	int state_iter;
	int sub_state_iter;
	double sum_error;

	int curr_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> curr_outer_state_networks;
	StateNetwork* curr_starting_score_network;
	// no starting state networks as ideally, state should already capture everything relevant

	int test_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_outer_state_networks;
	StateNetwork* test_starting_score_network;

	int curr_num_new_inner_states;	// in addition to sum_inner_inputs, starts at 1
	std::vector<std::vector<StateNetwork*>> curr_state_networks;
	std::vector<StateNetwork*> curr_score_networks;

	int test_num_new_inner_states;
	std::vector<std::vector<StateNetwork*>> test_state_networks;
	std::vector<StateNetwork*> test_score_networks;	// compare against curr_score_networks rather than score, as easier to measure

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

	int explore_result;

	int clean_outer_scope_index;
	std::set<int> curr_outer_scopes_needed;
	std::set<std::pair<int, int>> curr_outer_contexts_needed;
	std::set<int> reverse_test_outer_scopes_needed;
	std::set<std::pair<int, int>> reverse_test_outer_contexts_needed;

	int clean_outer_node_index;
	int clean_outer_state_index;
	std::map<int, std::vector<std::vector<bool>>> curr_outer_state_networks_not_needed;
	std::map<int, std::vector<std::vector<bool>>> test_outer_state_networks_not_needed;

	int clean_inner_step_index;
	int clean_inner_state_index;

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

	void explore_score_activate_helper(std::vector<double>& new_outer_state_vals,
									   ScopeHistory* scope_history,
									   std::vector<int>& curr_scope_context,
									   std::vector<int>& curr_node_context,
									   RunHelper& run_helper,
									   FoldHistory* history);
	void explore_score_activate(std::vector<double>& local_state_vals,
								std::vector<double>& input_vals,
								double& predicted_score,
								double& scale_factor,
								std::vector<ScopeHistory*>& context_histories,
								RunHelper& run_helper,
								FoldHistory* history);
	void explore_sequence_activate(std::vector<double>& local_state_vals,
								   std::vector<double>& input_vals,
								   std::vector<std::vector<double>>& flat_vals,
								   double& predicted_score,
								   double& scale_factor,
								   RunHelper& run_helper,
								   FoldHistory* history);
	void explore_backprop(std::vector<double>& local_state_errors,
						  std::vector<double>& input_errors,
						  double target_val,
						  double final_misguess,
						  double& predicted_score,
						  double& scale_factor,
						  RunHelper& run_helper,
						  FoldHistory* history);
	void explore_increment();

	void explore_end();
	void add_inner_state_end();
	void add_outer_state_end();

	void score_activate(std::vector<double>& local_state_vals,
						std::vector<double>& input_vals,
						std::vector<ScopeHistory*>& context_histories,
						RunHelper& run_helper,
						FoldHistory* history);
	void sequence_activate(std::vector<double>& local_state_vals,
						   std::vector<double>& input_vals,
						   std::vector<std::vector<double>>& flat_vals,
						   double& predicted_score,
						   double& scale_factor,
						   double& sum_impact,
						   RunHelper& run_helper,
						   FoldHistory* history);
	void backprop(std::vector<double>& local_state_errors,
				  std::vector<double>& input_errors,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  FoldHistory* history);
	void increment();

	void add_to_clean();

	void remove_outer_scope_score_activate_helper(std::vector<double>& new_outer_state_vals,
												  std::vector<double>& test_new_outer_state_vals,
												  ScopeHistory* scope_history,
												  std::vector<int>& curr_scope_context,
												  std::vector<int>& curr_node_context,
												  RunHelper& run_helper,
												  FoldHistory* history);
	void remove_outer_scope_score_activate(std::vector<double>& local_state_vals,
										   std::vector<double>& input_vals,
										   std::vector<ScopeHistory*>& context_histories,
										   RunHelper& run_helper,
										   FoldHistory* history);
	void remove_outer_scope_sequence_activate(std::vector<double>& local_state_vals,
											  std::vector<double>& input_vals,
											  std::vector<std::vector<double>>& flat_vals,
											  double& predicted_score,
											  double& scale_factor,
											  double& sum_impact,
											  RunHelper& run_helper,
											  FoldHistory* history);

	void remove_outer_scope_end();

	void remove_outer_network_score_activate_helper(std::vector<double>& new_outer_state_vals,
													std::vector<double>& test_new_outer_state_vals,
													ScopeHistory* scope_history,
													std::vector<int>& curr_scope_context,
													std::vector<int>& curr_node_context,
													RunHelper& run_helper,
													FoldHistory* history);
	void remove_outer_network_score_activate(std::vector<double>& local_state_vals,
											 std::vector<double>& input_vals,
											 std::vector<ScopeHistory*>& context_histories,
											 RunHelper& run_helper,
											 FoldHistory* history);
	void remove_outer_network_sequence_activate(std::vector<double>& local_state_vals,
												std::vector<double>& input_vals,
												std::vector<std::vector<double>>& flat_vals,
												double& predicted_score,
												double& scale_factor,
												double& sum_impact,
												RunHelper& run_helper,
												FoldHistory* history);

	void remove_outer_network_end();

	void clean_score_activate_helper(std::vector<double>& new_outer_state_vals,
									 ScopeHistory* scope_history,
									 std::vector<int>& curr_scope_context,
									 std::vector<int>& curr_node_context,
									 RunHelper& run_helper,
									 FoldHistory* history);
	void clean_score_activate(std::vector<double>& local_state_vals,
							  std::vector<double>& input_vals,
							  std::vector<ScopeHistory*> context_histories,
							  RunHelper& run_helper,
							  FoldHistory* history);
	void clean_sequence_activate(std::vector<double>& local_state_vals,
								 std::vector<double>& input_vals,
								 std::vector<std::vector<double>>& flat_vals,
								 double& predicted_score,
								 double& scale_factor,
								 double& sum_impact,
								 RunHelper& run_helper,
								 FoldHistory* history);

	void remove_inner_network_end();
	void remove_inner_state_end();
	void clear_inner_state_end();

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);

	void remove_outer_scope_from_load();
	void remove_outer_network_from_load();
	void remove_inner_network_from_load();
	void remove_inner_state_from_load();
	void clear_inner_state_from_load();
};

class FoldHistory {
public:
	Fold* fold;

	// to help track between score and sequence
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

	int state_iter_snapshot;	// heuristic to try to catch if state change occurred

	FoldHistory(Fold* fold);
	~FoldHistory();
};

#endif /* FOLD_H */