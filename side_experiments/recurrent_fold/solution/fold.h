// TODO: even if can't compress, can still split by testing if clearing to 0 works

#ifndef FOLD_H
#define FOLD_H

#include "network.h"

class Fold {
public:
	int sequence_length;
	// TODO: for inner scopes, don't deep learn initially

	Network* outer_network;

	int num_states;
	std::vector<int> states_updated;
	std::vector<std::vector<Network*>> state_networks;

	double sum_error;

	Fold(int sequence_length);
	~Fold();

	void activate(std::vector<std::vector<double>>& flat_vals);
	void backprop(double target_val);

	void add_state();
};

#endif /* FOLD_H */