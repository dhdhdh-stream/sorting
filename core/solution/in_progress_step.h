#ifndef IN_PROGRESS_STEP_H
#define IN_PROGRESS_STEP_H

#include <vector>

#include "finished_step.h"
#include "fold_network.h"
#include "network.h"
#include "small_network.h"
#include "scope.h"
#include "sub_fold_network.h"

const int STAGE_LEARN = 0;
const int STAGE_MEASURE = 1;

const int STATE_INNER_SCOPE_INPUT = 0;

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

class InProgressStep {
public:
	int obs_size;	// can be >1 for compound actions
	double original_flat_error;	// squared

	int state;
	int stage;
	int stage_iter;
	double sum_error;

	double new_state_factor;

	double best_sum_error;
	double tune_try;

	// first layer is from outer, so don't compress to 0
	std::vector<int> curr_scope_sizes;
	std::vector<int> curr_s_input_sizes;
	FoldNetwork* curr_fold;

	std::vector<int> test_scope_sizes;
	std::vector<int> test_s_input_sizes;
	FoldNetwork* test_fold;

	int new_layer_size;
	// TODO: add back obs_network
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
	std::vector<int> compressed_s_input_sizes;	// layer 0 equals original s_input_size, but doesn't matter if compress_new_size > 0
	SubFoldNetwork* test_compression_network;
	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<SmallNetwork*> input_networks;
	Network* small_compression_network;

	InProgressStep(std::vector<int> initial_scope_sizes,
				   std::vector<int> initial_s_input_sizes,
				   FoldNetwork* original_fold,
				   int obs_size,
				   double original_flat_error);
	~InProgressStep();

	void activate(std::vector<std::vector<double>>& state_vals,
				  std::vector<std::vector<double>>& s_input_vals,
				  std::vector<double>& obs,
				  double& predicted_score);
	void process(std::vector<std::vector<double>>& flat_inputs,
				 std::vector<std::vector<double>>& state_vals,
				 std::vector<std::vector<double>>& s_input_vals,
				 double& predicted_score,
				 double target_val,
				 std::vector<FinishedStep*>& steps);

private:
	void increment();
};

#endif /* IN_PROGRESS_STEP_H */