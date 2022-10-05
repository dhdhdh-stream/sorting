#ifndef TEST_NODE_H
#define TEST_NODE_H

#include <vector>

#include "fold_network.h"
#include "compression_network.h"
#include "node.h"
#include "score_network.h"
#include "state_network.h"

// TODO: divide into 2 layers
const int STATE_LEARN_SCORE = 0;
const int STATE_JUST_SCORE_LEARN = 1;
const int STATE_JUST_SCORE_MEASURE = 2;
const int STATE_JUST_SCORE_TUNE = 3;
const int STATE_LOCAL_SCOPE_LEARN = 4;
const int STATE_LOCAL_SCOPE_MEASURE = 5;
const int STATE_LOCAL_SCOPE_TUNE = 6;
const int STATE_CAN_COMPRESS_LEARN = 7;
const int STATE_CAN_COMPRESS_MEASURE = 8;
const int STATE_COMPRESS_LEARN = 9;
const int STATE_COMPRESS_MEASURE = 10;
const int STATE_COMPRESS_TUNE = 11;
const int STATE_ADD_SCOPE_LEARN = 12;
const int STATE_ADD_SCOPE_MEASURE = 13;
const int STATE_ADD_SCOPE_TUNE = 14;
const int STATE_DONE = 15;

class TestNode {
public:
	int obs_size;

	int state;
	int state_iter;
	double sum_error;
	double best_sum_error;	// for tune
	double tune_try;

	std::vector<int> curr_scope_sizes;
	FoldNetwork* curr_fold;

	std::vector<int> test_scope_sizes;
	FoldNetwork* test_fold;

	ScoreNetwork* score_network;

	StateNetwork* state_network;

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

	int new_scope_size;

	TestNode(std::vector<int> initial_scope_sizes,
			 FoldNetwork* original_fold,
			 int obs_size);
	~TestNode();

	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<bool>& scopes_on,
				  std::vector<double>& obs);
	void process(std::vector<std::vector<double>>& flat_inputs,
				 std::vector<std::vector<double>>& state_vals,
				 double target_val,
				 std::vector<Node*>& nodes);

private:
	void increment();
};

#endif /* TEST_NODE_H */