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
const int STATE_OBS_NEEDED = 2;
const int STATE_SCORE = 3;
const int STATE_SCORE_INPUT = 4;
const int STATE_SCORE_TUNE = 5;
const int STATE_COMPRESS_STATE = 6;
const int STATE_COMPRESS_SCOPE = 7;
const int STATE_COMPRESS_INPUT = 8;
const int STATE_DONE = 9;

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
	std::vector<double> scope_average_mods;
	std::vector<Network*> scope_scale_mod_calcs;
	std::vector<double> scope_scale_mods;

	Network* end_average_mod_calc;
	double end_average_mod;
	Network* end_scale_mod_calc;
	double end_scale_mod;

	std::vector<int> curr_scope_sizes;
	std::vector<int> curr_s_input_sizes;
	FoldNetwork* curr_fold;
	std::vector<FoldNetwork*> curr_scope_input_folds;
	FoldNetwork* curr_end_fold;

	std::vector<int> test_scope_sizes;
	std::vector<int> test_s_input_sizes;
	FoldNetwork* test_fold;
	std::vector<FoldNetwork*> test_scope_input_folds;
	FoldNetwork* curr_test_fold;
};

#endif /* FOLD_H */