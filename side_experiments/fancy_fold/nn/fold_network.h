#ifndef FOLD_NETWORK_H
#define FOLD_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class FoldNetwork : public AbstractNetwork {
public:
	int flat_size;
	int output_size;

	int state_size;

	Layer* flat_input;
	Layer* activated_input;
	Layer* obs_input;
	Layer* state_input;

	int next_state_size;
	Layer* next_state_input;

	Layer* hidden;
	Layer* output;

	int epoch_iter;

	std::mutex mtx;

	FoldNetwork(int flat_size,
				int output_size);
	FoldNetwork(int flat_size,
				int output_size,
				std::ifstream& input_file);
	FoldNetwork(FoldNetwork* original);
	~FoldNetwork();

	void activate(double* flat_inputs,
				  bool* activated,
				  std::vector<double>& obs);
	void activate(double* flat_inputs,
				  bool* activated,
				  std::vector<double>& obs,
				  std::vector<AbstractNetworkHistory*>& network_historys);

	void backprop(std::vector<double>& erors);
	void calc_max_update(double& max_update,
						 double learning_rate);
	void update_weights(double factor,
						double learning_rate);

	void set_next_state_size(int next_state_size);
	void activate_compare_hidden_and_backprop(
		double* flat_inputs,
		bool* activated,
		int fold_index,
		std::vector<double>& obs,
		std::vector<double>& state_vals,
		std::vector<double>& next_state_vals);
	void next_calc_max_update(double& max_update,
							  double learning_rate);
	void next_update_weights(double factor,
							 double learning_rate);
	void activate_full(double* flat_inputs,
					   bool* activated,
					   int fold_index,
					   std::vector<double>& obs,
					   std::vector<double>& next_state_vals);
	void activate_full(double* flat_inputs,
					   bool* activated,
					   int fold_index,
					   std::vector<double>& obs,
					   std::vector<double>& next_state_vals,
					   std::vector<AbstractNetworkHistory*>& network_historys);
	void take_next();
	void discard_next();

	void activate_curr(double* flat_inputs,
					   bool* activated,
					   int fold_index,
					   std::vector<double>& obs,
					   std::vector<double>& state_vals);

	void save(std::ofstream& output_file);

private:
	void construct();
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
	void reset_weights() override;
};

#endif /* FOLD_NETWORK_H */