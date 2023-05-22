#ifndef FOLD_H
#define FOLD_H

class Fold {
public:

	int parent_scope_id;
	// keep fixed even if parent scope updates
	int num_existing_states;

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<int> existing_scope_ids;
	std::vector<std::vector<Input*>> existing_scope_inputs;
	std::vector<Action> actions;

	// temporary to help measure state impact
	std::map<int, std::vector<Network*>> test_outer_score_networks;

};

class FoldHistory {
public:
	Fold* fold;


};

#endif /* FOLD_H */