#ifndef EXIT_NETWORK_H
#define EXIT_NETWORK_H

class ExitNetwork {
public:
	std::vector<int> context_indexes;
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

	ExitNetwork(std::vector<int>& exit_context);

	void activate(std::vector<std::vector<double>>& state_vals);
};

class ExitNetworkHistory {
public:
	ExitNetwork* network;

	std::vector<double> hidden_history;
	
};

#endif /* EXIT_NETWORK_H */