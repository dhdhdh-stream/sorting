#ifndef SEQUENCE_H
#define SEQUENCE_H

const int SEQUENCE_INPUT_TYPE_NONE = 0;
const int SEQUENCE_INPUT_TYPE_LOCAL = 1;
const int SEQUENCE_INPUT_TYPE_PREVIOUS = 2;
const int SEQUENCE_INPUT_TYPE_LAST_SEEN = 3;
// if empty (i.e., don't initialize), don't include in input_types, etc.

class Sequence {
public:
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

	std::vector<int> input_types;
	/**
	 * - if it's the same input in multiple layers, set at top layer, and have it cascade down
	 *   - insert new state by BRANCH_EXPERIMENT_STATE_WRAPUP
	 */
	std::vector<int> input_target_layers;
	std::vector<int> input_target_indexes;
	/**
	 * - needs to be state that isn't passed down further
	 * 
	 * - negative indexing from end
	 *   - original scope is 0
	 *     - (new experiment context not added yet)
	 * 
	 * - no longer needed by BRANCH_EXPERIMENT_STATE_WRAPUP
	 */
	std::vector<int> input_local_scope_depths;
	std::vector<int> input_local_input_indexes;
	std::vector<int> input_previous_step_index;
	std::vector<int> input_previous_input_index;
	std::vector<int> input_last_seen_class_ids;
	std::vector<bool> input_has_transform;
	std::vector<Transformation> input_transformations;
	/**
	 * - transform on activate start
	 *   - so if input_is_new_class, family is local family
	 */

	std::vector<std::vector<int>> node_ids;
	/**
	 * - scope nodes always given empty context
	 *   - so will always result in reasonable sequence
	 *     - (though may not be optimal due to original scope's later explored early exits)
	 *   - even if from halfway activate
	 * 
	 * - TODO: include branch nodes
	 */

	AbstractExperiment* experiment;
	int step_index;		// 0 if loop

	std::map<int, std::vector<std::vector<StateNetwork*>>> state_networks;
	// save separately from experiment to more easily update lasso weights
	std::map<int, int> scope_furthest_layer_seen_in;

	std::vector<std::vector<StateNetwork*>> step_state_networks;

	std::vector<int> input_furthest_layer_needed_in;
	std::vector<std::vector<bool>> input_steps_needed_in;

	/**
	 * - for SEQUENCE_INPUT_TYPE_LOCAL and SEQUENCE_INPUT_TYPE_PREVIOUS
	 *   - (for other types, check input_furthest_layer_seen_in to determine if needed)
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
	std::vector<std::vector<TransformationHelper>> new_transformations;

	/**
	 * - correlation between new state and new input
	 */
	std::vector<double> corr_calc_state_average_vals;
	std::vector<double> corr_calc_state_variances;
	std::vector<std::vector<double>> corr_calc_input_average_vals;
	std::vector<std::vector<double>> corr_calc_input_variances;
	std::vector<std::vector<double>> corr_calc_new_covariances;
	std::vector<std::vector<TransformationHelper>> new_new_transformations;

	std::vector<int> input_index_translations;

	Sequence(std::vector<Scope*> scopes,
			 std::vector<int> starting_node_ids,
			 std::vector<int> input_types,
			 std::vector<int> input_target_layers,
			 std::vector<int> input_target_indexes,
			 std::vector<int> input_local_scope_depths,
			 std::vector<int> input_local_input_indexes,
			 std::vector<int> input_previous_step_index,
			 std::vector<int> input_previous_input_index,
			 std::vector<int> input_last_seen_class_ids,
			 std::vector<bool> input_has_transform,
			 std::vector<Transformation> input_transformations,
			 std::vector<std::vector<int>> node_ids);
	~Sequence();

	void activate_pull(std::vector<double>& input_vals,
					   std::vector<ForwardContextLayer>& context,
					   std::vector<std::vector<double>>& previous_vals,
					   BranchExperimentHistory* branch_experiment_history,
					   RunHelper& run_helper);
	void activate(std::vector<double>& input_vals,
				  std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  SequenceHistory* history);
	void activate_reset(std::vector<double>& input_vals,
						std::vector<ForwardContextLayer>& context,
						std::vector<std::vector<double>>& previous_vals);
	/**
	 * - by EXPERIMENT_STATE_WRAPUP, reset immediately after activate
	 */

	void backprop_pull(std::vector<double>& input_errors,
					   std::vector<BackwardContextLayer>& context,
					   std::vector<std::vector<double>>& previous_errors);
	void backprop(std::vector<double>& input_errors,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  SequenceHistory* history);
	void backprop_reset(std::vector<double>& input_errors,
						std::vector<BackwardContextLayer>& context,
						std::vector<std::vector<double>>& previous_errors);

	void explore_activate_pull(std::vector<double>& input_vals,
							   std::vector<ForwardContextLayer>& context,
							   std::vector<std::vector<double>>& previous_vals,
							   RunHelper& run_helper);
	void explore_activate_reset(std::vector<double>& input_vals,
								std::vector<ForwardContextLayer>& context,
								std::vector<std::vector<double>>& previous_vals);

	void experiment_pre_activate_helper(bool on_path,
										int context_index,
										std::vector<double>& input_vals,
										RunHelper& run_helper,
										ScopeHistory* scope_history);
	void experiment_experiment_activate_helper(std::vector<double>& input_vals,
											   BranchExperimentHistory* branch_experiment_history,
											   RunHelper& run_helper);
	void experiment_activate_pull(std::vector<double>& input_vals,
								  std::vector<ForwardContextLayer>& context,
								  std::vector<std::vector<double>>& previous_vals,
								  BranchExperimentHistory* branch_experiment_history,
								  RunHelper& run_helper);
	void experiment_activate_reset(std::vector<double>& input_vals,
								   std::vector<ForwardContextLayer>& context,
								   std::vector<std::vector<double>>& previous_vals);
	void experiment_backprop_pull(std::vector<double>& input_errors,
								  std::vector<BackwardContextLayer>& context,
								  std::vector<std::vector<double>>& previous_errors);
	void experiment_backprop_reset(std::vector<double>& input_errors,
								   std::vector<BackwardContextLayer>& context,
								   std::vector<std::vector<double>>& previous_errors);

	void measure_pre_activate_helper(std::vector<double>& input_vals,
									 ScopeHistory* scope_history);
	void measure_activate_pull(std::vector<double>& input_vals,
							   std::vector<ForwardContextLayer>& context);
	void measure_activate_reset(std::vector<double>& input_vals,
								std::vector<ForwardContextLayer>& context);

	void first_clean_pre_activate_helper(bool on_path,
										 std::vector<double>& input_vals,
										 RunHelper& run_helper,
										 ScopeHistory* scope_history);
	void first_clean_step_activate_helper(int a_index,
										  std::vector<double>& input_vals,
										  BranchExperimentHistory* branch_experiment_history,
										  RunHelper& run_helper);
	void first_clean_activate_pull(std::vector<double>& input_vals,
								   std::vector<ForwardContextLayer>& context,
								   std::vector<std::vector<double>>& previous_vals,
								   BranchExperimentHistory* branch_experiment_history,
								   RunHelper& run_helper);
	void first_clean_activate_reset(std::vector<double>& input_vals,
									std::vector<ForwardContextLayer>& context,
									std::vector<std::vector<double>>& previous_vals);
	void first_clean_backprop_pull(std::vector<double>& input_errors,
								   std::vector<BackwardContextLayer>& context,
								   std::vector<std::vector<double>>& previous_errors);
	void first_clean_backprop_reset(std::vector<double>& input_errors,
									std::vector<BackwardContextLayer>& context,
									std::vector<std::vector<double>>& previous_errors);

	void second_clean_pre_activate_helper(bool on_path,
										  std::vector<int>& temp_scope_context,
										  std::vector<int>& temp_node_context,
										  std::vector<double>& input_vals,
										  RunHelper& run_helper,
										  ScopeHistory* scope_history);
	void second_clean_step_activate_helper(int a_index,
										   std::vector<int> temp_scope_context,
										   std::vector<int> temp_node_context,
										   std::vector<double>& input_vals,
										   BranchExperimentHistory* branch_experiment_history,
										   RunHelper& run_helper);
	void second_clean_activate_pull(std::vector<double>& input_vals,
									std::vector<ForwardContextLayer>& context,
									std::vector<std::vector<double>>& previous_vals,
									BranchExperimentHistory* branch_experiment_history,
									RunHelper& run_helper);
	void second_clean_activate_reset(std::vector<double>& input_vals,
									 std::vector<ForwardContextLayer>& context,
									 std::vector<std::vector<double>>& previous_vals);
	void second_clean_backprop_pull(std::vector<double>& input_errors,
									std::vector<BackwardContextLayer>& context,
									std::vector<std::vector<double>>& previous_errors);
	void second_clean_backprop_reset(std::vector<double>& input_errors,
									 std::vector<BackwardContextLayer>& context,
									 std::vector<std::vector<double>>& previous_errors);

	void wrapup_activate_pull(std::vector<double>& input_vals,
							  std::vector<ForwardContextLayer>& context);
	void wrapup_activate_reset(std::vector<double>& input_vals,
							   std::vector<ForwardContextLayer>& context);
};

class SequenceHistory {
public:
	Sequence* sequence;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	SequenceHistory(Sequence* sequence);
	~SequenceHistory();
};

#endif /* SEQUENCE_H */