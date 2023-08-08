#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

const int BRANCH_EXPERIMENT_STEP_TYPE_ACTION = 0;
const int BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE = 0;

class Scale;
class ScopeHistory;
class ScoreNetwork;
class StateNetwork;
class StateNetworkHistory;
class Sequence;

class BranchExperimentHistory;
class BranchExperiment : public AbstractExperiment {
public:
	int num_steps;
	std::vector<int> step_types;
	// std::vector<Action> actions;
	std::vector<Sequence*> sequences;

	int exit_depth;
	int exit_node_id;

	double seed_start_predicted_score;
	double seed_start_scale_factor;
	std::vector<double> seed_state_vals_snapshot;
	ScopeHistory* seed_context_history;
	double seed_target_val;		// set at run end
	// only worry about starting score network for seed, as due to updates for inner scopes, seed may quickly become irrelevant

	ScoreNetwork* starting_score_network;
	ScoreNetwork* starting_misguess_network;
	ScoreNetwork* starting_original_score_network;
	ScoreNetwork* starting_original_misguess_network;
	double branch_weight;

	std::vector<std::vector<StateNetwork*>> step_state_networks;
	std::vector<ScoreNetwork*> step_score_networks;
	/**
	 * - simply share networks instead of having separate networks for sequence
	 *   - can be seen as generalization anyways
	 */

	std::vector<Scale*> sequence_scale_mods;

	// for BRANCH_EXPERIMENT_STEP_TYPE_ACTION
	std::vector<std::vector<int>> new_action_node_target_indexes;
	std::vector<std::vector<StateNetwork*>> new_action_node_state_networks;

	BranchExperiment(std::vector<int> scope_context,
					 std::vector<int> node_context,
					 int num_steps,
					 std::vector<int> step_types,
					 // std::vector<Action> actions,
					 std::vector<Sequence*> sequences,
					 int exit_depth,
					 int exit_node_id,
					 double seed_start_predicted_score,
					 double seed_start_scale_factor,
					 std::vector<double> seed_state_vals_snapshot,
					 ScopeHistory* seed_context_history,
					 ScoreNetwork* existing_misguess_network);
	~BranchExperiment();

	void activate(std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  RunHelper& run_helper,
				  BranchExperimentHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  BranchExperimentHistory* history);

	void explore_activate(std::vector<double>& flat_vals,
						  std::vector<ForwardContextLayer>& context,
						  RunHelper& run_helper,
						  BranchExperimentHistory* history);
	void explore_transform();

	void seed_activate();
	void seed_pre_activate_helper(std::vector<double>& new_state_vals,
								  bool can_zero,
								  std::vector<double>& obs_snapshots,
								  std::vector<std::vector<double>>& state_vals_snapshots,
								  std::vector<std::vector<double>>& new_state_vals_snapshots,
								  std::vector<std::vector<StateNetworkHistory*>>& new_state_network_histories,
								  ScopeHistory* scope_history);

	void experiment_activate(std::vector<double>& flat_vals,
							 std::vector<ForwardContextLayer>& context,
							 RunHelper& run_helper,
							 BranchExperimentHistory* history);
	void experiment_pre_activate_helper(bool on_path,
										int context_index,
										double& temp_scale_factor,
										RunHelper& run_helper,
										ScopeHistory* scope_history);
	void experiment_backprop(std::vector<BackwardContextLayer>& context,
							 RunHelper& run_helper,
							 BranchExperimentHistory* history);
	void experiment_transform();

	void clean_activate(std::vector<double>& flat_vals,
						std::vector<ForwardContextLayer>& context,
						RunHelper& run_helper,
						BranchExperimentHistory* history);
	void clean_pre_activate_helper(bool on_path,
								   double& temp_scale_factor,
								   std::vector<int> temp_scope_context,
								   std::vector<int> temp_node_context,
								   RunHelper& run_helper,
								   ScopeHistory* scope_history);
	void clean_backprop(std::vector<BackwardContextLayer>& context,
						RunHelper& run_helper,
						BranchExperimentHistory* history);
	void first_clean_transform();
	void second_clean_transform();

	void wrapup_activate(std::vector<double>& flat_vals,
						 std::vector<ForwardContextLayer>& context,
						 RunHelper& run_helper,
						 BranchExperimentHistory* history);
	void wrapup_pre_activate_helper(double& temp_scale_factor,
									RunHelper& run_helper,
									ScopeHistory* scope_history);
	void wrapup_backprop(std::vector<BackwardContextLayer>& context,
						 double& scale_factor_error,
						 RunHelper& run_helper,
						 BranchExperimentHistory* history);
	void wrapup_transform();
};

class ExitNetworkHistory;
class ScoreNetworkHistory;
class SequenceHistory;

class BranchExperimentHistory : public AbstractExperimentHistory {
public:
	BranchExperiment* experiment;

	std::vector<double> starting_state_vals_snapshot;
	std::vector<double> starting_new_state_vals_snapshot;
	double existing_predicted_score;

	bool is_branch;
	ScoreNetworkHistory* score_network_history;
	ScoreNetworkHistory* misguess_network_history;
	double score_network_output;
	double misguess_network_output;

	std::vector<double> step_obs_snapshots;
	std::vector<std::vector<double>> step_starting_new_state_vals_snapshots;
	std::vector<std::vector<StateNetworkHistory*>> step_state_network_histories;
	std::vector<std::vector<double>> step_ending_new_state_vals_snapshots;
	std::vector<ScoreNetworkHistory*> step_score_network_histories;
	std::vector<double> step_score_network_outputs;
	std::vector<std::vector<int>> step_input_sequence_step_indexes;
	std::vector<std::vector<std::vector<double>>> step_input_vals_snapshots;
	std::vector<std::vector<std::vector<StateNetworkHistory*>>> step_input_state_network_histories;

	std::vector<SequenceHistory*> sequence_histories;

	std::vector<std::vector<double>> exit_state_vals_snapshot;
	std::vector<double> ending_new_state_vals_snapshot;
	std::vector<ExitNetworkHistory*> exit_network_histories;

	BranchExperimentHistory(BranchExperiment* experiment);
	~BranchExperimentHistory();
};

#endif /* BRANCH_EXPERIMENT_H */