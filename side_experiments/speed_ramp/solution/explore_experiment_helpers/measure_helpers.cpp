/**
 * - existing with experiment while new without
 *   - so not perfect comparison
 * 
 * TODO: perhaps allow one pure explore and one measure explore
 */

#include "explore_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "eval_experiment.h"
#include "helpers.h"
#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_SAMPLES = 2;
#else
const int MEASURE_NUM_SAMPLES = 100;
#endif /* MDEBUG */

void ExploreExperiment::measure_check_activate(SolutionWrapper* wrapper,
											   ExploreExperimentHistory* history) {
	if (history->is_on) {
		wrapper->has_explore = true;

		ScopeHistory* scope_history = wrapper->scope_histories.back();

		if (this->select_percentage == 1.0) {
			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		} else {
			double sum_vals = this->new_constant;

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
					sum_vals += this->new_weights[i_index] * normalized_val;
				}
			}

			if (this->new_network != NULL) {
				vector<double> input_vals(this->new_network_inputs.size());
				vector<bool> input_is_on(this->new_network_inputs.size());
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
				sum_vals += this->new_network->output->acti_vals[0];
			}

			bool decision_is_branch;
			#if defined(MDEBUG) && MDEBUG
			if (wrapper->curr_run_seed%2 == 0) {
				decision_is_branch = true;
			} else {
				decision_is_branch = false;
			}
			wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
			#else
			if (sum_vals >= 0.0) {
				decision_is_branch = true;
			} else {
				decision_is_branch = false;
			}
			#endif /* MDEBUG */

			if (decision_is_branch) {
				ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
				new_experiment_state->step_index = 0;
				wrapper->experiment_context.back() = new_experiment_state;
			}
		}
	}
}

void ExploreExperiment::measure_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 SolutionWrapper* wrapper,
									 ExploreExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

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

void ExploreExperiment::measure_exit_step(SolutionWrapper* wrapper,
										  ExploreExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::measure_backprop(double target_val,
										 ExploreExperimentHistory* history,
										 SolutionWrapper* wrapper) {
	if (history->is_on) {
		this->new_sum_scores += target_val;
		this->new_count++;
	} else {
		this->existing_sum_scores += target_val;
		this->existing_count++;
	}

	if (this->new_count >= MEASURE_NUM_SAMPLES) {
		double existing_score_average = this->existing_sum_scores / (double)this->existing_count;
		double new_score_average = this->new_sum_scores / (double)this->new_count;
		#if defined(MDEBUG) && MDEBUG
		if (new_score_average >= existing_score_average || rand()%3 != 0) {
		#else
		if (new_score_average >= existing_score_average) {
		#endif /* MDEBUG */
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

			new_eval_experiment->new_scope = this->best_new_scope;
			this->best_new_scope = NULL;
			new_eval_experiment->step_types = this->best_step_types;
			new_eval_experiment->actions = this->best_actions;
			new_eval_experiment->scopes = this->best_scopes;

			this->node_context->experiment = new_eval_experiment;
			delete this;
		} else {
			this->node_context->experiment = NULL;
			delete this;
		}
	}
}
