// TODO: zero train by tracking separate set of state, selecting 1 per run randomly forward

// TODO: for loops, because needs lots of 0, 1, 2, so only loop from existing?
// probably add like loop fold that can still add state
// state used to setup halt and continue networks

#ifndef FOLD_H
#define FOLD_H

#include "network.h"

const int STATE_EXPLORE = 0;	// with 1 new outer, 1 new local

const int STATE_EXPLORE_DONE = 1;

// TODO: consider adding input even if sequence isn't good enough

// freeze outer states scopes/nodes, use only if in outer_state_networks

const int STATE_ADD_INNER_STATE = 2;
const int STATE_ADD_OUTER_STATE = 3;

const int STATE_ADD_DONE = 4;

const int STATE_REMOVE_OUTER_SCOPE = 5;
// don't worry about removing outer states individually as will have no impact on scopes anyways
const int STATE_REMOVE_OUTER_NETWORK = 6;

// TODO: remove front-to-back
const int STATE_REMOVE_INNER_NETWORK = 7;
const int STATE_REMOVE_INNER_STATE = 8;
const int STATE_CLEAR_INNER_STATE = 9;

const int STATE_DONE = 10;

class Fold {
public:
	vector<int> scope_context;
	vector<int> node_context;

	int curr_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> curr_outer_state_networks;
	StateNetwork* curr_starting_score_network;

	int test_num_new_outer_states;
	std::map<int, std::vector<std::vector<StateNetwork*>>> test_outer_state_networks;
	StateNetwork* test_starting_score_network;

	int clean_outer_scope_index;
	std::set<int> outer_scopes_needed;
	std::map<int, bool> outer_scopes_checked;	// true if needed

	int clean_outer_node_index;
	int clean outer_state_index;
	std::map<int, std::vector<std::vector<bool>>> curr_outer_state_networks_not_needed;
	std::map<int, std::vector<std::vector<bool>>> test_outer_state_networks_not_needed;

	// keep fixed even if parent scope updates
	int num_local_states;
	int num_input_states;

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;

	int sum_inner_inputs;
	std::vector<int> inner_input_start_indexes;
	std::vector<int> num_inner_inputs;	// keep track here fixed even as scope updates
	// TODO: inner and ending scale_mods

	int curr_num_new_inner_states;	// in addition to sum_inner_inputs, starts at 1
	std::vector<std::vector<StateNetwork*>> curr_state_networks;
	std::vector<StateNetwork*> curr_score_networks;

	double curr_average_score;
	double curr_score_variance;
	double curr_average_misguess;
	double curr_misguess_variance;

	std::vector<double> curr_step_impacts;

	int test_num_new_inner_states;
	std::vector<std::vector<StateNetwork*>> test_state_networks;
	std::vector<StateNetwork*> test_score_networks;	// compare against curr_score_networks rather than score, as easier to measure

	double test_average_score;
	double test_score_variance;
	double test_average_misguess;
	double test_misguess_variance;

	// TODO: compare against existing score/variance as well

	std::vector<double> test_step_impacts;

	int state;
	int state_iter;
	double sum_error;

	int clean_inner_step_index;
	int clean_inner_state_index;

	std::vector<std::vector<bool>> curr_state_networks_not_needed;
	std::vector<std::vector<bool>> test_state_networks_not_needed;

	std::vector<std::vector<bool>> curr_state_not_needed_locally;
	std::vector<std::vector<bool>> test_state_not_needed_locally;

	std::vector<int> curr_num_states_cleared;
	std::vector<int> test_num_states_cleared;

	Fold(int num_input_states,
		 int sequence_length);
	~Fold();

	void explore_score_activate(std::vector<double>& local_state_vals,
								std::vector<double>& input_vals,
								double& predicted_score,
								double& scale_factor,
								std::vector<int>& context_iter,
								std::vector<ContextHistory*> context_histories;
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
						  FoldHistory* history);
	void explore_increment();

	void explore_to_add();

	void add_activate(std::vector<std::vector<double>>& flat_vals,
					  double& predicted_score);
	void add_backprop(double target_val,
					  double final_misguess,
					  double& predicted_score);
	void add_increment();

	void add_to_clean();

	void clean_activate(std::vector<std::vector<double>>& flat_vals,
						double& predicted_score);
	void clean_backprop(double target_val,
						double final_misguess,
						double& predicted_score);
	void clean_increment();
};

class FoldHistory {
public:
	Fold* fold;

	// to help track between score and sequence
	std::vector<double> new_outer_state_vals;

	std::vector<std::vector<StateNetworkHistory*>> outer_state_network_histories;
	double starting_score_update;
	StateNetworkHistory* starting_score_network_history;

	std::vector<std::vector<StateNetworkHistory*>> state_network_histories;
	std::vector<ScopeHistory*> inner_scope_histories;
	std::vector<double> score_network_updates;
	std::vector<StateNetworkHistory*> score_network_histories;
};

#endif /* FOLD_H */