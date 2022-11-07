#ifndef FOLD_H
#define FOLD_H

#include <vector>

#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "small_network.h"
#include "scope.h"
#include "sub_fold_network.h"

const int STAGE_LEARN = 0;
const int STAGE_MEASURE = 1;

const int STATE_INNER_SCOPE_INPUT = 0;
const int STATE_INNER_SCOPE = 1;
const int STATE_OBS = 2;
const int STATE_SCORE = 3;
const int STATE_SCORE_INPUT = 4;
const int STATE_SCORE_SMALL = 5;
const int STATE_SCORE_TUNE = 6;
const int STATE_COMPRESS_STATE = 7;
const int STATE_COMPRESS_SCOPE = 8;
const int STATE_COMPRESS_INPUT = 9;
const int STATE_COMPRESS_SMALL = 10;
const int STATE_FINAL_TUNE = 11;
const int STATE_DONE = 12;

class Fold {
public:
	std::string id;

	int sequence_length;
	std::vector<Scope*> compound_actions;
	std::vector<int> obs_sizes;
	double average_score;
	double original_flat_error;	// squared

	std::vector<Node*> nodes;

	int state;
	int stage;
	int stage_iter;
	double sum_error;

	double new_state_factor;

	double best_sum_error;
	double tune_try;

	std::vector<int> curr_scope_sizes;
	std::vector<int> curr_s_input_sizes;
	FoldNetwork* curr_fold;
	std::vector<FoldNetwork*> curr_scope_input_folds;

	std::vector<int> test_scope_sizes;
	std::vector<int> test_s_input_sizes;
	FoldNetwork* test_fold;
	std::vector<FoldNetwork*> test_scope_input_folds;

	SmallNetwork* action_input_network;
	Scope* action;

	int new_layer_size;
	Network* obs_network;

	SubFoldNetwork* curr_score_network;
	double average_misguess;
	SubFoldNetwork* test_score_network;
	std::vector<int> score_input_layer;
	std::vector<int> score_input_sizes;
	std::vector<SmallNetwork*> score_input_networks;
	SmallNetwork* small_score_network;

	SubFoldNetwork* curr_compression_network;
	int compress_size;
	int compress_num_layers;
	int compress_new_size;
	std::vector<int> compressed_scope_sizes;
	std::vector<int> compressed_s_input_sizes;
	SubFoldNetwork* test_compression_network;
	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<SmallNetwork*> input_networks;
	Network* small_compression_network;

	Fold(std::string id,
		 int sequence_length,
		 std::vector<Scope*> compound_actions,
		 std::vector<int> obs_sizes,
		 double average_score,
		 double original_flat_error,
		 FoldNetwork* original_fold,
		 std::vector<FoldNetwork*> original_input_folds);
	~Fold();

	void process(std::vector<std::vector<double>>& flat_vals,
				 std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
				 double target_val);

private:
	void increment();

	void inner_scope_input_step(std::vector<std::vector<double>>& flat_vals,
								std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
								double target_val);
	void inner_scope_step(std::vector<std::vector<double>>& flat_vals,
						  std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
						  double target_val);
	void obs_step(std::vector<std::vector<double>>& flat_vals,
				  std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
				  double target_val);
	void score_step(std::vector<std::vector<double>>& flat_vals,
					std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
					double target_val);
	void score_input_step(std::vector<std::vector<double>>& flat_vals,
						  std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
						  double target_val);
	void score_small_step(std::vector<std::vector<double>>& flat_vals,
						  std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
						  double target_val);
	void score_tune_step(std::vector<std::vector<double>>& flat_vals,
						 std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
						 double target_val);
	void compress_step(std::vector<std::vector<double>>& flat_vals,
					   std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
					   double target_val);
	void compress_input_step(std::vector<std::vector<double>>& flat_vals,
							 std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
							 double target_val);
	void compress_small_step(std::vector<std::vector<double>>& flat_vals,
							 std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
							 double target_val);
	void final_tune_step(std::vector<std::vector<double>>& flat_vals,
						 std::vector<std::vector<std::vector<double>>>& inner_flat_vals,
						 double target_val);
};

#endif /* FOLD_H */