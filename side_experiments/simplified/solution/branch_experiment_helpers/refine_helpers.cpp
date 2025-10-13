#include "branch_experiment.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_REFINE_ITERS = 10;
const int NUM_REFINE_CYCLES = 2;
#else
const int NUM_REFINE_ITERS = 200;
const int NUM_REFINE_CYCLES = 4;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int OPTIMIZE_ITERS = 10;
#else
const int OPTIMIZE_ITERS = 100000;
#endif /* MDEBUG */

void BranchExperiment::refine_check_activate(SolutionWrapper* wrapper,
											 BranchExperimentHistory* history) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	double existing_sum_vals = this->existing_constant;
	for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
		double val;
		bool is_on;
		fetch_input_helper(wrapper->scope_histories.back(),
						   this->existing_inputs[i_index],
						   0,
						   val,
						   is_on);
		if (is_on) {
			double normalized_val = (val - this->existing_input_averages[i_index]) / this->existing_input_standard_deviations[i_index];
			existing_sum_vals += this->existing_weights[i_index] * normalized_val;
		}
	}
	if (this->existing_network != NULL) {
		vector<double> input_vals(this->existing_network_inputs.size());
		vector<bool> input_is_on(this->existing_network_inputs.size());
		for (int i_index = 0; i_index < (int)this->existing_network_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(wrapper->scope_histories.back(),
							   this->existing_network_inputs[i_index],
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		this->existing_network->activate(input_vals,
									input_is_on);
		existing_sum_vals += this->existing_network->output->acti_vals[0];
	}

	double new_sum_vals = this->new_constant;

	vector<double> curr_new_factor_vals(this->new_inputs.size());
	for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_history,
						   this->new_inputs[i_index],
						   0,
						   val,
						   is_on);
		if (is_on) {
			double normalized_val = (val - this->new_input_averages[i_index]) / this->new_input_standard_deviations[i_index];
			new_sum_vals += this->new_weights[i_index] * normalized_val;

			curr_new_factor_vals[i_index] = normalized_val;
		} else {
			curr_new_factor_vals[i_index] = 0.0;
		}
	}

	vector<double> input_vals(this->new_network_inputs.size());
	vector<bool> input_is_on(this->new_network_inputs.size());
	if (this->new_network != NULL) {
		for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   this->new_network_inputs[i_index],
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		this->new_network->activate(input_vals,
									input_is_on);
		new_sum_vals += this->new_network->output->acti_vals[0];
	}

	bool decision_is_branch;
	if (this->select_percentage == 1.0) {
		decision_is_branch = true;
	} else {
		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#else
		if (new_sum_vals >= 0.0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		#endif /* MDEBUG */
	}

	if (decision_is_branch) {
		history->existing_predicted_scores.push_back(existing_sum_vals);

		history->factor_vals.push_back(curr_new_factor_vals);
		history->network_vals.push_back(input_vals);
		history->network_is_on.push_back(input_is_on);

		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	} else {
		/**
		 * - to make train distribution still representative of true distribution
		 */

		this->target_val_histories.push_back(new_sum_vals);

		this->factor_vals.push_back(curr_new_factor_vals);
		this->network_vals.push_back(input_vals);
		this->network_is_on.push_back(input_is_on);
	}
}

void BranchExperiment::refine_step(vector<double>& obs,
								   int& action,
								   bool& is_next,
								   SolutionWrapper* wrapper,
								   BranchExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void BranchExperiment::refine_exit_step(SolutionWrapper* wrapper,
										BranchExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

bool BranchExperiment::refine_helper() {
	Eigen::MatrixXd inputs(this->target_val_histories.size(), 1 + this->new_inputs.size());
	Eigen::VectorXd outputs(this->target_val_histories.size());
	uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
	/**
	 * - add some noise to prevent extremes
	 */
	for (int i_index = 0; i_index < (int)this->target_val_histories.size(); i_index++) {
		inputs(i_index, 0) = 1.0;
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			inputs(i_index, 1 + f_index) = this->factor_vals[i_index][f_index]
					+ noise_distribution(generator);
		}
		outputs(i_index) = this->target_val_histories[i_index];
	}

	bool train_factor_success = true;

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		train_factor_success = false;
	}

	#if defined(MDEBUG) && MDEBUG
	#else
	if (train_factor_success) {
		if (abs(weights(0)) > REGRESSION_WEIGHT_LIMIT) {
			cout << "abs(weights(0)): " << abs(weights(0)) << endl;
			train_factor_success = false;
		}
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			if (abs(weights(1 + f_index)) > REGRESSION_WEIGHT_LIMIT) {
				cout << "abs(weights(1 + f_index)): " << abs(weights(1 + f_index)) << endl;
				train_factor_success = false;
			}
		}
	}
	#endif /* MDEBUG */

	if (train_factor_success) {
		this->new_constant = weights(0);
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			this->new_weights[f_index] = weights(1 + f_index);
		}
	}

	vector<double> sum_vals(this->target_val_histories.size());
	vector<double> remaining_scores(this->target_val_histories.size());
	for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
		double sum_score = this->new_constant;
		for (int f_index = 0; f_index < (int)this->new_inputs.size(); f_index++) {
			sum_score += this->new_weights[f_index] * this->factor_vals[h_index][f_index];
		}

		sum_vals[h_index] = sum_score;
		remaining_scores[h_index] = this->target_val_histories[h_index] - sum_score;
	}

	if (this->new_network != NULL) {
		uniform_int_distribution<int> input_distribution(0, this->target_val_histories.size()-1);
		uniform_int_distribution<int> drop_distribution(0, 9);
		for (int iter_index = 0; iter_index < OPTIMIZE_ITERS; iter_index++) {
			int rand_index = input_distribution(generator);

			vector<bool> w_drop(this->new_network_inputs.size());
			for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
				if (drop_distribution(generator) == 0) {
					w_drop[i_index] = false;
				} else {
					w_drop[i_index] = this->network_is_on[rand_index][i_index];
				}
			}

			this->new_network->activate(this->network_vals[rand_index],
										w_drop);
			double error = remaining_scores[rand_index] - this->new_network->output->acti_vals[0];
			this->new_network->backprop(error);
		}

		for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
			this->new_network->activate(this->network_vals[h_index],
										this->network_is_on[h_index]);
			sum_vals[h_index] += this->new_network->output->acti_vals[0];
		}
	}

	double sum_predicted_score = 0.0;
	for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
		if (sum_vals[h_index] >= 0.0) {
			sum_predicted_score += this->target_val_histories[h_index];
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (rand()%2 == 0) {
		this->select_percentage = 0.5;
	} else {
		this->select_percentage = 0.0;
	}
	#else
	int num_positive = 0;
	for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
		if (sum_vals[h_index] >= 0.0) {
			num_positive++;
		}
	}
	this->select_percentage = (double)num_positive / (double)this->target_val_histories.size();
	#endif /* MDEBUG */

	// // temp
	// cout << "refine" << endl;
	// cout << "sum_predicted_score: " << sum_predicted_score << endl;
	// cout << "this->select_percentage: " << this->select_percentage << endl;

	#if defined(MDEBUG) && MDEBUG
	if ((sum_predicted_score >= 0.0
			&& this->select_percentage > 0.0) || rand()%2 == 0) {
	#else
	if (sum_predicted_score >= 0.0
			&& this->select_percentage > 0.0) {
	#endif /* MDEBUG */
		return true;
	} else {
		return false;
	}
}

void BranchExperiment::refine_backprop(double target_val,
									   BranchExperimentHistory* history) {
	if (history->is_hit) {
		for (int h_index = 0; h_index < (int)history->existing_predicted_scores.size(); h_index++) {
			this->target_val_histories.push_back(target_val - history->existing_predicted_scores[h_index]);
			this->factor_vals.push_back(history->factor_vals[h_index]);
			this->network_vals.push_back(history->network_vals[h_index]);
			this->network_is_on.push_back(history->network_is_on[h_index]);
		}

		this->state_iter++;
		if (this->state_iter >= NUM_REFINE_ITERS) {
			bool is_success = refine_helper();
			if (is_success) {
				this->num_refines++;
				if (this->num_refines >= NUM_REFINE_CYCLES) {
					this->new_sum_scores = 0.0;

					this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
					this->state_iter = 0;
				} else {
					this->state_iter = 0;
				}
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
