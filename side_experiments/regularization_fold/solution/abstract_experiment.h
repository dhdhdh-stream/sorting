#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_LOOP = 1;

const int EXPERIMENT_STATE_EXPLORE = -1;
const int EXPERIMENT_STATE_EXPERIMENT = 0;

const int EXPERIMENT_STATE_NEW_CLASSES = 1;	// for branch
const int EXPERIMENT_STATE_MEASURE = 1;		// for loop

/**
 * - add new state and update score networks in a controlled way
 *   - only activate experiment depending on score
 *     - but mark that experiment seen for run
 * 
 * - gradually scale down temp score networks during first half
 *   - then let permanent score networks settle during second half
 * 
 * - also calculate correlation between new classes and existing classes
 */
const int EXPERIMENT_STATE_CLEANUP = 2;
const int EXPERIMENT_STATE_DONE = 3;

const double DEFAULT_LASSO_WEIGHT = 0.2;

const int NUM_NEW_STATES = 8;
const std::vector<double> DEFAULT_NEW_STATE_LASSO_WEIGHTS{
	0.4, 0.8, 1.2, 1.6, 2.0, 2.4, 2.8, 3.2};

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
	int final_num_new_states;
	std::vector<int> layer_num_new_states;
	std::vector<int> last_layer_new_state_indexes;

};

#endif /* ABSTRACT_EXPERIMENT_H */