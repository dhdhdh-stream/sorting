#ifndef TEST_NODE_H
#define TEST_NODE_H

const int STAGE_LEARN = 0;
const int STAGE_MEASURE = 1;
const int STAGE_TUNE = 2;

const int STATE_OBS = 0;
const int STATE_SCORE = 1;
const int STATE_SCORE_INPUT = 2;
const int STATE_SCORE_SMALL = 3;
const int STATE_COMPRESS_STATE = 4;
const int STATE_COMPRESS_SCOPE = 5;
const int STATE_COMPRESS_INPUT = 6;
const int STATE_COMPRESS_SMALL = 7;
const int STATE_FINAL_TUNE = 8;
const int STATE_DONE = 9;

class TestNode {
public:
	int obs_size;

	int state;
	int stage;
	int stage_iter;
	double sum_error;

	double best_sum_error;
	double tune_try;

	std::vector<int> curr_scope_sizes;
	FoldNetwork* curr_fold;

	std::vector<int> test_scope_sizes;
	FoldNetwork* test_fold;

	int new_layer_size;
	Network* obs_network;

	SubFoldNetwork* curr_score_network;	// not ending score, but change in score
	double average_misguess;
	SubFoldNetwork* test_score_network;
	std::vector<int> score_input_layer;
	std::vector<int> score_input_sizes;
	std::vector<Network*> score_input_networks;
	// zero inputs layer by layer, starting from last to most recent, to determine inputs needed?
	Network* small_score_network;

	SubFoldNetwork* curr_compression_network;
	int compress_size;
	int compress_num_layers;	// can compress to nothing
	int compress_new_size;
	vector<int> compressed_scope_sizes;
	SubFoldNetwork* test_compression_network;
	std::vector<int> input_layer;	// take from this, and update +1
	std::vector<int> input_sizes;
	std::vector<std::vector<Network*>> input_networks;
	Network* small_compression_network;

	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<double>& obs,
				  double& predicted_score);
	void process(std::vector<std::vector<double>>& flat_inputs,
				 std::vector<std::vector<double>>& state_vals,
				 double& predicted_score,
				 double target_val,
				 std::vector<Node*>& nodes);
};

#endif /* TEST_NODE_H */