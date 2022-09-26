#ifndef TEST_NODE_H
#define TEST_NODE_H

#include <vector>

#include "fold_network.h"
#include "compression_network.h"
#include "network.h"
#include "node.h"
#include "score_network.h"
#include "state_network.h"

const int STATE_LEARN_SCORE = 0;
const int STATE_JUST_SCORE_LEARN = 1;
const int STATE_JUST_SCORE_MEASURE = 2;
const int STATE_LOCAL_SCOPE_LEARN = 3;
const int STATE_LOCAL_SCOPE_MEASURE = 4;
const int STATE_LOCAL_SCOPE_TUNE = 5;
const int STATE_CAN_COMPRESS_LEARN = 6;
const int STATE_CAN_COMPRESS_MEASURE = 7;
const int STATE_COMPRESS_LEARN = 8;
const int STATE_COMPRESS_MEASURE = 9;
const int STATE_COMPRESS_TUNE = 10;
const int STATE_DONE = 11;

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

	ScoreNetwork* score_network;

	StateNetwork* state_network;
	Network* test_state_network;

	std::vector<int> compress_num_scopes;
	std::vector<int> compress_sizes;
	std::vector<CompressionNetwork*> compression_networks;
	std::vector<std::vector<int>> compressed_scope_sizes;

	int test_compress_num_scopes;
	int test_compress_sizes;
	CompressionNetwork* test_compression_network;
	std::vector<int> test_compressed_scope_sizes;

	bool just_score;
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