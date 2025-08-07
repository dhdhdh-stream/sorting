/**
 * - reward signal is as much about preserving existing, as making improvements
 *   - as signal only valid under conditions
 * 
 * - there can be multiple options for "consistency" that make sense
 *   - e.g., train at a train station
 *     - could be correct to follow train or follow train station
 * - the option that should be chosen is the one that produces the best signal
 */

#include "helpers.h"

#include <cmath>
#include <iostream>
#include <limits>

#include "constants.h"
#include "factor.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int SPLIT_NUM_TRIES = 2;
#else
const int SPLIT_NUM_TRIES = 20;
#endif /* MDEBUG */

const double MIN_MATCH_RATIO = 0.1;

const double CHECK_MIN_MATCH_RATIO = 0.01;

bool check_incorrect_signals_helper(vector<ScopeHistory*>& scope_histories,
									vector<double>& target_val_histories,
									Signal signal) {
	Scope* scope = scope_histories[0]->scope;

	vector<double> match_target_vals;
	vector<double> match_signals;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		int match_factor_index = signal.match_factor_index;
		if (!scope_histories[h_index]->factor_initialized[match_factor_index]) {
			double value = scope->factors[match_factor_index]->back_activate(scope_histories[h_index]);
			scope_histories[h_index]->factor_initialized[match_factor_index] = true;
			scope_histories[h_index]->factor_values[match_factor_index] = value;
		}
		double match_val = scope_histories[h_index]->factor_values[match_factor_index];
		if (match_val > 0.0) {
			match_target_vals.push_back(target_val_histories[h_index]);

			double sum_vals = signal.score_average_val;
			for (int i_index = 0; i_index < (int)signal.score_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_histories[h_index],
								   signal.score_inputs[i_index],
								   0,
								   val,
								   is_on);
				if (is_on) {
					double normalized_val = (val - signal.score_input_averages[i_index])
						/ signal.score_input_standard_deviations[i_index];
					sum_vals += signal.score_weights[i_index] * normalized_val;
				}
			}
			match_signals.push_back(sum_vals);
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (rand()%4 == 0) {
	#else
	if (match_target_vals.size() < CHECK_MIN_MATCH_RATIO * scope_histories.size()) {
	#endif /* MDEBUG */
		return false;
	}

	double sum_vals = 0.0;
	for (int m_index = 0; m_index < (int)match_target_vals.size(); m_index++) {
		sum_vals += match_target_vals[m_index];
	}
	double match_average_val = sum_vals / (double)match_target_vals.size();

	double sum_misguesses = 0.0;
	for (int m_index = 0; m_index < (int)match_target_vals.size(); m_index++) {
		sum_misguesses += (match_target_vals[m_index] - match_average_val)
			* (match_target_vals[m_index] - match_average_val);
	}

	double sum_signal_misguesses = 0.0;
	for (int m_index = 0; m_index < (int)match_target_vals.size(); m_index++) {
		sum_signal_misguesses += (match_target_vals[m_index] - match_signals[m_index])
			* (match_target_vals[m_index] - match_signals[m_index]);
	}

	#if defined(MDEBUG) && MDEBUG
	return sum_signal_misguesses < sum_misguesses && rand()%4 == 0;
	#else
	return sum_signal_misguesses < sum_misguesses;
	#endif /* MDEBUG */
}

void check_incorrect_signals(vector<ScopeHistory*>& scope_histories,
							 vector<double>& target_val_histories,
							 ScopeNode* signal_scope_node) {
	for (int s_index = (int)signal_scope_node->signals.size()-1; s_index >= 0; s_index--) {
		bool is_valid = check_incorrect_signals_helper(scope_histories,
													   target_val_histories,
													   signal_scope_node->signals[s_index]);
		if (!is_valid) {
			signal_scope_node->signals.erase(signal_scope_node->signals.begin() + s_index);
		}
	}
}

void check_unhittable_signals(vector<ScopeHistory*>& scope_histories,
							  ScopeNode* signal_scope_node) {
	Scope* scope = scope_histories[0]->scope;

	vector<double> counts(signal_scope_node->signals.size(), 0);
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		for (int s_index = 0; s_index < (int)signal_scope_node->signals.size(); s_index++) {
			int match_factor_index = signal_scope_node->signals[s_index].match_factor_index;
			if (!scope_histories[h_index]->factor_initialized[match_factor_index]) {
				double value = scope->factors[match_factor_index]->back_activate(scope_histories[h_index]);
				scope_histories[h_index]->factor_initialized[match_factor_index] = true;
				scope_histories[h_index]->factor_values[match_factor_index] = value;
			}
			double match_val = scope_histories[h_index]->factor_values[match_factor_index];
			if (match_val > 0.0) {
				counts[s_index]++;
				break;
			}
		}
	}

	for (int s_index = (int)signal_scope_node->signals.size()-1; s_index >= 0; s_index--) {
		if (counts[s_index] == 0) {
			signal_scope_node->signals.erase(signal_scope_node->signals.begin() + s_index);
		}
	}
}

void gather_existing_histories_helper(ScopeHistory* scope_history,
									  ScopeNode* signal_scope_node,
									  vector<ScopeHistory*>& existing_scope_histories) {
	Scope* scope = scope_history->scope;

	Scope* parent_scope = signal_scope_node->parent;

	if (scope == parent_scope) {
		map<int, AbstractNodeHistory*>::iterator it = scope_history
			->node_histories.find(signal_scope_node->id);
		if (it != scope_history->node_histories.end()) {
			existing_scope_histories.push_back(scope_history);
		}
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == parent_scope) {
				is_child = true;
				break;
			}
		}

		if (is_child) {
			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
				AbstractNode* node = it->second->node;
				if (node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					gather_existing_histories_helper(scope_node_history->scope_history,
													 signal_scope_node,
													 existing_scope_histories);
				}
			}
		}
	}
}

double calc_miss_average_guess(vector<ScopeHistory*>& scope_histories,
							   vector<double>& target_val_histories,
							   vector<Input> new_match_inputs,
							   Network* new_match_network,
							   vector<Signal>& signals) {
	Scope* scope = scope_histories[0]->scope;

	double sum_target_vals = 0.0;
	int sum_count = 0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		vector<double> input_vals(new_match_inputs.size());
		vector<bool> input_is_on(new_match_inputs.size());
		for (int i_index = 0; i_index < (int)new_match_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_histories[h_index],
							   new_match_inputs[i_index],
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		new_match_network->activate(input_vals,
									input_is_on);
		#if defined(MDEBUG) && MDEBUG
		if (rand()%3 == 0) {
		#else
		if (new_match_network->output->acti_vals[0] > 0.0) {
		#endif /* MDEBUG */
			continue;
		}

		bool has_match = false;
		for (int s_index = 0; s_index < (int)signals.size(); s_index++) {
			int match_factor_index = signals[s_index].match_factor_index;
			if (!scope_histories[h_index]->factor_initialized[match_factor_index]) {
				double value = scope->factors[match_factor_index]->back_activate(scope_histories[h_index]);
				scope_histories[h_index]->factor_initialized[match_factor_index] = true;
				scope_histories[h_index]->factor_values[match_factor_index] = value;
			}
			double match_val = scope_histories[h_index]->factor_values[match_factor_index];
			#if defined(MDEBUG) && MDEBUG
			if (match_val > 0.0 || rand()%4 == 0) {
			#else
			if (match_val > 0.0) {
			#endif /* MDEBUG */
				has_match = true;
				break;
			}
		}

		if (!has_match) {
			sum_target_vals += target_val_histories[h_index];
			sum_count++;
		}
	}

	if (sum_count > 0) {
		return sum_target_vals / (double)sum_count;
	} else {
		return 0.0;
	}
}

double calc_signal(vector<Input> new_match_inputs,
				   Network* new_match_network,
				   double new_average_score,
				   vector<Input> new_factor_inputs,
				   vector<double> new_factor_input_averages,
				   vector<double> new_factor_input_standard_deviations,
				   vector<double> new_factor_weights,
				   vector<Input> new_network_inputs,
				   Network* new_network,
				   vector<Signal>& signals,
				   double miss_average_guess,
				   ScopeHistory* signal_needed_from) {
	Scope* scope = signal_needed_from->scope;

	{
		vector<double> input_vals(new_match_inputs.size());
		vector<bool> input_is_on(new_match_inputs.size());
		for (int i_index = 0; i_index < (int)new_match_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(signal_needed_from,
							   new_match_inputs[i_index],
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		new_match_network->activate(input_vals,
									input_is_on);
		#if defined(MDEBUG) && MDEBUG
		if (rand()%3 == 0) {
		#else
		if (new_match_network->output->acti_vals[0] > 0.0) {
		#endif /* MDEBUG */
			double sum_vals = new_average_score;
			for (int i_index = 0; i_index < (int)new_factor_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(signal_needed_from,
								   new_factor_inputs[i_index],
								   0,
								   val,
								   is_on);
				if (is_on) {
					double normalized_val = (val - new_factor_input_averages[i_index])
						/ new_factor_input_standard_deviations[i_index];
					sum_vals += new_factor_weights[i_index] * normalized_val;
				}
			}

			if (new_network != NULL) {
				vector<double> input_vals(new_network_inputs.size());
				vector<bool> input_is_on(new_network_inputs.size());
				for (int i_index = 0; i_index < (int)new_network_inputs.size(); i_index++) {
					double val;
					bool is_on;
					fetch_input_helper(signal_needed_from,
									   new_network_inputs[i_index],
									   0,
									   val,
									   is_on);
					input_vals[i_index] = val;
					input_is_on[i_index] = is_on;
				}
				new_network->activate(input_vals,
									  input_is_on);
				sum_vals += new_network->output->acti_vals[0];
			}

			return sum_vals;
		}
	}

	for (int s_index = 0; s_index < (int)signals.size(); s_index++) {
		int match_factor_index = signals[s_index].match_factor_index;
		if (!signal_needed_from->factor_initialized[match_factor_index]) {
			double value = scope->factors[match_factor_index]->back_activate(signal_needed_from);
			signal_needed_from->factor_initialized[match_factor_index] = true;
			signal_needed_from->factor_values[match_factor_index] = value;
		}
		double match_val = signal_needed_from->factor_values[match_factor_index];
		#if defined(MDEBUG) && MDEBUG
		if (match_val > 0.0 || rand()%4 == 0) {
		#else
		if (match_val > 0.0) {
		#endif /* MDEBUG */
			double sum_vals = signals[s_index].score_average_val;
			for (int i_index = 0; i_index < (int)signals[s_index].score_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(signal_needed_from,
								   signals[s_index].score_inputs[i_index],
								   0,
								   val,
								   is_on);
				if (is_on) {
					double normalized_val = (val - signals[s_index].score_input_averages[i_index])
						/ signals[s_index].score_input_standard_deviations[i_index];
					sum_vals += signals[s_index].score_weights[i_index] * normalized_val;
				}
			}

			return sum_vals;
		}
	}

	return miss_average_guess;
}

void create_reward_signal_helper(ScopeNode* scope_node,
								 SolutionWrapper* wrapper) {
	cout << "create_reward_signal_helper" << endl;

	vector<ScopeHistory*> existing_scope_histories;
	for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
		gather_existing_histories_helper(wrapper->solution->existing_scope_histories[h_index],
										 scope_node,
										 existing_scope_histories);
	}
	#if defined(MDEBUG) && MDEBUG
	if (existing_scope_histories.size() == 0) {
		return;
	}
	#endif /* MDEBUG */

	vector<double> existing_vals;
	if (scope_node->signals.size() > 0) {
		existing_vals = vector<double>(scope_node->explore_scope_histories.size());
		for (int h_index = 0; h_index < (int)scope_node->explore_scope_histories.size(); h_index++) {
			existing_vals[h_index] = calc_signal(scope_node,
												 scope_node->explore_scope_histories[h_index]);
		}
	} else {
		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)scope_node->explore_target_val_histories.size(); h_index++) {
			sum_vals += scope_node->explore_target_val_histories[h_index];
		}
		double average_val = sum_vals / (double)scope_node->explore_target_val_histories.size();

		existing_vals = vector<double>(scope_node->explore_scope_histories.size(), average_val);
	}

	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)scope_node->explore_scope_histories.size(); h_index++) {
		sum_misguess += (scope_node->explore_target_val_histories[h_index] - existing_vals[h_index])
			* (scope_node->explore_target_val_histories[h_index] - existing_vals[h_index]);
	}
	double curr_misguess_average = sum_misguess / (double)scope_node->explore_scope_histories.size();

	double sum_misguess_variance = 0.0;
	for (int h_index = 0; h_index < (int)scope_node->explore_scope_histories.size(); h_index++) {
		double curr_misguess = (scope_node->explore_target_val_histories[h_index] - existing_vals[h_index])
			* (scope_node->explore_target_val_histories[h_index] - existing_vals[h_index]);
		sum_misguess_variance += (curr_misguess - curr_misguess_average)
			* (curr_misguess - curr_misguess_average);
	}
	double curr_misguess_standard_deviation = sqrt(sum_misguess_variance / (double)scope_node->explore_scope_histories.size());

	#if defined(MDEBUG) && MDEBUG
	int num_min_match = max(2, (int)(MIN_MATCH_RATIO * (int)scope_node->explore_scope_histories.size()));
	#else
	int num_min_match = MIN_MATCH_RATIO * (int)scope_node->explore_scope_histories.size();
	#endif /* MDEBUG */
	for (int t_index = 0; t_index < SPLIT_NUM_TRIES; t_index++) {
		vector<Input> new_match_inputs;
		Network* new_match_network = NULL;
		bool split_is_success = split_helper(existing_scope_histories,
											 scope_node->explore_scope_histories,
											 scope_node,
											 new_match_inputs,
											 new_match_network);

		if (split_is_success) {
			vector<ScopeHistory*> match_histories;
			vector<double> match_target_vals;
			for (int h_index = 0; h_index < (int)scope_node->explore_scope_histories.size(); h_index++) {
				vector<double> input_vals(new_match_inputs.size());
				vector<bool> input_is_on(new_match_inputs.size());
				for (int i_index = 0; i_index < (int)new_match_inputs.size(); i_index++) {
					double val;
					bool is_on;
					fetch_input_helper(scope_node->explore_scope_histories[h_index],
									   new_match_inputs[i_index],
									   0,
									   val,
									   is_on);
					input_vals[i_index] = val;
					input_is_on[i_index] = is_on;
				}
				new_match_network->activate(input_vals,
											input_is_on);
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				if (new_match_network->output->acti_vals[0] > 0.0) {
				#endif /* MDEBUG */
					match_histories.push_back(scope_node->explore_scope_histories[h_index]);
					match_target_vals.push_back(scope_node->explore_target_val_histories[h_index]);
				}
			}

			cout << "match_histories.size(): " << match_histories.size() << endl;

			if ((int)match_histories.size() >= num_min_match) {
				double new_average_score;
				vector<Input> new_factor_inputs;
				vector<double> new_factor_input_averages;
				vector<double> new_factor_input_standard_deviations;
				vector<double> new_factor_weights;
				vector<Input> new_network_inputs;
				Network* new_network = NULL;
				bool is_success = train_score(match_histories,
											  match_target_vals,
											  scope_node,
											  new_average_score,
											  new_factor_inputs,
											  new_factor_input_averages,
											  new_factor_input_standard_deviations,
											  new_factor_weights,
											  new_network_inputs,
											  new_network);
				if (is_success) {
					double potential_miss_average_guess = calc_miss_average_guess(
						scope_node->explore_scope_histories,
						scope_node->explore_target_val_histories,
						new_match_inputs,
						new_match_network,
						scope_node->signals);

					vector<double> potential_vals(scope_node->explore_scope_histories.size());
					for (int h_index = 0; h_index < (int)scope_node->explore_scope_histories.size(); h_index++) {
						potential_vals[h_index] = calc_signal(new_match_inputs,
															  new_match_network,
															  new_average_score,
															  new_factor_inputs,
															  new_factor_input_averages,
															  new_factor_input_standard_deviations,
															  new_factor_weights,
															  new_network_inputs,
															  new_network,
															  scope_node->signals,
															  potential_miss_average_guess,
															  scope_node->explore_scope_histories[h_index]);
					}

					double sum_potential_misguess = 0.0;
					for (int h_index = 0; h_index < (int)scope_node->explore_scope_histories.size(); h_index++) {
						sum_potential_misguess += (scope_node->explore_target_val_histories[h_index] - potential_vals[h_index])
							* (scope_node->explore_target_val_histories[h_index] - potential_vals[h_index]);
					}
					double potential_misguess_average = sum_potential_misguess / (double)scope_node->explore_scope_histories.size();

					double sum_potential_misguess_variance = 0.0;
					for (int h_index = 0; h_index < (int)scope_node->explore_scope_histories.size(); h_index++) {
						double curr_misguess = (scope_node->explore_target_val_histories[h_index] - potential_vals[h_index])
							* (scope_node->explore_target_val_histories[h_index] - potential_vals[h_index]);
						sum_potential_misguess_variance += (curr_misguess - potential_misguess_average)
							* (curr_misguess - potential_misguess_average);
					}
					double potential_misguess_standard_deviation = sqrt(sum_potential_misguess_variance / (double)scope_node->explore_scope_histories.size());

					double misguess_improvement = curr_misguess_average - potential_misguess_average;
					double min_standard_deviation = min(curr_misguess_standard_deviation, potential_misguess_standard_deviation);
					double t_score = misguess_improvement / (min_standard_deviation / sqrt((double)scope_node->explore_scope_histories.size()));

					cout << "measure t_score: " << t_score << endl;

					#if defined(MDEBUG) && MDEBUG
					if (t_score >= 1.282 || rand()%2 == 0) {
					#else
					if (t_score >= 1.282) {
					#endif /* MDEBUG */
						Signal new_signal;

						{
							Factor* new_factor = new Factor();
							new_factor->inputs = new_match_inputs;
							new_factor->network = new_match_network;
							new_factor->is_meaningful = true;

							scope_node->parent->factors.push_back(new_factor);

							new_factor->link((int)scope_node->parent->factors.size()-1);

							new_signal.match_factor_index = (int)scope_node->parent->factors.size()-1;

							new_match_network = NULL;
						}

						if (new_network != NULL) {
							Factor* new_factor = new Factor();
							new_factor->inputs = new_network_inputs;
							new_factor->network = new_network;
							new_factor->is_meaningful = true;

							scope_node->parent->factors.push_back(new_factor);

							new_factor->link((int)scope_node->parent->factors.size()-1);

							Input new_input;
							new_input.scope_context = {scope_node->parent};
							new_input.factor_index = (int)scope_node->parent->factors.size()-1;
							new_input.node_context = {-1};
							new_input.obs_index = -1;

							new_factor_inputs.push_back(new_input);
							new_factor_input_averages.push_back(0.0);
							new_factor_input_standard_deviations.push_back(1.0);
							new_factor_weights.push_back(1.0);

							new_network = NULL;
						}

						new_signal.score_average_val = new_average_score;
						new_signal.score_inputs = new_factor_inputs;
						new_signal.score_input_averages = new_factor_input_averages;
						new_signal.score_input_standard_deviations = new_factor_input_standard_deviations;
						new_signal.score_weights = new_factor_weights;

						scope_node->signals.insert(scope_node->signals.begin(), new_signal);
						scope_node->miss_average_guess = potential_miss_average_guess;

						curr_misguess_average = potential_misguess_average;
						curr_misguess_standard_deviation = potential_misguess_standard_deviation;

						// temp
						wrapper->save("saves/", "main.txt");
					}

					// temp
					throw invalid_argument("success?");
				}

				if (new_network != NULL) {
					delete new_network;
				}
			}
		}

		if (new_match_network != NULL) {
			delete new_match_network;
		}
	}
}

void update_reward_signals(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)it->second;
				if (scope_node->explore_scope_histories.size() >= EXPLORE_TARGET_NUM_SAMPLES) {
					check_incorrect_signals(scope_node->explore_scope_histories,
											scope_node->explore_target_val_histories,
											scope_node);

					#if defined(MDEBUG) && MDEBUG
					#else
					check_unhittable_signals(scope_node->explore_scope_histories,
											 scope_node);
					#endif /* MDEBUG */

					create_reward_signal_helper(scope_node,
												wrapper);

					for (int h_index = 0; h_index < (int)scope_node->explore_scope_histories.size(); h_index++) {
						delete scope_node->explore_scope_histories[h_index];
					}
					scope_node->explore_scope_histories.clear();
					scope_node->explore_target_val_histories.clear();
				}
			}
		}
	}
}

bool factor_has_dependency_on_scope_node(
		Scope* scope,
		int factor_index,
		int scope_node_id) {
	Factor* factor = scope->factors[factor_index];
	for (int i_index = 0; i_index < (int)factor->inputs.size(); i_index++) {
		if (factor->inputs[i_index].scope_context.size() == 1
				&& factor->inputs[i_index].factor_index != -1) {
			bool has_dependency = factor_has_dependency_on_scope_node(
				scope,
				factor->inputs[i_index].factor_index,
				scope_node_id);
			if (has_dependency) {
				return true;
			}
		} else {
			if (factor->inputs[i_index].node_context[0] == scope_node_id) {
				return true;
			}
		}
	}

	return false;
}
