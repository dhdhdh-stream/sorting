#ifndef TEST_NODE_H
#define TEST_NODE_H

#include <vector>

#include "fold_network.h"
#include "compression_network.h"
#include "node.h"
#include "state_network.h"

const int STATE_NO_OUTPUT_MEASURE = 0;
const int STATE_LOCAL_SCOPE_LEARN = 1;	// either works or add scope
const int STATE_LOCAL_SCOPE_MEASURE = 2;
const int STATE_LOCAL_SCOPE_TUNE = 3;
const int STATE_COMPRESS_LEARN = 4;	// if update_existing_scope
const int STATE_COMPRESS_MEASURE = 5;
const int STATE_COMPRESS_TUNE = 6;
const int STATE_DONE = 7;

class TestNode {
public:
	int state;
	int state_iter;
	double sum_error;
	double best_sum_error;	// for tune
	double tune_try;

	double target_error;

	std::vector<int> curr_scope_sizes;
	FoldNetwork* curr_fold;

	std::vector<int> test_scope_sizes;
	FoldNetwork* test_fold;

	StateNetwork* state_network;

	std::vector<CompressionNetwork*> compression_networks;
	std::vector<int> compressed_scope_sizes;

	bool outputs_state;
	bool update_existing_scope;

	TestNode(double target_error,
			 std::vector<int> initial_scope_sizes,
			 FoldNetwork* original_fold);
	~TestNode();

	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  double observation);
	void process(double* flat_inputs,
				 bool* activated,
				 std::vector<std::vector<double>>& state_vals,
				 double observation,
				 double target_val,
				 bool is_zero_train,
				 std::vector<Node*>& nodes);

private:
	void increment();
};

#endif /* TEST_NODE_H */