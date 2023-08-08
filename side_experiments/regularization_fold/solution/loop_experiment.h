/**
 * - on loop reuse (and occasionally), try all iters to learn state (or update weights)
 *   - don't try to learn continuous relation between state+iters and score
 *     - ineffective when relation is sharp
 *     - would still need to try all iters on reuse anyways
 *       - can't calculate iter gradient for iter equation with state being inaccurate
 *       - can't calculate state gradient for score equation with iter being inaccurate
 *         - and zigzag is unreliable
 */

#ifndef LOOP_EXPERIMENT_H
#define LOOP_EXPERIMENT_H

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class Scale;
class ScoreNetwork;
class Sequence;

class LoopExperimentHistory;
class LoopExperiment : public AbstractExperiment {
public:
	/**
	 * - for inputs, trace from matching outer scope
	 *   - keep (i.e., initialize) with 75% probability at explore(?)
	 *     - if kept at explore, then simply always include in new scope
	 */
	Sequence* sequence;
	Scale* scale_mod;

	ScoreNetwork* continue_score_network;
	ScoreNetwork* continue_misguess_network;
	ScoreNetwork* halt_score_network;
	ScoreNetwork* halt_misguess_network;

	LoopExperiment(std::vector<int> scope_context,
				   std::vector<int> node_context,
				   Sequence* sequence,
				   ScoreNetwork* existing_misguess_network);
	~LoopExperiment();

	void activate(std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  RunHelper& run_helper,
				  LoopExperimentHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  LoopExperimentHistory* history);

	void explore_activate(std::vector<double>& flat_vals,
						  std::vector<ForwardContextLayer>& context,
						  RunHelper& run_helper);
	void explore_transform();

	void experiment_activate(std::vector<double>& flat_vals,
							 std::vector<ForwardContextLayer>& context,
							 RunHelper& run_helper,
							 LoopExperimentHistory* history);
	void experiment_pre_activate_helper(bool on_path,
										int context_index,
										double& temp_scale_factor,
										RunHelper& run_helper,
										ScopeHistory* scope_history);
	void experiment_backprop(std::vector<BackwardContextLayer>& context,
							 RunHelper& run_helper,
							 LoopExperimentHistory* history);

	void measure_activate(std::vector<double>& flat_vals,
						  std::vector<ForwardContextLayer>& context,
						  RunHelper& run_helper);
	void measure_pre_activate_helper(double& temp_scale_factor,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history);

	void experiment_transform();

	void clean_activate(std::vector<double>& flat_vals,
						std::vector<ForwardContextLayer>& context,
						RunHelper& run_helper,
						LoopExperimentHistory* history);
	void clean_pre_activate_helper(bool on_path,
								   double& temp_scale_factor,
								   std::vector<int> temp_scope_context,
								   std::vector<int> temp_node_context,
								   RunHelper& run_helper,
								   ScopeHistory* scope_history);
	void clean_backprop(std::vector<BackwardContextLayer>& context,
						RunHelper& run_helper,
						LoopExperimentHistory* history);
	void first_clean_transform();
	void second_clean_transform();

	void wrapup_activate(std::vector<double>& flat_vals,
						 std::vector<ForwardContextLayer>& context,
						 RunHelper& run_helper,
						 LoopExperimentHistory* history);
	void wrapup_pre_activate_helper(double& temp_scale_factor,
									RunHelper& run_helper,
									ScopeHistory* scope_history);
	void wrapup_backprop(std::vector<BackwardContextLayer>& context,
						 double& scale_factor_error,
						 RunHelper& run_helper,
						 LoopExperimentHistory* history);
	void wrapup_transform();
};

class ExitNetworkHistory;
class ScoreNetworkHistory;
class SequenceHistory;

class LoopExperimentHistory : public AbstractExperimentHistory {
public:
	LoopExperiment* experiment;

	std::vector<std::vector<double>> iter_input_vals_snapshots;
	std::vector<std::vector<double>> iter_new_state_vals_snapshots;
	std::vector<ScoreNetworkHistory*> continue_score_network_histories;
	std::vector<double> continue_score_network_outputs;
	std::vector<ScoreNetworkHistory*> continue_misguess_network_histories;
	std::vector<double> continue_misguess_network_outputs;
	std::vector<double> halt_score_snapshots;
	std::vector<double> halt_misguess_snapshots;
	std::vector<SequenceHistory*> sequence_histories;

	std::vector<double> ending_input_vals_snapshot;
	std::vector<double> ending_new_state_vals_snapshot;

	ScoreNetworkHistory* halt_score_network_history;
	double halt_score_network_output;
	ScoreNetworkHistory* halt_misguess_network_history;
	double halt_misguess_network_output;

	std::vector<std::vector<double>> exit_state_vals_snapshot;
	std::vector<ExitNetworkHistory*> exit_network_histories;

	LoopExperimentHistory(LoopExperiment* experiment);
	~LoopExperimentHistory();
};

#endif /* LOOP_EXPERIMENT_H */