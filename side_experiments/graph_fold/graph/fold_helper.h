#ifndef FOLD_HELPER_H
#define FOLD_HELPER_H

#include <vector>

class NDVector;
class FoldHelper {
public:
	int layer;

	NDVector* input_indexes;

	FoldHelper(int layer);
	~FoldHelper();

	void process(double obs,
				 double* network_input,
				 std::vector<int>& loop_scope_counts);
};

class NDVector {
public:
	int height;

	int value;
	std::vector<NDVector*> inner;	// hardcoded to length 5 for folds

	NDVector(int height);
	~NDVector();

	void set_value(std::vector<int>& index,
				   int curr,
				   int value);
	void get_value(std::vector<int>& index,
				   int curr,
				   int& value);
};

#endif /* FOLD_HELPER_H */