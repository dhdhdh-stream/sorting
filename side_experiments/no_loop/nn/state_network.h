#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

class StateNetwork {
public:
	Layer* obs_input;

	Layer* state_input;

	Layer* hidden;
	Layer* output;

	bool can_be_end;
	double correlation_to_end;
	/**
	 * - used to calculate predicted_score before resolved if can't be ending
	 */

	std::set<StateNetwork*> preceding_networks;
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



	// TODO: if last_seen_network is NULL, then just have to do starting norm
	void activate(double obs_val,
				  StateStatus& state_status);
	// TODO: set both val and last network

};

#endif /* STATE_NETWORK_H */