#include "loop_fold.h"

using namespace std;

LoopFold::LoopFold(vector<int> scope_context,
				   vector<int> node_context,
				   int sequence_length,
				   vector<bool> is_inner_scope,
				   vector<int> existing_scope_ids,
				   double* existing_average_score,
				   double* existing_score_variance,
				   double* existing_average_misguess,
				   double* existing_misguess_variance) {

}

LoopFold::LoopFold(ifstream& input_file,
				   int scope_id,
				   int scope_index) {

}

LoopFold::~LoopFold() {

}

void LoopFold::experiment_activate(vector<double>& local_state_vals,
								   vector<double>& input_vals,
								   vector<vector<double>>& flat_vals,
								   double& predicted_score,
								   double& scale_factor,
								   vector<ScopeHistory*>& context_histories,
								   RunHelper& run_help,
								   FoldHistory* history) {
	if (this->sub_state == LOOP_FOLD_SUB_STATE_LEARN) {
		learn_activate(local_state_vals,
					   input_vals,
					   flat_vals,
					   predicted_score,
					   scale_factor,
					   context_histories,
					   run_help,
					   history);
	} else {
		// this->sub_state == LOOP_FOLD_SUB_STATE_MEASURE
		measure_activate(local_state_vals,
						 input_vals,
						 flat_vals,
						 predicted_score,
						 scale_factor,
						 context_histories,
						 run_help,
						 history);
	}
}

void LoopFold::experiment_backprop(vector<double>& local_state_errors,
								   vector<double>& input_errors,
								   double target_val,
								   double final_misguess,
								   double& predicted_score,
								   double& scale_factor,
								   RunHelper& run_helper,
								   FoldHistory* history) {
	if (this->sub_state == LOOP_FOLD_STATE_LEARN) {
		learn_backprop(local_state_errors,
					   input_errors,
					   target_val,
					   final_misguess,
					   predicted_score,
					   scale_factor,
					   run_helper,
					   history);
	} else {
		// this->sub_state == LOOP_FOLD_STATE_MEASURE
		measure_backprop(local_state_errors,
						 input_errors,
						 target_val,
						 final_misguess,
						 predicted_score,
						 scale_factor,
						 run_helper,
						 history);
	}

	explore_increment();
}

void LoopFold::explore_increment() {
	this->state_iter++;

	if (this->state == LOOP_FOLD_STATE_EXPERIMENT
			&& this->sub_state == LOOP_FOLD_SUB_STATE_LEARN) {
		if (this->state_iter == 500000) {
			this->sub_state = LOOP_FOLD_SUB_STATE_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == LOOP_FOLD_STATE_EXPERIMENT
			&& this->sub_state == LOOP_FOLD_SUB_STATE_MEASURE) {
		if (this->state_iter == 10000) {
			explore_end();
		}
	} else if (this->state == LOOP_FOLD_STATE_ADD_OUTER_STATE
			&& this->sub_state == LOOP_FOLD_SUB_STATE_LEARN) {
		if (this->state_iter == 500000) {
			this->sub_state = LOOP_FOLD_SUB_STATE_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == LOOP_FOLD_STATE_ADD_OUTER_STATE
			&& this->sub_state == LOOP_FOLD_SUB_STATE_MEASURE) {
		if (this->state_iter == 10000) {
			add_outer_state_end();
		}
	} else if (this->state == LOOP_FOLD_STATE_ADD_INNER_STATE
			&& this->sub_state == LOOP_FOLD_SUB_STATE_LEARN) {
		if (this->state_iter == 500000) {
			this->sub_state = LOOP_FOLD_SUB_STATE_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else {
		// this->state == LOOP_FOLD_STATE_ADD_INNER_STATE && this->sub_state == LOOP_FOLD_SUB_STATE_MEASURE
		if (this->state_iter == 10000) {
			add_inner_state_end();
		}
	}
}
