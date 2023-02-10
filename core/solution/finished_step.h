#ifndef FINISHED_STEP_H
#define FINISHED_STEP_H

#include <fstream>
#include <vector>

#include "fold_network.h"
#include "network.h"
#include "problem.h"
#include "run_status.h"
#include "scope.h"

class FinishedStepHistory;
class Fold;
class Scope;
class FinishedStep {
public:
	int id;

	bool is_inner_scope;
	Scope* scope;
	Action action;

	std::vector<int> inner_input_input_layer;
	std::vector<int> inner_input_input_sizes;
	std::vector<FoldNetwork*> inner_input_input_networks;
	FoldNetwork* inner_input_network;
	Network* scope_scale_mod;

	FoldNetwork* score_network;
	FoldNetwork* confidence_network; 

	double average_inner_scope_impact;
	double average_local_impact;

	int compress_num_layers;
	bool active_compress;
	int compress_new_size;
	FoldNetwork* compress_network;
	int compress_original_size;	// for constructing scope

	std::vector<int> compressed_s_input_sizes;	// earliest to latest
	std::vector<int> compressed_scope_sizes;

	std::vector<int> input_layer;
	std::vector<int> input_sizes;
	std::vector<FoldNetwork*> input_networks;

	FinishedStep(bool is_inner_scope,
				 Scope* scope,
				 Action action,
				 std::vector<int> inner_input_input_layer,
				 std::vector<int> inner_input_input_sizes,
				 std::vector<FoldNetwork*> inner_input_input_networks,
				 FoldNetwork* inner_input_network,
				 Network* scope_scale_mod,
				 FoldNetwork* score_network,
				 FoldNetwork* confidence_network,
				 int compress_num_layers,
				 int compress_new_size,
				 FoldNetwork* compress_network,
				 int compress_original_size,
				 std::vector<int> compressed_s_input_sizes,
				 std::vector<int> compressed_scope_sizes,
				 std::vector<int> input_layer,
				 std::vector<int> input_sizes,
				 std::vector<FoldNetwork*> input_networks);
	FinishedStep(std::ifstream& input_file);
	~FinishedStep();

	void explore_off_path_activate(Problem& problem,
								   std::vector<std::vector<double>>& s_input_vals,
								   std::vector<std::vector<double>>& state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   RunStatus& run_status,
								   FinishedStepHistory* history);
	void explore_off_path_backprop(std::vector<std::vector<double>>& s_input_errors,
								   std::vector<std::vector<double>>& state_errors,
								   double& predicted_score,
								   double target_val,
								   double& scale_factor,
								   double& scale_factor_error,
								   FinishedStepHistory* history);
	void existing_flat_activate(Problem& problem,
								std::vector<std::vector<double>>& s_input_vals,
								std::vector<std::vector<double>>& state_vals,
								double& predicted_score,
								double& scale_factor,
								RunStatus& run_status,
								FinishedStepHistory* history);
	void existing_flat_backprop(std::vector<std::vector<double>>& s_input_errors,
								std::vector<std::vector<double>>& state_errors,
								double& predicted_score,
								double predicted_score_error,
								double& scale_factor,
								double& scale_factor_error,
								FinishedStepHistory* history);
	void update_activate(Problem& problem,
						 std::vector<std::vector<double>>& s_input_vals,
						 std::vector<std::vector<double>>& state_vals,
						 double& predicted_score,
						 double& scale_factor,
						 RunStatus& run_status,
						 FinishedStepHistory* history);
	void update_backprop(double& predicted_score,
						 double target_val,
						 double final_misguess,
						 double& scale_factor,
						 double& scale_factor_error,
						 FinishedStepHistory* history);
	void existing_update_activate(Problem& problem,
								  std::vector<std::vector<double>>& s_input_vals,
								  std::vector<std::vector<double>>& state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  RunStatus& run_status,
								  FinishedStepHistory* history);
	void existing_update_backprop(double& predicted_score,
								  double predicted_score_error,
								  double& scale_factor,
								  double& scale_factor_error,
								  FinishedStepHistory* history);

	void update_increment(FinishedStepHistory* history,
						  std::vector<Fold*>& folds_to_delete);

	void save(std::ofstream& output_file);
};

class ScopeHistory;
class FinishedStepHistory {
public:
	FinishedStep* finished_step;

	std::vector<FoldNetworkHistory*> inner_input_input_network_histories;
	FoldNetworkHistory* inner_input_network_history;

	ScopeHistory* scope_history;

	FoldNetworkHistory* score_network_history;
	double score_update;
	FoldNetworkHistory* confidence_network_history;
	double confidence_network_output;

	FoldNetworkHistory* compress_network_history;

	std::vector<FoldNetworkHistory*> input_network_histories;

	int exit_location;

	FinishedStepHistory(FinishedStep* finished_step);
	~FinishedStepHistory();
};

#endif /* FINISHED_STEP_H */