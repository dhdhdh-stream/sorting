#ifndef ACTION_NETWORK_H
#define ACTION_NETWORK_H

class ActionNetwork {
public:
	Layer* obs_input;	// size always 1 for sorting

	Layer* state_input;

	int hidden_size;
	Layer* hidden_input;

	Layer* output;

	double lasso_weight;
	double impact;



	void input_activate();

	void new_activate();

};

#endif /* STATE_NETWORK_H */