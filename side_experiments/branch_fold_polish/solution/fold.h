// TODO: loops not difficult to add, but will wait to switch problem to sorting for easier testing

#ifndef FOLD_H
#define FOLD_H

#include <vector>

#include "finished_step.h"
#include "fold_network.h"
#include "scope.h"

const int STATE_FLAT = -1;

const int STATE_STARTING_COMPRESS = 0;

const int STATE_INNER_SCOPE_INPUT = 1;
const int STATE_SCORE = 2;
const int STATE_SCORE_TUNE = 3;
const int STATE_COMPRESS_STATE = 4;
const int STATE_COMPRESS_SCOPE = 5;
const int STATE_INPUT = 6;
const int STATE_STEP_ADDED = 7;	// for last_state bookkeeping
// TODO: merge STATE_STEP_ADDED and STATE_FLAT

// don't split into LEARN and MEASURE stages, and instead combine

class Fold {
public:
	int sequence_length;
	std::vector<Scope*> existing_actions;
	std::vector<int> obs_sizes;
	int output_size;

	std::vector<FinishedStep*> finished_steps;

	int state;
	int last_state;	// for supporting explore_off_path
	int state_iter;
	double sum_error;

	double new_state_factor;

	FoldNetwork* starting_score_network;
	double replace_improvement;
	FoldNetwork* starting_compress_network;
	FoldNetwork* combined_score_network;	// replace existing if already branch
	double combined_improvement;

	std::vector<Network*> scope_scale_mod_calcs;
	Network* end_scale_mod_calc;
	// only need to modify scale as can't modify average at flat anyways

	std::vector<int> curr_s_input_sizes;
	std::vector<int> curr_scope_sizes;
	FoldNetwork* curr_fold;
	std::vector<FoldNetwork*> curr_input_folds;
	FoldNetwork* curr_end_fold;

	double average_misguess;
	double* existing_misguess;	// ref to branch end average_misguess

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

	FoldNetwork* curr_compress_network;
	int compress_size;
	int compress_num_layers;
	int compress_new_size;
	std::vector<int> compressed_s_input_sizes;
	std::vector<int> compressed_scope_sizes;
	FoldNetwork* test_compress_network;

	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<FoldNetwork*> input_networks;

	// TODO: need to be able to save Folds, even during partial progress
};

class FoldHistory {
public:
	double existing_score;

	double starting_score_update;
	double combined_score_update;

	std::vector<FoldNetworkHistory*> curr_input_fold_histories;
	std::vector<ScopeHistory*> scope_histories;

	FoldNetworkHistory* curr_fold_history;
	FoldNetworkHistory* curr_end_fold_history;

	double ending_score_update;

	std::vector<FinishedStepHistory*> finished_step_histories;
};

#endif /* FOLD_H */