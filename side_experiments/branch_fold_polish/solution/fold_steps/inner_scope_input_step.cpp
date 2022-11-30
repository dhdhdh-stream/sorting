#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::inner_scope_input_step_explore_off_path_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		int& explore_phase,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_explore_off_path_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double target_val,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_existing_flat_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_existing_flat_backprop(
		vector<double>& local_s_input_errors,
		vector<double>& local_state_errors,
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_update_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {
	// starting_score_network activated in branch
	history->starting_score_update = starting_score;
	predicted_score += starting_score;	// already scaled

	// starting compress for Folds will always be active_compress (vs. existing path which may not be)
	if (this->starting_compress_size > 0) {
		this->starting_compress_network->activate_small(local_s_input_vals,
														local_state_vals);
		int compress_new_size = (int)local_state_vals.size() - this->starting_compress_size;
		local_state_vals.clear();
		local_state_vals.reserve(compress_new_size);
		for (int s_index = 0; s_index < compress_new_size; s_index++) {
			local_state_vals.push_back(this->starting_compress_network->output->acti_vals[s_index]);
		}
	}

	vector<vector<double>> s_input_vals{local_s_input_vals};
	vector<vector<double>> state_vals{local_state_vals};

	for (int n_index = 0; n_index < (int)this->finished_steps.size(); n_index++) {
		FinishedStepHistory* finished_step_history = new FinishedStepHistory(this->finished_steps[n_index]);
		this->finished_steps[n_index]->update_activate(flat_vals,
													   s_input_vals,
													   state_vals,
													   predicted_score,
													   scale_factor,
													   finished_step_history);
		history->finished_step_histories[n_index] = finished_step_history;
	}



	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);


}

void Fold::inner_scope_input_step_update_backprop(
		double& predicted_score,
		double& next_predicted_score,
		double target_val,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_existing_update_activate(
		vector<vector<double>>& flat_vals,
		double starting_score,
		vector<double>& local_s_input_vals,
		vector<double>& local_state_vals,
		double& predicted_score,
		double& scale_factor,
		FoldHistory* history) {

}

void Fold::inner_scope_input_step_existing_update_backprop(
		double& predicted_score,
		double predicted_score_error,
		double& scale_factor,
		double& scale_factor_error,
		FoldHistory* history) {

}
