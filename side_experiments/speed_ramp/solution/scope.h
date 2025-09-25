/**
 * TODO:
 * - predict score relative to current average
 *   - more likely for signal to predict relative than global true
 *     - so predicting relative leads to less adjustments
 */

#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "input.h"

class AbstractExperiment;
class AbstractNode;
class AbstractNodeHistory;
class DefaultSignal;
class ExploreExperiment;
class ExploreExperimentHistory;
class EvalExperimentHistory;
class Factor;
class Signal;
class SignalExperiment;
class SignalExperimentHistory;
class Solution;
class SolutionWrapper;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<Factor*> factors;

	/**
	 * - separate actions/obs for signal
	 *   - don't have to worry about changes destroying signal
	 *   - don't have to worry about branching
	 * 
	 * - simply only worry about signal at end
	 *   - impact of an explore will be calculated through train_existing step
	 * 
	 * - signals need to be perfect
	 *   - at least for everything so far
	 *   - at least when predicting good
	 *   - anytime a trap is hit, either learn to recognize or give up on signal
	 * 
	 * - always perform signal actions
	 *   - as experiments done in their presence
	 */
	std::vector<int> signal_pre_actions;
	std::vector<int> signal_post_actions;
	std::vector<Signal*> signals;
	DefaultSignal* default_signal;

	std::vector<Scope*> child_scopes;

	int num_generalize_successes;

	SignalExperiment* signal_experiment;
	SignalExperimentHistory* signal_experiment_history;

	Scope();
	~Scope();

	void signal_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper);

	void invalidate_factor(ScopeHistory* scope_history,
						   int f_index);

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<bool> factor_initialized;
	std::vector<double> factor_values;

	std::vector<std::vector<double>> signal_pre_obs;
	std::vector<std::vector<double>> signal_post_obs;

	int num_actions_snapshot;

	std::vector<ExploreExperimentHistory*> explore_experiment_callbacks;
	std::vector<EvalExperimentHistory*> eval_experiment_callbacks;
	std::vector<SignalExperimentHistory*> signal_experiment_callbacks;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */