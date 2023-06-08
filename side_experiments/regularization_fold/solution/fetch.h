#ifndef FETCH_H
#define FETCH_H

class Fetch {
public:
	std::vector<int> num_states;

	std::vector<std::vector<AbstractNode*>> nodes;	// only action nodes and scope nodes

	std::vector<int> output_scope_depths;
	std::vector<int> output_input_indexes;
	std::vector<Transformation*> output_transformations;

	void activate(std::vector<double>& flat_vals,
				  std::vector<double>& new_state_vals,
				  );
};

#endif /* FETCH_H */