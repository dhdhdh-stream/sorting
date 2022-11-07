#ifndef TEST_NODE_H
#define TEST_NODE_H

// #include <vector>

// #include "fold_network.h"
// #include "network.h"
// #include "node.h"
// #include "small_network.h"
// #include "sub_fold_network.h"

// const int STAGE_LEARN = 0;
// const int STAGE_MEASURE = 1;
// const int STAGE_TUNE = 2;

// const int STATE_INNER_SCOPE = 0;
// const int STATE_OBS = 1;
// const int STATE_SCORE = 2;
// const int STATE_SCORE_INPUT = 3;
// const int STATE_SCORE_SMALL = 4;
// const int STATE_COMPRESS_STATE = 5;
// const int STATE_COMPRESS_SCOPE = 6;
// const int STATE_COMPRESS_INPUT = 7;
// const int STATE_COMPRESS_SMALL = 8;
// const int STATE_FINAL_TUNE = 9;
// const int STATE_DONE = 10;

// class TestNode {
// public:
// 	int obs_size;
// 	double max_allowable_error;
// 	double max_needed_error;

// 	int state;
// 	int stage;
// 	int stage_iter;
// 	double sum_error;

// 	double new_state_factor;

// 	double best_sum_error;
// 	double tune_try;

// 	std::vector<int> curr_scope_sizes;
// 	std::vector<int> curr_s_input_sizes;
// 	FoldNetwork* curr_fold;
// 	std::vector<FoldNetwork*> curr_scope_input_folds;

// 	std::vector<int> test_scope_sizes;
// 	std::vector<int> test_s_input_sizes;
// 	FoldNetwork* test_fold;
// 	std::vector<FoldNetwork*> test_scope_input_folds;

// 	bool is_scope;
// 	SmallNetwork* action_input_network;
// 	Scope* action;

// 	int new_layer_size;
// 	Network* obs_network;

// 	SubFoldNetwork* curr_score_network;
// 	double average_misguess;
// 	SubFoldNetwork* test_score_network;
// 	std::vector<int> score_input_layer;
// 	std::vector<int> score_input_sizes;
// 	std::vector<SmallNetwork*> score_input_networks;
// 	SmallNetwork* small_score_network;

// 	SubFoldNetwork* curr_compression_network;
// 	int compress_size;
// 	int compress_num_layers;
// 	int compress_new_size;
// 	std::vector<int> compressed_scope_sizes;
// 	std::vector<int> compressed_s_input_sizes;
// 	SubFoldNetwork* test_compression_network;
// 	std::vector<int> input_layer;
// 	std::vector<int> input_sizes;
// 	std::vector<SmallNetwork*> input_networks;
// 	Network* small_compression_network;

// 	TestNode(std::vector<int> initial_scope_sizes,
// 			 std::vector<int> initial_s_input_sizes,
// 			 FoldNetwork* original_fold,
// 			 bool is_scope,
// 			 Scope* action,
// 			 int obs_size,
// 			 double max_allowable_error,
// 			 double max_needed_error);
// 	~TestNode();

// 	void activate(std::vector<std::vector<double>>& state_vals,
// 				  std::vector<std::vector<double>>& s_input_vals,
// 				  std::vector<double>& obs,
// 				  double& predicted_score);
// 	void activate(std::vector<std::vector<double>>& state_vals,
// 				  std::vector<std::vector<double>>& s_input_vals,
// 				  std::vector<std::vector<double>>& inner_flat_vals,
// 				  double& predicted_score);
// 	void process(std::vector<std::vector<double>>& flat_inputs,
// 				 std::vector<std::vector<double>>& state_vals,
// 				 double& predicted_score,
// 				 double target_val,
// 				 std::vector<Node*>& nodes);

// private:
// 	void increment();
// };

#endif /* TEST_NODE_H */