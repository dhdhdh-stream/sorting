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
	int flat_size;
	int output_size;

	int state_size;

	Layer* flat_input;
	Layer* activated_input;
	Layer* state_input;
	Layer* obs_input;

	Layer* hidden;
	Layer* output;

	int epoch;
	int iter;

	std::mutex mtx;

	FoldNetwork(int flat_size,
				int output_size);
	FoldNetwork(int flat_size,
				int state_size,
				int output_size);
	~FoldNetwork();

	void activate(double* flat_inputs,
				  bool* activated,
				  std::vector<double>& obs);
	void activate(double* flat_inputs,
				  bool* activated,
				  std::vector<double>& obs,
				  std::vector<AbstractNetworkHistory*>& network_historys);

	void activate(double* flat_inputs,
				  bool* activated,
				  std::vector<double>& state_vals,
				  std::vector<double>& obs);
	void activate(double* flat_inputs,
				  bool* activated,
				  std::vector<double>& state_vals,
				  std::vector<double>& obs,
				  std::vector<AbstractNetworkHistory*>& network_historys);

	void full_backprop(std::vector<double>& errors);
	void full_calc_max_update(double& max_update,
							  double learning_rate,
							  double momentum);
	void full_update_weights(double factor,
							 double learning_rate,
							 double momentum);

	void set_state_size(int state_size);
	// void reset_state();

	void state_backprop(std::vector<double>& errors);
	void state_calc_max_update(double& max_update,
							   double learning_rate,
							   double momentum);
	void state_update_weights(double factor,
							  double learning_rate,
							  double momentum);
};

class FoldNetworkHistory : public AbstractNetworkHistory {
public:
	std::vector<double> flat_input_history;
	std::vector<double> activated_input_history;
	std::vector<double> state_input_history;
	std::vector<double> obs_input_history;

	std::vector<double> hidden_history;
	std::vector<double> output_history;

	FoldNetworkHistory(FoldNetwork* network);
	~FoldNetworkHistory();
	void reset_weights() override;
};

#endif /* FOLD_NETWORK_H */