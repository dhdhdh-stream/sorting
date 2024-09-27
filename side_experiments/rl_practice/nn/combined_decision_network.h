/**
 * - not good to combine outputs
 *   - especially because some outputs will initially be sparse
 *     - initial subtle signals will be washed away by the constant changes from the majority updates
 */

#ifndef COMBINED_DECISION_NETWORK_H
#define COMBINED_DECISION_NETWORK_H

#include <vector>

#include "layer.h"

class CombinedDecisionNetwork {
public:
	Layer* obs_input;
	Layer* state_input;

	Layer* hidden_1;
	Layer* hidden_2;
	Layer* output;

	int epoch_iter;
	double hidden_1_average_max_update;
	double hidden_2_average_max_update;
	double output_average_max_update;

	CombinedDecisionNetwork(int num_obs,
							int num_actions,
							int num_state);
	CombinedDecisionNetwork(std::ifstream& input_file);
	~CombinedDecisionNetwork();

	void activate(std::vector<double>& obs_vals,
				  std::vector<double>& state_vals,
				  int& action);
	void backprop(std::vector<double>& obs_vals,
				  int action,
				  std::vector<double>& state_vals,
				  double eval);

	void save(std::ofstream& output_file);
};

#endif /* COMBINED_DECISION_NETWORK_H */