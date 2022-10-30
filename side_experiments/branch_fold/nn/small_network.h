#ifndef SMALL_NETWORK_H
#define SMALL_NETWORK_H

#include <fstream>
#include <mutex>
#include <vector>

#include "abstract_network.h"
#include "layer.h"

class SmallNetwork : public AbstractNetwork {
public:
	int state_input_size;
	int s_input_input_size;
	int hidden_size;
	int output_size;

	Layer* state_input;
	Layer* s_input_input;
	
	Layer* hidden;
	Layer* output;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	std::mutex mtx;

	SmallNetwork(int state_input_size,
				 int s_input_input_size,
				 int hidden_size,
				 int output_size);
	SmallNetwork(SmallNetwork* original);
	SmallNetwork(std::ifstream& input_file);
	~SmallNetwork();

	void activate(std::vector<double>& state_vals,
				  std::vector<double>& s_input_vals);
	void backprop(std::vector<double>& errors,
				  double target_max_update);

	void backprop_errors_with_no_weight_change(std::vector<double>& errors);
	void backprop_weights_with_no_error_signal(std::vector<double>& errors,
											   double target_max_update);

	void save(std::ofstream& output_file);

private:
	void construct();
};

#endif /* SMALL_NETWORK_H */