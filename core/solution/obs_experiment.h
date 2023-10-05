/**
 * TODO: instead of running on new problem instances every time, run on a small number of seeds
 */

#ifndef OBS_EXPERIMENT_H
#define OBS_EXPERIMENT_H

#include <set>
#include <vector>

#include "problem.h"

class AbstractNode;
class BranchExperiment;
class FlatNetwork;
class Scope;
class StateNetwork;

const int OBS_EXPERIMENT_STATE_FLAT = 0;
const int OBS_EXPERIMENT_STATE_RNN = 1;
const int OBS_EXPERIMENT_STATE_DONE = 2;

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

	std::set<int> resolved_network_indexes;
	double resolved_variance;

	double new_average_misguess;

	ObsExperiment(Scope* parent_scope);
	~ObsExperiment();

	void hook(void* key);
	void unhook(void* key);

	void backprop(double target_val,
				  double existing_predicted_score,
				  std::vector<int>& obs_indexes,
				  std::vector<double>& obs_vals);

	void flat(double target_val,
			  double existing_predicted_score,
			  std::vector<int>& obs_indexes,
			  std::vector<double>& obs_vals);
	void trim();
	void rnn(double target_val,
			 double existing_predicted_score,
			 std::vector<int>& obs_indexes,
			 std::vector<double>& obs_vals);

	void scope_eval(Scope* parent);
	void branch_experiment_eval(BranchExperiment* branch_experiment);
};

#endif /* OBS_EXPERIMENT_H */