#ifndef SEQUENCE_H
#define SEQUENCE_H

class Sequence {
public:
	std::vector<int> input_types;
	// input_inner_layer always 0
	std::vector<int> input_inner_ids;
	std::vector<int> input_scope_depths;
	std::vector<int> input_outer_ids;
	std::vector<double> input_init_vals;

	std::vector<int> output_inner_ids;
	std::vector<int> output_scope_depths;
	std::vector<int> output_outer_ids;

	Scope* scope;

};

#endif /* SEQUENCE_H */