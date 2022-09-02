#ifndef FOLD_NETWORK_H
#define FOLD_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "fold_helper.h"
#include "layer.h"
#include "node.h"

class FoldNetwork : public AbstractNetwork {
public:
	int existing_state_size;
	int flat_size;
	int new_state_size;
	int obs_size;

	std::vector<bool> input_on;

	Layer* existing_state_input;
	Layer* flat_input;
	Layer* activated_input;
	Layer* new_state_input;
	Layer* obs_input;

	Layer* hidden;
	Layer* output;

	int epoch;
	int iter;

	std::mutex mtx;

	FoldNetwork(int existing_state_size,
				int flat_size,
				int new_state_size);
	~FoldNetwork();

	void activate(std::vector<double>& existing_state_inputs,
				  double* flat_inputs,
				  bool* activated,
				  double* new_state_vals,
				  std::vector<double>& obs);

	void full_backprop(std::vector<double>& errors,
					   double* new_state_errors);
	void full_calc_max_update(double& max_update,
							  double learning_rate,
							  double momentum);
	void full_update_weights(double factor,
							 double learning_rate,
							 double momentum);

	void state_backprop(std::vector<double>& errors,
						double* new_state_errors);
	void state_calc_max_update(double& max_update,
							   double learning_rate,
							   double momentum);
	void state_update_weights(double factor,
							  double learning_rate,
							  double momentum);
};

class FoldNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<double> existing_state_input_history;
	std::vector<double> flat_input_history;
	std::vector<double> activated_input_history;
	std::vector<double> new_state_input_history;
	std::vector<double> obs_input_history;

	std::vector<double> hidden_history;
	std::vector<double> output_history;

	FoldNetworkHistory(FoldNetwork* network);
	~FoldNetworkHistory();
	void reset_weights() override;
};

#endif /* FOLD_NETWORK_H */