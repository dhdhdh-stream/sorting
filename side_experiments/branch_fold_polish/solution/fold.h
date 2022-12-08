// TODO: loops not difficult to add, but will wait to switch problem to sorting for easier testing

#ifndef FOLD_H
#define FOLD_H

#include <vector>

#include "finished_step.h"
#include "fold_network.h"
#include "scope.h"

const int STATE_STARTING_COMPRESS = 0;

const int STATE_INNER_SCOPE_INPUT = 1;
// no fold step, and instead, simply transfer weights
const int STATE_SCORE = 2;	// adjust fold meanwhile as well
const int STATE_COMPRESS_STATE = 3;
const int STATE_COMPRESS_SCOPE = 4;
const int STATE_INPUT = 5;
const int STATE_STEP_ADDED = 6;	// for last_state bookkeeping

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

	double average_misguess;
	double* existing_misguess;	// ref to branch end average_misguess
	double average_misguess_standard_deviation;	// assume equal standard deviation for existing

	FoldNetwork* starting_score_network;
	double replace_existing;
	double replace_existing_standard_deviation;
	double replace_combined;
	double replace_combined_standard_deviation;
	FoldNetwork* combined_score_network;	// replace existing if already branch
	double combined_improvement;
	double combined_standard_deviation;

	std::vector<Network*> scope_scale_mod_calcs;
	Network* end_scale_mod_calc;	// can drop if replace
	// only need to modify scale as can't modify average at flat anyways

	std::vector<int> curr_s_input_sizes;
	std::vector<int> curr_scope_sizes;
	FoldNetwork* curr_fold;
	// Note: input_folds don't care about current obs whereas fold and end_fold do
	std::vector<FoldNetwork*> curr_input_folds;
	FoldNetwork* curr_end_fold;	// becomes last compress network

	int curr_starting_compress_new_size;
	FoldNetwork* curr_starting_compress_network;
	int starting_compress_original_size;
	int test_starting_compress_new_size;
	FoldNetwork* test_starting_compress_network;

	std::vector<int> test_s_input_sizes;
	std::vector<int> test_scope_sizes;
	FoldNetwork* test_fold;
	std::vector<FoldNetwork*> test_input_folds;
	FoldNetwork* test_end_fold;

	// don't need extra test fields as curr_input_network and test_input_network use different layers
	std::vector<int> inner_input_input_layer;
	std::vector<int> inner_input_input_sizes;
	std::vector<FoldNetwork*> inner_input_input_networks;
	FoldNetwork* curr_input_network;
	FoldNetwork* test_input_network;

	FoldNetwork* curr_score_network;
	FoldNetwork* test_score_network;

	FoldNetwork* curr_compress_network;
	int curr_compress_num_layers;
	int curr_compress_new_size;
	int curr_compress_original_size;	// for constructing scope
	std::vector<int> curr_compressed_s_input_sizes;
	std::vector<int> curr_compressed_scope_sizes;
	
	FoldNetwork* test_compress_network;
	int test_compress_num_layers;
	int test_compress_new_size;
	int test_compress_original_size;
	std::vector<int> test_compressed_s_input_sizes;
	std::vector<int> test_compressed_scope_sizes;

	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<FoldNetwork*> input_networks;

	Fold(int sequence_length,
		 std::vector<Scope*> existing_actions,
		 std::vector<int> obs_sizes,
		 int output_size,
		 double* existing_misguess,
		 int starting_s_input_size,
		 int starting_state_size);	// super edge case but can be 0 if scope num_outputs is 0

	// TODO: need to be able to save Folds, even during partial progress
};

class FoldHistory {
public:
	double existing_score;

	double starting_score_update;
	double combined_score_update;

	FoldNetworkHistory* curr_starting_compress_network_history;

	std::vector<FinishedStepHistory*> finished_step_histories;

	std::vector<FoldNetworkHistory*> inner_input_input_network_histories;
	FoldNetworkHistory* curr_input_network_history;

	FoldNetworkHistory* curr_score_network_history;

	FoldNetworkHistory* curr_compress_network_history;

	std::vector<FoldNetworkHistory*> curr_input_fold_histories;
	std::vector<ScopeHistory*> scope_histories;

	FoldNetworkHistory* curr_fold_history;
	FoldNetworkHistory* curr_end_fold_history;

	double ending_score_update;
};

#endif /* FOLD_H */