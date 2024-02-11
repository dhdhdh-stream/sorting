#ifndef SCENARIO_EXPERIMENT_H
#define SCENARIO_EXPERIMENT_H

const int SCENARIO_EXPERIMENT_STATE_LEARN_LINEAR = 0;
const int SCENARIO_EXPERIMENT_STATE_LEARN_NETWORK = 1;

const int SCENARIO_EXPERIMENT_STATE_DONE = 2;

class ScenarioExperiment {
public:
	Scenario* scenario_type;

	int state;
	int state_iter;
	int sub_state_iter;

	Scope* scope;

	double is_sequence_average;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;

	std::vector<double> linear_weights;
	std::vector<std::vector<int>> network_input_indexes;
	Network* network;
	double average_misguess;
	double misguess_variance;

	std::vector<ScopeHistory*> scope_histories;
	std::vector<bool> is_sequence_histories;

}

#endif /* SCENARIO_EXPERIMENT_H */