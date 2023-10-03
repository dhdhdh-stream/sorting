#ifndef SEQUENCE_H
#define SEQUENCE_H

class Sequence {
public:
	std::vector<int> input_types;
	// input_inner_layer always 0
	// input_inner_is_local always false
	std::vector<int> input_inner_ids;
	std::vector<int> input_scope_depths;
	std::vector<bool> input_outer_is_local;
	std::vector<int> input_outer_ids;
	std::vector<double> input_init_vals;

	std::vector<int> output_inner_ids;
	std::vector<int> output_scope_depths;
	std::vector<bool> output_outer_is_local;
	std::vector<int> output_outer_ids;

	Scope* scope;

};

class SequenceHistory {
public:
	Sequence* sequence;

	ScopeHistory* scope_history;

	// scope initially has no local state/obs_snapshots


};

#endif /* SEQUENCE_H */