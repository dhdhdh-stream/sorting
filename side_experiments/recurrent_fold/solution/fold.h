// TODO: even if can't compress, can still split by testing if clearing to 0 works

// TODO: while training, add like a 5% probability for input to be 0.0
// pass separate set of state and store separate set of history
// zero 1 thing per run, and randomly select forward, and if later selected, clear previous

// TODO: input extends into outer scope as well
// if fold is good enough, keep trying to add input
// if fold isn't good enough, see if input is unique, and if not, add it

#ifndef FOLD_H
#define FOLD_H

#include "network.h"

const int STATE_REMOVE_NETWORK = 0;
const int STATE_REMOVE_STATE = 1;
const int STATE_CLEAR_STATE = 2;

const int STATE_DONE = 3;

class Fold {
public:
	int sequence_length;

	int curr_num_states;
	std::vector<std::vector<Network*>> curr_state_networks;
	std::vector<Network*> curr_score_networks;	// TODO: worry about trimming inputs only when constructing scope

	int test_num_states;
	std::vector<std::vector<Network*>> test_state_networks;
	std::vector<Network*> test_score_networks;	// compare against curr_score_networks rather than score, as easier to measure

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

	void activate(std::vector<std::vector<double>>& flat_vals);
	void backprop(double target_val);

	void add_state();
};

#endif /* FOLD_H */