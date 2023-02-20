// TODO: while training, add like a 5% probability for input to be 0.0
// pass separate set of state and store separate set of history
// zero 1 thing per run, and randomly select forward, and if later selected, clear previous

// TODO: input extends into outer scope as well
// if fold is good enough, keep trying to add input
// if fold isn't good enough, see if input is unique, and if not, add it

#ifndef FOLD_H
#define FOLD_H

#include "network.h"

const int STATE_EXPLORE = 0;	// with 1 state

const int STATE_EXPLORE_DONE = 1;

const int STATE_ADD_STATE = 2;	// don't add to solution yet because still changing

const int STATE_ADD_DONE = 3;

const int STATE_REMOVE_NETWORK = 4;
const int STATE_REMOVE_STATE = 5;
const int STATE_CLEAR_STATE = 6;

const int STATE_DONE = 7;

class Fold {
public:
	int num_input_states;

	int sequence_length;

	Network* starting_score_network;
	Network* combined_score_network;
	Network* end_scale_mod;

	int curr_num_states;
	std::vector<std::vector<Network*>> curr_state_networks;
	std::vector<Network*> curr_score_networks;	// TODO: worry about trimming inputs only when constructing scope

	double curr_average_misguess;
	double curr_misguess_variance;

	std::vector<double> curr_step_impacts;

	int test_num_states;
	std::vector<std::vector<Network*>> test_state_networks;
	std::vector<Network*> test_score_networks;	// compare against curr_score_networks rather than score, as easier to measure

	double test_average_misguess;
	double test_misguess_variance;

	std::vector<double> test_step_impacts;

	int state;
	int state_iter;
	double sum_error;

	int clean_step_index;
	int clean_state_index;

	// TODO: remove back to front? Or might not matter because trimming makes the order not matter?
	// intuitively, removing back to front matches the idea of scopes more, so go with that
	std::vector<std::vector<bool>> curr_state_networks_not_needed;
	std::vector<std::vector<bool>> test_state_networks_not_needed;

	// TODO: remove back to front?
	std::vector<std::vector<bool>> curr_state_not_needed_locally;
	std::vector<std::vector<bool>> test_state_not_needed_locally;

	std::vector<int> curr_num_states_cleared;
	std::vector<int> test_num_states_cleared;

	Fold(int sequence_length);
	~Fold();

	void explore_activate(std::vector<std::vector<double>>& flat_vals,
						  double& predicted_score);
	void explore_backprop(double target_val,
						  double final_misguess,
						  double& predicted_score);
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

#endif /* FOLD_H */