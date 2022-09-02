#ifndef FOLD_HELPER_H
#define FOLD_HELPER_H

class FoldHelper {
public:
	int layer;

	NDVector* input_indexes;

	FoldHelper(int layer);
	~FoldHelper();

	void set_index(std::vector<int>& loop_scope_counts,
				   int& curr_index);
	void process(double* flat_inputs,
				 bool* activated,
				 std::vector<int>& loop_scope_counts);
};

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