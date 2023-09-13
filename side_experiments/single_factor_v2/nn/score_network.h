#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

class ScoreNetwork {
public:
	std::vector<int> state_indexes;
	Layer* state_input;

	Layer* output;

	int epoch_iter;
	double output_average_max_update;

	
};

#endif /* SCORE_NETWORK_H */