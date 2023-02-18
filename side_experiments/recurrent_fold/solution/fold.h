// TODO: while training, add like a 5% probability for input to be 0.0
// pass separate set of state and store separate set of history
// zero 1 thing per run, and randomly select forward, and if later selected, clear previous

// TODO: input extends into outer scope as well
// if fold is good enough, keep trying to add input
// if fold isn't good enough, see if input is unique, and if not, add it

#ifndef FOLD_H
#define FOLD_H

#include "network.h"

const int STATE_FLAT = 0;	// with 1 state

const int STATE_FLAT_DONE = 1;

const int STATE_FOLD_ADD_STATE = 2;

const int STATE_FOLD_REMOVE_NETWORK = 3;
const int STATE_FOLD_REMOVE_STATE = 4;
const int STATE_FOLD_CLEAR_STATE = 5;

const int STATE_FOLD_DONE = 6;

class Fold {
public:
	int sequence_length;

	int curr_num_states;
	std::vector<std::vector<Network*>> curr_state_networks;
	std::vector<Network*> curr_score_networks;	// TODO: worry about trimming inputs only when constructing scope

	double curr_average_misguess;
	double curr_misguess_variance;

	int test_num_states;
	std::vector<std::vector<Network*>> test_state_networks;
	std::vector<Network*> test_score_networks;	// compare against curr_score_networks rather than score, as easier to measure

	double test_average_misguess;
	double test_misguess_variance;

	int state;
	int state_iter;
	double sum_error;

	int fold_step_index;
	int fold_state_index;

	std::vector<std::vector<bool>> curr_state_networks_not_needed;
	std::vector<std::vector<bool>> test_state_networks_not_needed;

	std::vector<std::vector<bool>> curr_state_not_needed_locally;
	std::vector<std::vector<bool>> test_state_not_needed_locally;

	std::vector<int> curr_num_states_cleared;
	std::vector<int> test_num_states_cleared;

	Fold(int sequence_length);
	~Fold();

	void flat_activate(std::vector<std::vector<double>>& flat_vals,
					   double& predicted_score);
	void flat_backprop(double target_val,
					   double final_misguess,
					   double& predicted_score);
	void flat_increment();

	void flat_to_fold();

	void fold_activate(std::vector<std::vector<double>>& flat_vals,
					   double& predicted_score);
	void fold_backprop(double target_val,
					   double final_misguess,
					   double& predicted_score);

	void fold_add_activate(std::vector<std::vector<double>>& flat_vals,
						   double& predicted_score);
	void fold_add_backprop(double target_val,
						   double final_misguess,
						   double& predicted_score);
	void fold_add_increment();

	void fold_clean_activate(std::vector<std::vector<double>>& flat_vals,
							 double& predicted_score);
	void fold_clean_backprop(double target_val,
							 double final_misguess,
							 double& predicted_score);
	void fold_clean_increment();
};

#endif /* FOLD_H */