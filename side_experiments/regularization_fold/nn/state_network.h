#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

class StateNetwork {
public:
	Layer* obs_input;	// size always 1 for sorting

	std::vector<int> state_indexes;
	Layer* state_input;

	int new_state_size;
	Layer* new_state_input;

	int hidden_size;
	Layer* hidden;

	Layer* output;	// size always 1

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;


};

class StateNetworkHistory {
public:
	StateNetworkHistory* history;

	std::vector<double> hidden_history;

	
};

#endif /* STATE_NETWORK_H */