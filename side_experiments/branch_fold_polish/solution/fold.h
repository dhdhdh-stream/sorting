#ifndef FOLD_H
#define FOLD_H

#include <vector>

#include "finished_step.h"
#include "fold_network.h"
#include "scope.h"

const int PHASE_FLAT = 0;
const int PHASE_FOLD = 1;

const int STATE_INNER_SCOPE_INPUT_INPUT = 0;
const int STATE_INNER_SCOPE_TUNE = 1;
const int STATE_SCORE = 2;
const int STATE_SCORE_TUNE = 3;
const int STATE_COMPRESS_STATE = 4;
const int STATE_COMPRESS_SCOPE = 5;
const int STATE_INPUT = 6;
const int STATE_DONE = 7;

const int STAGE_LEARN = 0;
const int STAGE_MEASURE = 1;

class Fold {
public:
	int phase;

	int sequence_length;
	std::vector<Scope*> existing_actions;
	std::vector<int> obs_sizes;
	int output_size;

	double original_flat_error;

	std::vector<FinishedStep*> finished_steps;

	int state;
	int stage;
	int stage_iter;
	double sum_error;

	double new_state_factor;

	FoldNetwork* starting_score_network;
	FoldNetwork* starting_compress_network;

	std::vector<Network*> scope_average_mod_calcs;
	std::vector<Network*> scope_scale_mod_calcs;

	Network* end_average_mod;
	Network* end_scale_mod;

	std::vector<int> curr_s_input_sizes;
	std::vector<int> curr_scope_sizes;
	FoldNetwork* curr_fold;	// after PHASE_FLAT, no longer need to backprop till end?
	std::vector<FoldNetwork*> curr_input_folds;
	FoldNetwork* curr_end_fold;

	std::vector<int> test_s_input_sizes;
	std::vector<int> test_scope_sizes;
	FoldNetwork* test_fold;
	std::vector<FoldNetwork*> test_input_folds;
	FoldNetwork* test_end_fold;

	FoldNetwork* curr_input_network;
	FoldNetwork* test_input_network;
	std::vector<int> inner_input_input_layer;
	std::vector<int> inner_input_input_sizes;
	std::vector<FoldNetwork*> inner_input_input_networks;

	FoldNetwork* curr_score_network;
	FoldNetwork* test_score_network;

	FoldNetwork* curr_compression_network;
	int compress_size;
	int compress_num_layers;
	int compress_new_size;
	std::vector<int> compressed_s_input_sizes;
	std::vector<int> compressed_scope_sizes;
	FoldNetwork* test_compression_network;

	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<FoldNetwork*> input_networks;

	// TODO: need to be able to save Folds, even during partial progress
};

class FoldHistory {
public:
	std::vector<ScopeHistory*> scope_histories;

	// don't worry about other histories for now (as they're unique)
};

#endif /* FOLD_H */