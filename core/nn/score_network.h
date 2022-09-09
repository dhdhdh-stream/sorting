#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

class ScoreNetwork : public AbstractNetwork {
public:
	std::vector<Layer*> state_inputs;
	Layer* obs_input;

	Layer* hidden;
	Layer* output;

	int epoch;
	int iter;

	std::mutex mtx;

	Network(std::vector<int> local_state_sizes);
	~Network();

	void insert_scope(int layer,
					  int new_state_size);

	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<double>& obs,
				  std::vector<AbstractNetworkHistory*>& network_historys);
};

#endif /* SCORE_NETWORK_H */