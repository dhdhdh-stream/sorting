// TODO: always mod by mean and variance after trained
// TODO: if scale negative sign, then make standard deviation negative?
// - also if negatively correlated to end?
//   - not possible if multi-input/output, which should assume will always end up being the case

// TODO: or just don't have scale initially (i.e., scale always 1.0)
// - then let scale drift after state trained when readjusting

#ifndef STATE_NETWORK_H
#define STATE_NETWORK_H

class StateNetwork {
public:
	Layer* obs_input;

	Layer* state_input;

	Layer* hidden;
	Layer* output;

	/**
	 * - use to modify predicted_score before resolved
	 *   - "resolve" is after containing scope exits
	 * 
	 * - can be 0 even for last network if XOR within loop
	 */
	double correlation_to_end;

	double starting_state_mean;
	double starting_state_variance;
	double starting_state_standard_deviation;

	double ending_state_mean;
	double ending_state_variance;
	double ending_state_standard_deviation;

	int epoch_iter;
	double hidden_average_max_update;
	double output_average_max_update;

};

#endif /* STATE_NETWORK_H */