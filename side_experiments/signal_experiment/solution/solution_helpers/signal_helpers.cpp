#include "solution_helpers.h"

#include <iostream>

#include "network.h"
#include "scope.h"
#include "signal_experiment.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int MIN_EVAL_ITERS = 3;

double calc_signal(vector<ScopeHistory*>& post_scope_histories,
				   double target_val,
				   SolutionWrapper* wrapper) {
	double sum_vals = target_val - wrapper->solution->curr_score;
	for (int h_index = 0; h_index < (int)post_scope_histories.size(); h_index++) {
		if (!post_scope_histories[h_index]->signal_initialized) {
			post_scope_histories[h_index]->signal_initialized = true;

			Scope* scope = post_scope_histories[h_index]->scope;

			vector<double> inputs = post_scope_histories[h_index]->pre_obs;
			inputs.insert(inputs.end(), post_scope_histories[h_index]->post_obs.begin(), post_scope_histories[h_index]->post_obs.end());

			scope->signal_experiment->consistency_network->activate(inputs);
			double consistency = scope->signal_experiment->consistency_network->output->acti_vals[0];
			if (consistency <= 0.0) {
				post_scope_histories[h_index]->signal_val = 0.0;
			} else {
				if (consistency > 1.0) {
					consistency = 1.0;
				}

				scope->signal_experiment->signal_network->activate(inputs);
				double adjusted = consistency * scope->signal_experiment->signal_network->output->acti_vals[0];

				post_scope_histories[h_index]->signal_val = adjusted;
			}
		}
		sum_vals += post_scope_histories[h_index]->signal_val;
	}

	return sum_vals;
}

// temp
double calc_only_signal(vector<ScopeHistory*>& post_scope_histories,
						double target_val,
						SolutionWrapper* wrapper) {
	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)post_scope_histories.size(); h_index++) {
		if (!post_scope_histories[h_index]->signal_initialized) {
			post_scope_histories[h_index]->signal_initialized = true;

			Scope* scope = post_scope_histories[h_index]->scope;

			vector<double> inputs = post_scope_histories[h_index]->pre_obs;
			inputs.insert(inputs.end(), post_scope_histories[h_index]->post_obs.begin(), post_scope_histories[h_index]->post_obs.end());

			scope->signal_experiment->consistency_network->activate(inputs);
			double consistency = scope->signal_experiment->consistency_network->output->acti_vals[0];
			if (consistency <= 0.0) {
				post_scope_histories[h_index]->signal_val = 0.0;
			} else {
				if (consistency > 1.0) {
					consistency = 1.0;
				}

				scope->signal_experiment->signal_network->activate(inputs);
				double adjusted = consistency * scope->signal_experiment->signal_network->output->acti_vals[0];

				post_scope_histories[h_index]->signal_val = adjusted;
			}
		}
		sum_vals += post_scope_histories[h_index]->signal_val;
	}

	return sum_vals;
}

void eval_signal_experiment(SolutionWrapper* wrapper) {
	/**
	 * - check that signal is increasing and correlates with result
	 */
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		if (wrapper->solution->scopes[s_index]->signal_experiment != NULL) {
			SignalExperiment* signal_experiment = wrapper->solution->scopes[s_index]->signal_experiment;
			// temp
			for (int h_index = 0; h_index < (int)signal_experiment->signal_history.size(); h_index++) {
				cout << h_index << ": " << signal_experiment->signal_history[h_index] << endl;
			}
			if (signal_experiment->signal_history.size() >= MIN_EVAL_ITERS) {
				vector<double> iter_vals(signal_experiment->signal_history.size());
				for (int i_index = 0; i_index < (int)signal_experiment->signal_history.size(); i_index++) {
					iter_vals[i_index] = i_index;
				}
				double increase_pcc = calc_pcc(signal_experiment->signal_history,
											   iter_vals);
				cout << "increase_pcc: " << increase_pcc << endl;

				vector<double> score_vals(signal_experiment->signal_history.size());
				for (int i_index = 0; i_index < (int)signal_experiment->signal_history.size(); i_index++) {
					score_vals[i_index] = wrapper->solution->improvement_history[
						wrapper->solution->improvement_history.size() - signal_experiment->signal_history.size() + i_index];
				}
				double correlation_pcc = calc_pcc(signal_experiment->signal_history,
												  score_vals);
				cout << "correlation_pcc: " << correlation_pcc << endl;

				if (increase_pcc < 0.2 || correlation_pcc < 0.2) {
					delete signal_experiment;
					wrapper->solution->scopes[s_index]->signal_experiment = NULL;
				}
			}
		}
	}
}
