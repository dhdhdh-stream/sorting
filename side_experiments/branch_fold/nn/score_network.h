#ifndef SCORE_NETWORK_H
#define SCORE_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class ScoreNetwork : public AbstractNetwork {
public:
	std::vector<int> scope_sizes;

	std::vector<Layer*> state_inputs;

	int fold_index;
	double average_error;

	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	ScoreNetwork(std::vector<int> scope_sizes);
	ScoreNetwork(std::ifstream& input_file);
	ScoreNetwork(ScoreNetwork* original);
	~ScoreNetwork();

	void add_state(int layer);
	void activate(std::vector<std::vector<double>>& state_vals);
	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<AbstractNetworkHistory*>& network_historys);
	void backprop(double target_val,
				  double target_max_update);
	void backprop_weights_with_no_error_signal(double target_val,
											   double target_max_update);

	void save(std::ofstream& output_file);

private:
	void construct();
};

class ScoreNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<std::vector<double>> state_inputs_historys;

	std::vector<double> hidden_history;
	std::vector<double> output_history;

	ScoreNetworkHistory(ScoreNetwork* network);
	void reset_weights() override;
};

#endif /* SCORE_NETWORK_H */