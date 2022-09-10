#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

class ScoreNetwork : public AbstractNetwork {
public:
	std::vector<int> local_state_sizes;

	std::vector<Layer*> state_inputs;
	Layer* obs_input;

	Layer* hidden;
	Layer* output;

	int epoch;
	int iter;

	std::mutex mtx;

	Network(std::vector<int> local_state_sizes);
	Network(std::ifstream& input_file);
	~Network();

	void insert_scope(int layer,
					  int new_state_size);

	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<double>& obs,
				  std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop(std::vector<double>& errors);
	void calc_max_update(double& max_update,
						 double learning_rate,
						 double momentum);
	void update_weights(double factor,
						double learning_rate,
						double momentum);

	void save(std::ofstream& output_file);
};

class ScoreNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<std::vector<double>> state_inputs_history;
	std::vector<double> obs_input_history;

	std::vector<double> hidden_history;
	std::vector<double> output_history;

	ScoreNetworkHistory(ScoreNetwork* network);
	void reset_weights() override;
};

#endif /* SCORE_NETWORK_H */