// TODO: loops not difficult to add, but will wait to switch problem to sorting for easier testing

#ifndef FOLD_H
#define FOLD_H

#include <fstream>
#include <vector>

#include "finished_step.h"
#include "fold_network.h"
#include "network.h"
#include "scope.h"

const int STATE_STARTING_COMPRESS = 0;

const int STATE_INNER_SCOPE_INPUT = 1;
// no fold step, and instead, simply transfer weights
const int STATE_SCORE = 2;	// adjust fold meanwhile as well
const int STATE_COMPRESS_STATE = 3;
const int STATE_COMPRESS_SCOPE = 4;
const int STATE_INPUT = 5;
const int STATE_STEP_ADDED = 6;	// for last_state bookkeeping

const int STATE_DONE = 7;

// don't split into LEARN and MEASURE stages, and instead combine

class FinishedStep;
class FoldHistory;
class Scope;
class Fold {
public:
	int id;

	int sequence_length;
	std::vector<bool> is_existing;
	std::vector<Scope*> existing_actions;	// shallow copy initialy, deep copy on adding FinishedStep
	std::vector<int> obs_sizes;
	int output_size;

	std::vector<FinishedStep*> finished_steps;

	int state;
	int last_state;	// for supporting explore_off_path
	int state_iter;
	double sum_error;
	double new_state_factor;

	double score_standard_deviation;
	double existing_misguess_standard_deviation;	// squared twice
	double* existing_average_score;
	double* existing_average_misguess;	// ref to branch end average_misguess

	double misguess_improvement;

	FoldNetwork* starting_score_network;
	FoldNetwork* combined_score_network;	// replace existing if already branch
	double combined_improvement;
	double replace_existing;
	double replace_combined;

	std::vector<Network*> scope_scale_mod_calcs;

	std::vector<int> curr_s_input_sizes;
	std::vector<int> curr_scope_sizes;
	FoldNetwork* curr_fold;
	// Note: input_folds don't care about current obs whereas fold and end_fold do
	std::vector<FoldNetwork*> curr_input_folds;
	FoldNetwork* curr_end_fold;	// becomes last compress network

	double starting_average_score;
	double starting_average_misguess;
	double starting_average_local_impact;

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
		 std::vector<bool> is_existing,
		 std::vector<Scope*> existing_actions,
		 std::vector<int> obs_sizes,
		 int output_size,
		 double* existing_average_score,
		 double* existing_average_misguess,
		 int starting_s_input_size,
		 int starting_state_size);
	Fold(Fold* original);
	Fold(std::ifstream& input_file);
	~Fold();

	void explore_on_path_activate(double existing_score,
								  std::vector<std::vector<double>>& flat_vals,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  FoldHistory* history);
	int explore_on_path_backprop(std::vector<double>& local_state_errors,
								 double& predicted_score,
								 double target_val,
								 double& scale_factor,
								 FoldHistory* history);
	void explore_off_path_activate(std::vector<std::vector<double>>& flat_vals,
								   double starting_score,
								   std::vector<double>& local_s_input_vals,
								   std::vector<double>& local_state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   int& explore_phase,
								   FoldHistory* history);
	void explore_off_path_backprop(std::vector<double>& local_s_input_errors,
								   std::vector<double>& local_state_errors,
								   double& predicted_score,
								   double target_val,
								   double& scale_factor,
								   FoldHistory* history);
	void existing_flat_activate(std::vector<std::vector<double>>& flat_vals,
								double starting_score,
								std::vector<double>& local_s_input_vals,
								std::vector<double>& local_state_vals,
								double& predicted_score,
								double& scale_factor,
								FoldHistory* history);
	void existing_flat_backprop(std::vector<double>& local_s_input_errors,
								std::vector<double>& local_state_errors,
								double& predicted_score,
								double predicted_score_error,
								double& scale_factor,
								double& scale_factor_error,
								FoldHistory* history);
	void update_activate(std::vector<std::vector<double>>& flat_vals,
						 double starting_score,
						 std::vector<double>& local_s_input_vals,
						 std::vector<double>& local_state_vals,
						 double& predicted_score,
						 double& scale_factor,
						 FoldHistory* history);
	void update_backprop(double& predicted_score,
						 double& next_predicted_score,
						 double target_val,
						 double& scale_factor,
						 FoldHistory* history);
	void existing_update_activate(std::vector<std::vector<double>>& flat_vals,
								  double starting_score,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  FoldHistory* history);
	void existing_update_backprop(double& predicted_score,
								  double predicted_score_error,
								  double& scale_factor,
								  double& scale_factor_error,
								  FoldHistory* history);

	void save(std::ofstream& output_file);

	void flat_step_explore_on_path_activate(double existing_score,
											std::vector<std::vector<double>>& flat_vals,
											std::vector<double>& local_s_input_vals,
											std::vector<double>& local_state_vals,
											double& predicted_score,
											double& scale_factor,
											FoldHistory* history);
	void flat_step_explore_on_path_backprop(std::vector<double>& local_state_errors,
											double& predicted_score,
											double target_val,
											double& scale_factor,
											FoldHistory* history);

	void flat_to_fold();

	void starting_compress_step_explore_off_path_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history);
	void starting_compress_step_explore_off_path_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void starting_compress_step_existing_flat_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void starting_compress_step_existing_flat_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);
	void starting_compress_step_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void starting_compress_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void starting_compress_step_existing_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void starting_compress_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);

	void starting_compress_end();

	void inner_scope_input_step_explore_off_path_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history);
	void inner_scope_input_step_explore_off_path_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void inner_scope_input_step_existing_flat_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void inner_scope_input_step_existing_flat_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);
	void inner_scope_input_step_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void inner_scope_input_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void inner_scope_input_step_existing_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void inner_scope_input_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);

	void inner_scope_input_end();

	void score_step_explore_off_path_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history);
	void score_step_explore_off_path_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void score_step_existing_flat_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void score_step_existing_flat_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);
	void score_step_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void score_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void score_step_existing_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void score_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);

	void score_end();

	void compress_step_explore_off_path_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history);
	void compress_step_explore_off_path_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void compress_step_existing_flat_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void compress_step_existing_flat_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);
	void compress_step_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void compress_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void compress_step_existing_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void compress_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);

	void compress_state_end();

	void compress_scope_end();

	void input_step_explore_off_path_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history);
	void input_step_explore_off_path_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void input_step_existing_flat_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void input_step_existing_flat_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);
	void input_step_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void input_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void input_step_existing_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void input_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);

	void input_end();

	void add_finished_step();
	void restart_from_finished_step();

	void step_added_step_explore_off_path_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history);
	void step_added_step_explore_off_path_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history);
	void step_added_step_existing_flat_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void step_added_step_existing_flat_backprop(
		std::vector<double>& local_s_input_errors,
		std::vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);
	void step_added_step_existing_update_activate(
		std::vector<std::vector<double>>& flat_vals,
		double starting_score,
		std::vector<double>& local_s_input_vals,
		std::vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history);
	void step_added_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history);

	void fold_increment();
};

class FinishedStepHistory;
class ScopeHistory;
class FoldHistory {
public:
	Fold* fold;

	double existing_score;

	double starting_score_update;
	double combined_score_update;

	FoldNetworkHistory* curr_starting_compress_network_history;

	std::vector<FinishedStepHistory*> finished_step_histories;

	std::vector<FoldNetworkHistory*> inner_input_input_network_histories;
	FoldNetworkHistory* curr_input_network_history;

	FoldNetworkHistory* curr_score_network_history;
	double score_update;

	FoldNetworkHistory* curr_compress_network_history;

	std::vector<FoldNetworkHistory*> input_network_histories;

	std::vector<FoldNetworkHistory*> curr_input_fold_histories;	// initialize to empty
	std::vector<ScopeHistory*> scope_histories;	// initialize to empty

	FoldNetworkHistory* curr_fold_history;
	FoldNetworkHistory* curr_end_fold_history;

	double ending_score_update;

	FoldHistory(Fold* fold);
	~FoldHistory();
};

#endif /* FOLD_H */