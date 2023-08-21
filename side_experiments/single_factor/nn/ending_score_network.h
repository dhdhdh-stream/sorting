#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

#include "layer.h"

class ScoreNetwork {
public:
	Layer* input;
	Layer* output;

	int epoch_iter;
	double output_average_max_update;

	ScoreNetwork();
	~ScoreNetwork();

	void activate();
	void backprop(double target_max_update);
};

#endif /* SCORE_NETWORK_H */