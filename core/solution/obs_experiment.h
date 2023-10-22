#ifndef OBS_EXPERIMENT_H
#define OBS_EXPERIMENT_H

#include <list>
#include <set>
#include <vector>

class AbstractNode;
class BranchExperiment;
class BranchExperimentHistory;
class Scope;
class ScopeHistory;
class StateNetwork;

class ObsExperiment {
public:
	std::vector<AbstractNode*> nodes;
	std::vector<std::vector<int>> scope_contexts;
	std::vector<std::vector<int>> node_contexts;
	std::vector<int> obs_indexes;

	std::vector<StateNetwork*> state_networks;

	// for update_diffs()
	std::vector<std::vector<int>> d_obs_indexes;
	std::vector<std::vector<double>> d_obs_vals;

	double resolved_variance;

	double existing_misguess;
	double new_misguess;

	ObsExperiment();
	~ObsExperiment();

	void experiment(std::list<ScopeHistory*>& scope_histories,
					std::vector<double>& diffs);
	void hook();
	void unhook();
	void flat_vals_branch_experiment_helper(std::vector<int>& scope_context,
											std::vector<int>& node_context,
											BranchExperimentHistory* branch_experiment_history,
											int d_index,
											int stride_size,
											std::vector<double>& flat_vals);
	void flat_vals_helper(std::vector<int>& scope_context,
						  std::vector<int>& node_context,
						  ScopeHistory* scope_history,
						  int d_index,
						  int stride_size,
						  std::vector<double>& flat_vals);
	void flat(std::vector<double>& flat_vals,
			  std::vector<double>& diffs);
	void rnn_vals_branch_experiment_helper(std::vector<int>& scope_context,
										   std::vector<int>& node_context,
										   BranchExperimentHistory* branch_experiment_history,
										   std::vector<int>& obs_indexes,
										   std::vector<double>& obs_vals);
	void rnn_vals_helper(std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 ScopeHistory* scope_history,
						 std::vector<int>& obs_indexes,
						 std::vector<double>& obs_vals);
	void rnn(std::vector<double>& diffs);
	void evaluate(std::vector<double>& diffs);

	bool scope_eval(Scope* parent);
	bool branch_experiment_eval(BranchExperiment* branch_experiment,
								bool update_starting);

	void update_diffs(std::vector<double>& diffs);
};

#endif /* OBS_EXPERIMENT_H */