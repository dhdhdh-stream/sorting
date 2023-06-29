#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_LOOP = 1;

const int EXPERIMENT_STATE_EXPLORE = -1;
const int EXPERIMENT_STATE_EXPERIMENT = 0;

const int EXPERIMENT_STATE_NEW_CLASSES = 1;	// for branch
const int EXPERIMENT_STATE_MEASURE = 1;		// for loop

const int EXPERIMENT_STATE_CLEANUP = 2;

const int NUM_NEW_STATES = 8;

class AbstractExperiment {
public:
	int type;

	std::vector<int> scope_context;
	std::vector<int> node_context;

	int state;
	int state_iter;
	double sum_error;

	/**
	 * - use same networks for each layer, but use different lasso weights per layer
	 *   - if a network is used in multiple layers, OK to let it use any of the lasso weights
	 */
	std::map<int, std::vector<std::vector<StateNetwork*>>> state_networks;
	// temporary to determine state needed
	std::map<int, std::vector<ScoreNetwork*>> score_networks;

	/**
	 * - index 0 is global
	 * - contexts in the middle
	 * - last index is inner
	 */
	std::map<int, int> scope_furthest_layer_seen_in;

	std::vector<int> new_state_furthest_layer_seen_in;
	std::vector<int> layer_num_new_states;

};

#endif /* ABSTRACT_EXPERIMENT_H */