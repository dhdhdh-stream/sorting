#include "helpers.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

bool check_match(ScopeHistory* scope_history) {
	Scope* scope = scope_history->scope;

	if (scope->check_match) {
		if (!scope_history->factor_initialized[scope->match_factor_index]) {
			double value = scope->factors[scope->match_factor_index]->back_activate(scope_history);
			scope_history->factor_initialized[scope->match_factor_index] = true;
			scope_history->factor_values[scope->match_factor_index] = value;
		}

		double val = scope_history->factor_values[scope->match_factor_index];

		if (val <= 0.0) {
			return false;
		}
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		if (node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			bool inner_result = check_match(scope_node_history->scope_history);
			if (!inner_result) {
				return false;
			}
		}
	}

	return true;
}

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

void add_explore_helper(ScopeHistory* scope_history,
						double target_val,
						Scope* explore_scope) {
	Scope* scope = scope_history->scope;

	if (scope != explore_scope) {
		if (scope->explore_scope_histories.size() < EXPLORE_TARGET_NUM_SAMPLES) {
			scope->explore_scope_histories.push_back(new ScopeHistory(scope_history));
			scope->explore_target_val_histories.push_back(target_val);
		} else {
			uniform_int_distribution<int> distribution(0, scope->explore_scope_histories.size()-1);
			int random_index = distribution(generator);
			delete scope->explore_scope_histories[random_index];
			scope->explore_scope_histories[random_index] = new ScopeHistory(scope_history);
			scope->explore_target_val_histories[random_index] = target_val;
		}
	}

	double inner_target_val;
	if (scope->score_inputs.size() > 0
			&& scope != explore_scope) {
		if (!scope_history->signal_initialized) {
			scope_history->signal_val = calc_reward_signal(scope_history);
		}
		inner_target_val = scope_history->signal_val;
	} else {
		inner_target_val = target_val;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		if (node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			add_explore_helper(scope_node_history->scope_history,
							   inner_target_val,
							   explore_scope);
		}
	}
}

void update_reward_signals(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		if (scope->explore_scope_histories.size() >= EXPLORE_TARGET_NUM_SAMPLES) {
			create_reward_signal_helper(scope,
										wrapper);

			for (int h_index = 0; h_index < (int)scope->explore_scope_histories.size(); h_index++) {
				delete scope->explore_scope_histories[h_index];
			}
			scope->explore_scope_histories.clear();
			scope->explore_target_val_histories.clear();
		}
	}
}

void fetch_signals_helper(ScopeHistory* scope_history,
						  map<Scope*, vector<pair<double,double>>>& signals) {
	Scope* scope = scope_history->scope;

	if (scope->score_inputs.size() > 0) {
		if (!scope_history->factor_initialized[scope->match_factor_index]) {
			double value = scope->factors[scope->match_factor_index]->back_activate(scope_history);
			scope_history->factor_initialized[scope->match_factor_index] = true;
			scope_history->factor_values[scope->match_factor_index] = value;
		}
		double match_val = scope_history->factor_values[scope->match_factor_index];
		match_val = min(max(match_val, -1.0), 1.0);

		/**
		 * - compare signal even if explore to try to protect
		 */
		if (!scope_history->signal_initialized) {
			scope_history->signal_val = calc_reward_signal(scope_history);
		}

		map<Scope*, vector<pair<double,double>>>::iterator it = signals.find(scope);
		if (it == signals.end()) {
			it = signals.insert({scope, vector<pair<double,double>>()}).first;
		}

		it->second.push_back({match_val, scope_history->signal_val});
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		if (node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			fetch_signals_helper(scope_node_history->scope_history,
								 signals);
		}
	}
}

bool compare_result(vector<double>& existing_scores,
					map<Scope*, vector<pair<double,double>>>& existing_signals,
					vector<double>& new_scores,
					map<Scope*, vector<pair<double,double>>>& new_signals,
					double& improvement) {
	{
		double existing_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)existing_scores.size(); h_index++) {
			existing_sum_score += existing_scores[h_index];
		}
		double existing_score = existing_sum_score / (double)existing_scores.size();

		double new_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)new_scores.size(); h_index++) {
			new_sum_score += new_scores[h_index];
		}
		double new_score = new_sum_score / (double)new_scores.size();

		improvement = new_score - existing_score;

		double existing_sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)existing_scores.size(); h_index++) {
			existing_sum_variance += (existing_scores[h_index] - existing_score)
				* (existing_scores[h_index] - existing_score);
		}
		double existing_standard_deviation = sqrt(existing_sum_variance / (double)existing_scores.size());
		double existing_standard_error = existing_standard_deviation / sqrt((double)existing_scores.size());

		double new_sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)new_scores.size(); h_index++) {
			new_sum_variance += (new_scores[h_index] - new_score)
				* (new_scores[h_index] - new_score);
		}
		double new_standard_deviation = sqrt(new_sum_variance / (double)new_scores.size());
		double new_standard_error = new_standard_deviation / sqrt((double)new_scores.size());

		double denom = sqrt(existing_standard_error * existing_standard_error
			+ new_standard_error * new_standard_error);

		double t_score = (new_score - existing_score) / denom;

		/**
		 * - accept changes that are not provably bad to drive progress
		 *   - but select best from number of candidates to drive improvement
		 */
		if (t_score < -0.674) {
			return false;
		}
	}

	for (map<Scope*, vector<pair<double,double>>>::iterator existing_it = existing_signals.begin();
			existing_it != existing_signals.end(); existing_it++) {
		map<Scope*, vector<pair<double,double>>>::iterator new_it = new_signals.find(existing_it->first);
		if (new_it != new_signals.end()) {
			{
				double existing_sum_match = 0.0;
				for (int h_index = 0; h_index < (int)existing_it->second.size(); h_index++) {
					existing_sum_match += existing_it->second[h_index].first;
				}
				double existing_match = existing_sum_match / (double)existing_it->second.size();

				double existing_sum_variance = 0.0;
				for (int h_index = 0; h_index < (int)existing_it->second.size(); h_index++) {
					existing_sum_variance += (existing_it->second[h_index].first - existing_match)
						* (existing_it->second[h_index].first - existing_match);
				}
				double existing_standard_deviation = sqrt(existing_sum_variance / (double)existing_it->second.size());
				double existing_standard_error = existing_standard_deviation / sqrt((double)existing_it->second.size());

				double new_sum_match = 0.0;
				for (int h_index = 0; h_index < (int)new_it->second.size(); h_index++) {
					new_sum_match += new_it->second[h_index].first;
				}
				double new_match = new_sum_match / (double)new_it->second.size();

				double new_sum_variance = 0.0;
				for (int h_index = 0; h_index < (int)new_it->second.size(); h_index++) {
					new_sum_variance += (new_it->second[h_index].first - new_match)
						* (new_it->second[h_index].first - new_match);
				}
				double new_standard_deviation = sqrt(new_sum_variance / (double)new_it->second.size());
				double new_standard_error = new_standard_deviation / sqrt((double)new_it->second.size());

				double denom = sqrt(existing_standard_error * existing_standard_error
					+ new_standard_error * new_standard_error);

				double t_score = (new_match - existing_match) / denom;

				// temp
				cout << "new_match: " << new_match << endl;
				cout << "existing_match: " << existing_match << endl;
				cout << "t_score: " << t_score << endl;

				if (t_score < -0.674) {
					return false;
				}
			}

			{
				double existing_sum_signal = 0.0;
				for (int h_index = 0; h_index < (int)existing_it->second.size(); h_index++) {
					existing_sum_signal += existing_it->second[h_index].second;
				}
				double existing_signal = existing_sum_signal / (double)existing_it->second.size();

				double existing_sum_variance = 0.0;
				for (int h_index = 0; h_index < (int)existing_it->second.size(); h_index++) {
					existing_sum_variance += (existing_it->second[h_index].second - existing_signal)
						* (existing_it->second[h_index].second - existing_signal);
				}
				double existing_standard_deviation = sqrt(existing_sum_variance / (double)existing_it->second.size());
				double existing_standard_error = existing_standard_deviation / sqrt((double)existing_it->second.size());

				double new_sum_signal = 0.0;
				for (int h_index = 0; h_index < (int)new_it->second.size(); h_index++) {
					new_sum_signal += new_it->second[h_index].second;
				}
				double new_signal = new_sum_signal / (double)new_it->second.size();

				double new_sum_variance = 0.0;
				for (int h_index = 0; h_index < (int)new_it->second.size(); h_index++) {
					new_sum_variance += (new_it->second[h_index].second - new_signal)
						* (new_it->second[h_index].second - new_signal);
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
	}

	return true;
}
