#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

class ScoreNetwork {
public:
	int state_size;
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

class ScoreNetworkHistory {
public:
	ScoreNetwork* network;

	std::vector<double> hidden_history;

	
};

#endif /* SCORE_NETWORK_H */