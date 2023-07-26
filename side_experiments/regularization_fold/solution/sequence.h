#ifndef SEQUENCE_H
#define SEQUENCE_H

const int SEQUENCE_INPUT_INIT_NONE = 0;
const int SEQUENCE_INPUT_INIT_LOCAL = 1;
const int SEQUENCE_INPUT_INIT_LAST_SEEN = 2;
// if empty (i.e., not initialize), don't include in input_init_types, etc.

class Sequence {
public:
	BranchExperiment* experiment;
	int step_index;

	/**
	 * - non-loop
	 */
	std::vector<Scope*> scopes;

	/**
	 * - starting_node_ids[0] == node_ids[0][0]
	 * 
	 * - last node id can be any type
	 *   - others all have to be non-loop scope nodes
	 */
	std::vector<int> starting_node_ids;

	std::vector<int> input_init_types;
	/**
	 * - if it's the same input in multiple layers, set at top layer, and have it cascade down
	 *   - append with new state by BRANCH_EXPERIMENT_STATE_WRAPUP
	 */
	std::vector<int> input_init_target_layers;
	std::vector<int> input_init_target_indexes;
	/**
	 * - needs to be state that isn't passed down further
	 * 
	 * - can be reused between sequences, with each re-setting and re-fetching
	 * 
	 * - negative indexing from end
	 *   - original scope is 0
	 *     - (experiment context not added yet)
	 * 
	 * - no longer needed by BRANCH_EXPERIMENT_STATE_WRAPUP
	 */
	std::vector<int> input_init_local_scope_depths;
	std::vector<int> input_init_local_input_indexes;
	std::vector<ClassDefinition*> input_init_last_seen_classes;
	std::vector<Transformation*> input_transformations;

	std::vector<std::vector<int>> node_ids;
	/**
	 * - scope nodes always given empty context
	 *   - so will always result in reasonable sequence
	 *     - (though may not be optimal due to original scope's later explored early exits)
	 *   - even if from halfway activate
	 * 
	 * - don't include branch nodes (and won't have exit nodes)
	 *   - also on success, create new scopes rather than reuse original scopes
	 *     - but inner scopes will be reused and generalized
	 */

	std::map<int, std::vector<std::vector<StateNetwork*>>> state_networks;
	// save separately from experiment to more easily update lasso weights
	std::map<int, int> scope_furthest_layer_seen_in;

	std::vector<std::vector<StateNetwork*>> step_state_networks;

	std::vector<int> input_furthest_layer_needed_in;
	std::vector<int> input_earliest_step_needed_in;

	/**
	 * - only for SEQUENCE_INPUT_INIT_LOCAL
	 *   - (for other input types, check input_furthest_layer_seen_in to determine if needed)
	 */
	std::vector<bool> input_is_new_class;

	std::vector<std::set<int>> scope_additions_needed;
	std::vector<std::set<std::pair<int, int>>> scope_node_additions_needed;

	/**
	 * - even though already have family, calculate correlation to try to build relations between existing families
	 */
	std::vector<std::vector<double>> corr_calc_new_average_vals;
	std::vector<std::vector<double>> corr_calc_new_variances;
	std::vector<std::vector<double>> corr_calc_covariances;
	std::vector<std::vector<Transformation*>> new_transformations;

	/**
	 * TODO:
	 * - correlation between new state and new input
	 */
	std::vector<std::vector<double>> corr_calc_state_average_vals;
	std::vector<std::vector<double>> corr_calc_state_variances;
	std::vector<std::vector<double>> corr_calc_input_average_vals;
	std::vector<std::vector<double>> corr_calc_input_variances;
	std::vector<std::vector<double>> corr_calc_new_covariances;
	std::vector<std::vector<Transformation*>> new_new_transformations;

	/**
	 * - from experiment layer
	 */
	std::vector<int> last_layer_new_indexes;



	void activate(std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  BranchExperimentHistory* branch_experiment_history,
				  RunHelper& run_helper,
				  SequenceHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  SequenceHistory* history);

};

class SequenceHistory {
public:
	Sequence* sequence;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

};

#endif /* SEQUENCE_H */