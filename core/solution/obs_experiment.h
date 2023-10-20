#ifndef OBS_EXPERIMENT_H
#define OBS_EXPERIMENT_H

#include <list>
#include <set>
#include <vector>

class AbstractNode;
class BranchExperiment;
class Scope;
class StateNetwork;

class ObsExperiment {
public:
	std::vector<AbstractNode*> nodes;
	std::vector<std::vector<int>> scope_contexts;
	std::vector<std::vector<int>> node_contexts;
	std::vector<int> obs_indexes;

	std::vector<StateNetwork*> state_networks;

	double resolved_variance;

	double existing_misguess;
	double new_misguess;

	ObsExperiment();
	~ObsExperiment();

	void experiment(std::list<ScopeHistory*>& scope_histories,
					std::list<double>& target_val_histories,
					std::list<double>& predicted_score_histories);

	bool scope_eval(Scope* parent);
	bool branch_experiment_eval(BranchExperiment* branch_experiment);
};

#endif /* OBS_EXPERIMENT_H */