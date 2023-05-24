#ifndef EXPERIMENT_H
#define EXPERIMENT_H

class Experiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;	// store explore node index in node_context[0]

	// keep fixed even if parent scope updates
	int num_existing_states;

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;
	std::vector<ObjectDefinition*> fetch_inner_inputs;
	/**
	 * Notes:
	 * - can reuse between different inner scopes
	 * - if explore succeeds, simply assume assignment fits, and build on top of
	 */
	std::vector<std::vector<InnerInput*>> existing_scope_inputs;
	std::vector<Action> actions;

	// temporary to help measure state impact
	std::map<int, std::vector<Network*>> test_outer_score_networks;

};

class ExperimentHistory {
public:
	Experiment* Experiment;


};

#endif /* NEW_SEQUENCE_EXPERIMENT_H */