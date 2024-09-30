#ifndef MARKOV_EXPERIMENT_H
#define MARKOV_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "action.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class Network;
class Problem;
class Scope;
class Solution;

const int MARKOV_EXPERIMENT_STATE_TRAIN = 0;
const int MARKOV_EXPERIMENT_STATE_MEASURE = 1;

const int MARKOV_EXPERIMENT_NUM_OPTIONS = 4;	// temp

class MarkovExperimentHistory;
class MarkovExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int sub_state_iter;

	AbstractNode* exit_next_node;

	std::vector<std::vector<int>> step_types;
	std::vector<std::vector<Action>> actions;
	std::vector<std::vector<Scope*>> scopes;
	std::vector<Network*> networks;
	Network* halt_network;

	double combined_score;

	std::vector<int> option_histories;
	std::vector<std::vector<std::vector<double>>> obs_histories;
	std::vector<double> target_val_histories;

	MarkovExperiment(Scope* scope_context,
					 AbstractNode* node_context,
					 bool is_branch);
	~MarkovExperiment();
	void decrement(AbstractNode* experiment_node);

	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void train_activate(AbstractNode*& curr_node,
						Problem* problem,
						std::vector<ContextLayer>& context,
						RunHelper& run_helper,
						MarkovExperimentHistory* history);
	void train_backprop(double target_val,
						RunHelper& run_helper);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  MarkovExperimentHistory* history);
	void measure_backprop(double target_val,
						  RunHelper& run_helper);

	void finalize(Solution* duplicate);
};

class MarkovExperimentHistory : public AbstractExperimentHistory {
public:
	MarkovExperimentHistory(MarkovExperiment* experiment);
};

#endif /* MARKOV_EXPERIMENT_H */