#include "refine_experiment.h"

#include "abstract_node.h"
#include "constants.h"
#include "eval_experiment.h"
#include "helpers.h"
#include "network.h"
#include "scope.h"
#include "signal_experiment.h"
#include "solution.h"
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

void RefineExperiment::check_activate(AbstractNode* experiment_node,
									  bool is_branch,
									  SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		RefineExperimentHistory* history;
		map<RefineExperiment*, RefineExperimentHistory*>::iterator it =
			wrapper->refine_histories.find(this);
		if (it == wrapper->refine_histories.end()) {
			history = new RefineExperimentHistory();
			wrapper->refine_histories[this] = history;
		} else {
			history = it->second;
		}

		ScopeHistory* scope_history = wrapper->scope_histories.back();

		if (history->is_on) {
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

			vector<double> curr_new_network_vals(this->new_network_inputs.size());
			vector<bool> curr_new_network_is_on(this->new_network_inputs.size());
			if (this->new_network != NULL) {
				for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
					double val;
					bool is_on;
					fetch_input_helper(scope_history,
									   this->new_network_inputs[i_index],
									   0,
									   val,
									   is_on);
					curr_new_network_vals[i_index] = val;
					curr_new_network_is_on[i_index] = is_on;
				}
				this->new_network->activate(curr_new_network_vals,
											curr_new_network_is_on);
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
				vector<double> curr_existing_factor_vals(this->existing_inputs.size());
				for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
					double val;
					bool is_on;
					fetch_input_helper(scope_history,
									   this->existing_inputs[i_index],
									   0,
									   val,
									   is_on);
					if (is_on) {
						double normalized_val = (val - this->existing_input_averages[i_index]) / this->existing_input_standard_deviations[i_index];
						curr_existing_factor_vals[i_index] = normalized_val;
					} else {
						curr_existing_factor_vals[i_index] = 0.0;
					}
				}

				vector<double> curr_existing_network_vals(this->existing_network_inputs.size());
				vector<bool> curr_existing_network_is_on(this->existing_network_inputs.size());
				for (int i_index = 0; i_index < (int)this->existing_network_inputs.size(); i_index++) {
					double val;
					bool is_on;
					fetch_input_helper(scope_history,
									   this->existing_network_inputs[i_index],
									   0,
									   val,
									   is_on);
					curr_existing_network_vals[i_index] = val;
					curr_existing_network_is_on[i_index] = is_on;
				}

				history->sum_signal_vals.push_back(0.0);
				history->sum_counts.push_back(0.0);

				for (int i_index = 0; i_index < (int)wrapper->scope_histories.size(); i_index++) {
					Scope* scope = wrapper->scope_histories[i_index]->scope;
					if (scope->pre_default_signal != NULL) {
						if (scope->signal_experiment_history == NULL
								|| !scope->signal_experiment_history->is_on) {
							wrapper->scope_histories[i_index]->refine_experiment_callbacks
								.push_back(history);
							wrapper->scope_histories[i_index]->refine_experiment_instance_indexes
								.push_back((int)history->sum_signal_vals.size()-1);
						}
					}
				}

				history->new_factor_vals.push_back(curr_new_factor_vals);
				history->new_network_vals.push_back(curr_new_network_vals);
				history->new_network_is_on.push_back(curr_new_network_is_on);

				history->existing_factor_vals.push_back(curr_existing_factor_vals);
				history->existing_network_vals.push_back(curr_existing_network_vals);
				history->existing_network_is_on.push_back(curr_existing_network_is_on);
			} else {
				this->new_factor_vals.push_back(curr_new_factor_vals);
				this->new_network_vals.push_back(curr_new_network_vals);
				this->new_network_is_on.push_back(curr_new_network_is_on);
				this->new_target_val_status.push_back(STATUS_TYPE_DONE);
				this->new_target_vals.push_back(new_sum_vals);
				this->new_existing_factor_vals.push_back(vector<double>());
				this->new_existing_network_vals.push_back(vector<double>());
				this->new_existing_network_is_on.push_back(vector<bool>());
			}
		} else {
			vector<double> curr_existing_factor_vals(this->existing_inputs.size());
			for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_history,
								   this->existing_inputs[i_index],
								   0,
								   val,
								   is_on);
				if (is_on) {
					double normalized_val = (val - this->existing_input_averages[i_index]) / this->existing_input_standard_deviations[i_index];
					curr_existing_factor_vals[i_index] = normalized_val;
				} else {
					curr_existing_factor_vals[i_index] = 0.0;
				}
			}

			vector<double> curr_existing_network_vals(this->existing_network_inputs.size());
			vector<bool> curr_existing_network_is_on(this->existing_network_inputs.size());
			for (int i_index = 0; i_index < (int)this->existing_network_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_history,
								   this->existing_network_inputs[i_index],
								   0,
								   val,
								   is_on);
				curr_existing_network_vals[i_index] = val;
				curr_existing_network_is_on[i_index] = is_on;
			}

			history->sum_signal_vals.push_back(0.0);
			history->sum_counts.push_back(0.0);

			for (int i_index = 0; i_index < (int)wrapper->scope_histories.size(); i_index++) {
				Scope* scope = wrapper->scope_histories[i_index]->scope;
				if (scope->pre_default_signal != NULL) {
					if (scope->signal_experiment_history == NULL
							|| !scope->signal_experiment_history->is_on) {
						wrapper->scope_histories[i_index]->refine_experiment_callbacks
							.push_back(history);
						wrapper->scope_histories[i_index]->refine_experiment_instance_indexes
							.push_back((int)history->sum_signal_vals.size()-1);
					}
				}
			}

			history->existing_factor_vals.push_back(curr_existing_factor_vals);
			history->existing_network_vals.push_back(curr_existing_network_vals);
			history->existing_network_is_on.push_back(curr_existing_network_is_on);
		}
	}
}

void RefineExperiment::experiment_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   bool& fetch_action,
									   SolutionWrapper* wrapper) {
	RefineExperimentState* experiment_state = (RefineExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void RefineExperiment::set_action(int action,
								  SolutionWrapper* wrapper) {
	// do nothing
}

void RefineExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	RefineExperimentState* experiment_state = (RefineExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void RefineExperiment::backprop(double target_val,
								RefineExperimentHistory* history,
								SolutionWrapper* wrapper) {
	if (history->is_on) {
		double average_score = wrapper->solution->sum_scores / (double)HISTORIES_NUM_SAVE;
		for (int i_index = 0; i_index < (int)history->sum_signal_vals.size(); i_index++) {
			history->sum_signal_vals[i_index] += (target_val - average_score);
			history->sum_counts[i_index]++;

			double average_val = history->sum_signal_vals[i_index]
				/ (double)history->sum_counts[i_index];

			this->new_factor_vals.push_back(history->new_factor_vals[i_index]);
			this->new_network_vals.push_back(history->new_network_vals[i_index]);
			this->new_network_is_on.push_back(history->new_network_is_on[i_index]);
			this->new_target_val_status.push_back(STATUS_TYPE_NEED_VS);
			this->new_target_vals.push_back(average_val);
			this->new_existing_factor_vals.push_back(history->existing_factor_vals[i_index]);
			this->new_existing_network_vals.push_back(history->existing_network_vals[i_index]);
			this->new_existing_network_is_on.push_back(history->existing_network_is_on[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= NUM_REFINE_ITERS) {
			calc_vs();
			bool is_success = refine();
			if (is_success) {
				this->num_refines++;
				if (this->num_refines >= NUM_REFINE_CYCLES) {
					EvalExperiment* new_eval_experiment = new EvalExperiment();

					new_eval_experiment->node_context = this->node_context;
					new_eval_experiment->is_branch = this->is_branch;
					new_eval_experiment->exit_next_node = this->exit_next_node;

					new_eval_experiment->select_percentage = this->select_percentage;

					new_eval_experiment->new_constant = this->new_constant;
					new_eval_experiment->new_inputs = this->new_inputs;
					new_eval_experiment->new_input_averages = this->new_input_averages;
					new_eval_experiment->new_input_standard_deviations = this->new_input_standard_deviations;
					new_eval_experiment->new_weights = this->new_weights;
					new_eval_experiment->new_network_inputs = this->new_network_inputs;
					new_eval_experiment->new_network = this->new_network;
					this->new_network = NULL;

					new_eval_experiment->new_scope = this->new_scope;
					this->new_scope = NULL;
					new_eval_experiment->step_types = this->step_types;
					new_eval_experiment->actions = this->actions;
					new_eval_experiment->scopes = this->scopes;

					this->node_context->experiment = new_eval_experiment;
					delete this;
				} else {
					this->existing_factor_vals.clear();
					this->existing_network_vals.clear();
					this->existing_network_is_on.clear();
					this->existing_target_vals.clear();

					this->state_iter = 0;
				}
			} else {
				this->node_context->experiment = NULL;
				delete this;
			}
		}
	} else {
		double average_score = wrapper->solution->sum_scores / (double)HISTORIES_NUM_SAVE;
		for (int i_index = 0; i_index < (int)history->sum_signal_vals.size(); i_index++) {
			history->sum_signal_vals[i_index] += (target_val - average_score);
			history->sum_counts[i_index]++;

			double average_val = history->sum_signal_vals[i_index]
				/ (double)history->sum_counts[i_index];

			this->existing_factor_vals.push_back(history->existing_factor_vals[i_index]);
			this->existing_network_vals.push_back(history->existing_network_vals[i_index]);
			this->existing_network_is_on.push_back(history->existing_network_is_on[i_index]);
			this->existing_target_vals.push_back(average_val);
		}
	}
}
