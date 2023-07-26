#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_LOOP = 1;

const double DEFAULT_LASSO_WEIGHT = 0.2;

const int NUM_NEW_STATES = 10;
const std::vector<double> DEFAULT_NEW_STATE_LASSO_WEIGHTS{
	0.4, 0.8, 1.2, 1.6, 2.0, 2.4, 2.8, 3.2, 3.6, 4.0};

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

	std::vector<int> new_state_furthest_layer_needed_in;
	std::vector<int> layer_num_new_states;

	/**
	 * - scope_additions_needed doesn't include all scopes that need to be modified
	 *   - check scope_node_additions_needed as well
	 */
	std::vector<std::set<int>> scope_additions_needed;
	std::vector<std::set<std::pair<int, int>>> scope_node_additions_needed;

	/**
	 * - calculate against pre-experiment values
	 *   - (as not all state is updated post-experiment)
	 * 
	 * - if new state not at depth, then ignore
	 */
	std::vector<int> corr_calc_scope_depths;
	std::vector<int> corr_calc_input_indexes;
	std::vector<double> corr_calc_average_vals;
	std::vector<double> corr_calc_variances;
	std::vector<std::vector<double>> corr_calc_new_average_vals;
	std::vector<std::vector<double>> corr_calc_new_variances;
	std::vector<std::vector<double>> corr_calc_covariances;
	std::vector<std::vector<Transformation*>> new_transformations;

	int new_num_states;
	std::vector<bool> new_states_initialized;
	std::vector<FamilyDefinition*> new_state_families;
	std::vector<ClassDefinition*> new_default_state_classes;
	/**
	 * - includes both new states and new inputs
	 */
	std::vector<int> last_layer_new_indexes;
	std::vector<int> last_layer_new_target_indexes;
	std::vector<Transformation*> last_layer_new_transformations;

};

#endif /* ABSTRACT_EXPERIMENT_H */