#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

class ScoreNetwork : public AbstractNetwork {
public:
	std::vector<Layer*> input;
	Layer* hidden;
	Layer* output;

	int epoch;
	int iter;

	std::mutex mtx;

	Network(std::vector<int> local_state);
	~Network();

	void activate(std::vector<std::vector<double>>& state_vals);
};

#endif /* SCORE_NETWORK_H */