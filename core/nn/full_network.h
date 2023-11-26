#ifndef FULL_NETWORK_H
#define FULL_NETWORK_H

#include <set>

class ForgetNetwork;
class ForgetNetworkHistory;
class IndexNetwork;
class IndexNetworkHistory;
class State;
class StateNetwork;
class StateNetworkHistory;
class StateStatus;
class UpdateSizeNetwork;
class UpdateSizeNetworkHistory;

class FullNetworkHistory;
class FullNetwork {
public:
	UpdateSizeNetwork* update_size_network;
	StateNetwork* state_network;
	ForgetNetwork* forget_network;
	IndexNetwork* index_network;

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

	/**
	 * - index * update_size / sum_update_size
	 */
	double average_update_size;
	double average_index;

	FullNetwork(int index);
	FullNetwork(std::ifstream& input_file,
				State* parent_state,
				int index);
	~FullNetwork();

	void activate(double obs_val,
				  double& state_val,
				  double& index_val,
				  FullNetworkHistory* history);
	void backprop(double& state_error,
				  double& index_error,
				  FullNetworkHistory* history);

	void activate(double obs_val,
				  double& state_val,
				  double& index_val);

	void activate(double obs_val,
				  StateStatus& state_status);

	void save(std::ofstream& output_file);
};

class FullNetworkHistory {
public:
	FullNetwork* network;

	UpdateSizeNetworkHistory* update_size_network_history;
	StateNetworkHistory* state_network_history;
	ForgetNetworkHistory* forget_network_history;
	IndexNetworkHistory* index_network_history;

	double starting_index_val_snapshot;

	double update_size_snapshot;
	double state_update_snapshot;

	FullNetworkHistory(FullNetwork* network);
	~FullNetworkHistory();
};

#endif /* FULL_NETWORK_H */