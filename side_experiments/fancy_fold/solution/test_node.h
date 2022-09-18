#ifndef TEST_NODE_H
#define TEST_NODE_H

#include <vector>

#include "fold_network.h"
#include "network.h"

const int STATE_NO_OUTPUT_MEASURE = 0;
const int STATE_LOCAL_SCOPE_LEARN = 1;	// either works or add scope
const int STATE_LOCAL_SCOPE_MEASURE = 2;
const int STATE_LOCAL_SCOPE_ZERO_TRAIN = 3;
const int STATE_COMPRESS_LEARN = 4;	// if update_existing_scope
const int STATE_COMPRESS_MEASURE = 5;
const int STATE_COMPRESS_ZERO_TRAIN = 6;
const int STATE_NEW_SCOPE_LEARN_FOLD = 7;
const int STATE_DONE = 8;

class TestNode {
public:
	int state;
	int state_iter;
	double sum_error;

	double target_error;

	std::vector<int> curr_scope_sizes;
	FoldNetwork* curr_fold;

	std::vector<int> test_scope_sizes;
	FoldNetwork* test_fold;

	Network* state_network;

	std::vector<Network*> compression_networks;

	bool outputs_state;
	bool update_existing_scope;

	TestNode(double target_error,
			 std::vector<int> initial_scope_sizes,
			 FoldNetwork* original_fold);
	~TestNode();

	void activate(std::vector<std::vector<double>>& state_vals,
				  double observation,
				  std::vector<std::vector<double>>& test_state_vals);
	void process(double* flat_inputs,
				 bool* activated,
				 std::vector<std::vector<double>>& state_vals,
				 std::vector<std::vector<double>>& test_state_vals,
				 double observation,
				 double target_val);

	void process_zero_train(std::vector<std::vector<double>>& state_vals,
							std::vector<std::vector<double>>& zero_train_state_vals,
							double observation);

private:
	void increment();
};

#endif /* TEST_NODE_H */