#ifndef FOLD_HELPER_H
#define FOLD_HELPER_H

class FoldHelper {
public:
	SolutionNode* parent;

	int layer;

	NDVector* input_indexes;

	int new_state_size;
	Network* new_state_network;

	FoldHelper(SolutionNode* parent,
			   int layer);
	~FoldHelper();

	void set_index(std::vector<int>& loop_scope_counts,
				   int& curr_index);
	void initialize_new_state_network(int new_state_size);
	void process_and_rerun_target_state(
		std::vector<int>& loop_scope_counts,
		double observations,
		int input_index_on,
		double* flat_inputs,
		bool* activated,
		std::vector<double>& new_state_vals,
		std::vector<SolutionNode*>& backprop_nodes,
		std::vector<AbstractNetworkHistory*>& network_historys);

class NDVector {
public:
	int height;

	int value;
	std::vector<NDVector*> inner;	// hardcoded to length 6 for folds

	NDVector(int height);
	~NDVector();

	void set_value(std::vector<int>& index,
				   int& curr,
				   int value);
	double get_value(std::vector<int>& index,
					 int& curr);
};

#endif /* FOLD_HELPER_H */