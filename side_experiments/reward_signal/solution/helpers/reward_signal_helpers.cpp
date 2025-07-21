#include "helpers.h"

#include <cmath>
#include <iostream>

#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

double calc_reward_signal(ScopeHistory* scope_history) {
	Scope* scope = scope_history->scope;

	double sum_vals = scope->score_average_val;
	for (int i_index = 0; i_index < (int)scope->score_inputs.size(); i_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_history,
						   scope->score_inputs[i_index],
						   0,
						   val,
						   is_on);
		if (is_on) {
			double normalized_val = (val - scope->score_input_averages[i_index]) / scope->score_input_standard_deviations[i_index];
			sum_vals += scope->score_weights[i_index] * normalized_val;
		}
	}

	return sum_vals;
}

void update_reward_signals(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		train_score(wrapper->solution->scopes[s_index],
					wrapper);
	}

	for (int h_index = 0; h_index < (int)wrapper->solution->explore_scope_histories.size(); h_index++) {
		delete wrapper->solution->explore_scope_histories[h_index];
	}
	wrapper->solution->explore_scope_histories.clear();
	wrapper->solution->explore_target_val_histories.clear();
}

bool compare_signals(map<Scope*, vector<double>>& existing_signals,
					 map<Scope*, vector<double>>& new_signals) {
	for (map<Scope*, vector<double>>::iterator existing_it = existing_signals.begin();
			existing_it != existing_signals.end(); existing_it++) {
		map<Scope*, vector<double>>::iterator new_it = new_signals.find(existing_it->first);
		if (new_it != new_signals.end()) {
			double existing_sum_signal = 0.0;
			for (int h_index = 0; h_index < (int)existing_it->second.size(); h_index++) {
				existing_sum_signal += existing_it->second[h_index];
			}
			double existing_signal = existing_sum_signal / (double)existing_it->second.size();

			double existing_sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)existing_it->second.size(); h_index++) {
				existing_sum_variance += (existing_it->second[h_index] - existing_signal)
					* (existing_it->second[h_index] - existing_signal);
			}
			double existing_standard_deviation = sqrt(existing_sum_variance / (double)existing_it->second.size());
			double existing_standard_error = existing_standard_deviation / sqrt((double)existing_it->second.size());

			double new_sum_signal = 0.0;
			for (int h_index = 0; h_index < (int)new_it->second.size(); h_index++) {
				new_sum_signal += new_it->second[h_index];
			}
			double new_signal = new_sum_signal / (double)new_it->second.size();

			double new_sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)new_it->second.size(); h_index++) {
				new_sum_variance += (new_it->second[h_index] - new_signal)
					* (new_it->second[h_index] - new_signal);
			}
			double new_standard_deviation = sqrt(new_sum_variance / (double)new_it->second.size());
			double new_standard_error = new_standard_deviation / sqrt((double)new_it->second.size());

			double denom = sqrt(existing_standard_error * existing_standard_error
				+ new_standard_error * new_standard_error);

			double t_score = (new_signal - existing_signal) / denom;

			// temp
			cout << "new_signal: " << new_signal << endl;
			cout << "existing_signal: " << existing_signal << endl;
			cout << "t_score: " << t_score << endl;

			if (t_score < -0.674) {
				return false;
			}
		}
	}

	return true;
}
