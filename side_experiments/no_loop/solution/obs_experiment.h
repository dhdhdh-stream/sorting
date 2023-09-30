/**
 * TODO: instead of running on new problem instances every time, run on a small number of seeds
 */

#ifndef OBS_EXPERIMENT_H
#define OBS_EXPERIMENT_H

const int OBS_EXPERIMENT_STATE_FLAT = 0;
const int OBS_EXPERIMENT_STATE_RNN = 1;

class ObsExperiment {
public:
	int state;
	int state_iter;

	std::vector<AbstractNode*> nodes;
	std::vector<std::vector<int>> scope_contexts;
	std::vector<std::vector<int>> node_contexts;
	std::vector<int> obs_indexes;

	FlatNetwork* flat_network;

	std::vector<StateNetwork*> state_networks;
	/**
	 * - don't have ending scale while training (i.e., scale always 1.0)
	 *   - add scale after state trained and readjusting impact
	 */
	double resolved_variance;

	double new_average_misguess;

};

#endif /* OBS_EXPERIMENT_H */