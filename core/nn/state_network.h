#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

#include <fstream>
#include <set>

#include "layer.h"
#include "state_status.h"

class State;

class StateNetwork {
public:
	Layer* obs_input;
	Layer* state_input;

	Layer* hidden;
	Layer* output;

	State* parent_state;
	int index;
	std::set<int> preceding_network_indexes;
	/**
	 * - if last state network is not among preceding networks, then normalize
	 */

	double starting_mean;
	double starting_variance;
	double starting_standard_deviation;

	/**
	 * - averaged from following networks' starting mean/variance
	 *   - not actual ending mean/variance
	 */
	double ending_mean;
	double ending_variance;
	double ending_standard_deviation;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

	StateNetwork(int index);
	StateNetwork(std::ifstream& input_file,
				 State* parent_state,
				 int index);
	~StateNetwork();

	void activate(double obs_val,
				  double& state_val);
	void backprop(double& state_error);

	void activate(double obs_val,
				  StateStatus& state_status);

	void save(std::ofstream& output_file);

private:
	void construct();
};

#endif /* STATE_NETWORK_H */