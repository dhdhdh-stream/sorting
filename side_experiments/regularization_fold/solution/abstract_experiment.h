#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_LOOP = 1;

class AbstractExperiment {
public:
	int type;

	std::vector<int> scope_context;
	std::vector<int> node_context;

	int num_new_states;
	/**
	 * - index 0 is global
	 * - contexts in the middle
	 * - last index is inner
	 */
	std::vector<std::map<int, std::vector<std::vector<Network*>>>> action_node_state_networks;
	// temporary to determine state needed
	std::vector<std::map<int, std::vector<Network*>>> action_node_score_networks;

	std::vector<std::vector<std::map<StateDefinition*, ExitNetwork*>>> exit_node_state_networks;
};

#endif /* ABSTRACT_EXPERIMENT_H */