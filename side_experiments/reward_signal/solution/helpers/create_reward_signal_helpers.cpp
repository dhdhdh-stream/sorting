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

	if (match_target_vals.size() < CHECK_MIN_MATCH_RATIO * scope_histories.size()) {
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

	return sum_signal_misguesses < sum_misguesses;
}

void check_incorrect_signals(vector<ScopeHistory*>& scope_histories,
							 vector<double>& target_val_histories,
							 ScopeNode* signal_scope_node) {
	for (int s_index = (int)signal_scope_node->signals.size() - 1; s_index >= 0; s_index--) {
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
	
}

// void gather_existing_histories_helper(ScopeHistory* scope_history,
// 									  Scope* signal_scope,
// 									  vector<ScopeHistory*>& existing_scope_histories) {
// 	Scope* scope = scope_history->scope;

// 	if (scope == signal_scope) {
// 		existing_scope_histories.push_back(scope_history);
// 	} else {
// 		bool is_child = false;
// 		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
// 			if (scope->child_scopes[c_index] == signal_scope) {
// 				is_child = true;
// 				break;
// 			}
// 		}

// 		if (is_child) {
// 			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
// 					it != scope_history->node_histories.end(); it++) {
// 				AbstractNode* node = it->second->node;
// 				if (node->type == NODE_TYPE_SCOPE) {
// 					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
// 					gather_existing_histories_helper(scope_node_history->scope_history,
// 													 signal_scope,
// 													 existing_scope_histories);
// 				}
// 			}
// 		}
// 	}
// }

double calc_miss_average_guess(vector<ScopeHistory*>& scope_histories,
							   vector<double>& target_val_histories,
							   vector<Signal>& signals) {
	Scope* scope = scope_histories[0]->scope;

	double sum_target_vals = 0.0;
	int sum_count = 0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		bool has_match = false;
		for (int s_index = 0; s_index < (int)signals.size(); s_index++) {
			int match_factor_index = signals[s_index].match_factor_index;
			if (!scope_histories[h_index]->factor_initialized[match_factor_index]) {
				double value = scope->factors[match_factor_index]->back_activate(scope_histories[h_index]);
				scope_histories[h_index]->factor_initialized[match_factor_index] = true;
				scope_histories[h_index]->factor_values[match_factor_index] = value;
			}
			double match_val = scope_histories[h_index]->factor_values[match_factor_index];
			if (match_val > 0.0) {
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

double calc_signal(vector<Signal>& signals,
				   double miss_average_guess,
				   ScopeHistory* signal_needed_from) {
	Scope* scope = signal_needed_from->scope;

	for (int s_index = 0; s_index < (int)signals.size(); s_index++) {
		int match_factor_index = signals[s_index].match_factor_index;
		if (!signal_needed_from->factor_initialized[match_factor_index]) {
			double value = scope->factors[match_factor_index]->back_activate(signal_needed_from);
			signal_needed_from->factor_initialized[match_factor_index] = true;
			signal_needed_from->factor_values[match_factor_index] = value;
		}
		double match_val = signal_needed_from->factor_values[match_factor_index];
		if (match_val > 0.0) {
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

void measure_signal(vector<ScopeHistory*>& scope_histories,
					vector<double>& target_val_histories,
					vector<Signal>& signals,
					double miss_average_guess,
					double& new_misguess,
					double& new_misguess_standard_deviation) {
	vector<double> vals(scope_histories.size());
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		vals[h_index] = calc_signal(signals,
									miss_average_guess,
									scope_histories[h_index]);
	}

	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		sum_misguess += (target_val_histories[h_index] - vals[h_index])
			* (target_val_histories[h_index] - vals[h_index]);
	}
	new_misguess = sum_misguess / (double)scope_histories.size();

	double sum_misguess_variance = 0.0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		double curr_misguess = (target_val_histories[h_index] - vals[h_index])
			* (target_val_histories[h_index] - vals[h_index]);
		sum_misguess_variance += (curr_misguess - new_misguess)
			* (curr_misguess - new_misguess);
	}
	new_misguess_standard_deviation = sqrt(sum_misguess_variance / (double)scope_histories.size());
}

void create_reward_signal_helper(ScopeNode* scope_node,
								 SolutionWrapper* wrapper) {
	// double best_highest_signal = numeric_limits<double>::lowest();
	// if (scope->score_inputs.size() > 0) {
	// 	for (int h_index = 0; h_index < (int)scope->explore_scope_histories.size(); h_index++) {
	// 		ScopeHistory* scope_history = scope->explore_scope_histories[h_index];

	// 		bool is_match;
	// 		if (scope->check_match) {
	// 			if (!scope_history->factor_initialized[scope->match_factor_index]) {
	// 				double value = scope->factors[scope->match_factor_index]->back_activate(scope_history);
	// 				scope_history->factor_initialized[scope->match_factor_index] = true;
	// 				scope_history->factor_values[scope->match_factor_index] = value;
	// 			}

	// 			double val = scope_history->factor_values[scope->match_factor_index];

	// 			if (val > 0.0) {
	// 				is_match = true;
	// 			}
	// 		} else {
	// 			is_match = true;
	// 		}

	// 		if (is_match) {
	// 			double signal = calc_reward_signal(scope_history);
	// 			if (signal > best_highest_signal) {
	// 				best_highest_signal = signal;
	// 			}
	// 		}
	// 	}
	// }

	// {
	// 	double new_average_score;
	// 	vector<Input> new_factor_inputs;
	// 	vector<double> new_factor_input_averages;
	// 	vector<double> new_factor_input_standard_deviations;
	// 	vector<double> new_factor_weights;
	// 	vector<Input> new_network_inputs;
	// 	Network* new_network = NULL;
	// 	double new_highest_signal;
	// 	bool is_success = train_score(scope->explore_scope_histories,
	// 								  scope->explore_target_val_histories,
	// 								  new_average_score,
	// 								  new_factor_inputs,
	// 								  new_factor_input_averages,
	// 								  new_factor_input_standard_deviations,
	// 								  new_factor_weights,
	// 								  new_network_inputs,
	// 								  new_network,
	// 								  new_highest_signal);
	// 	if (is_success && new_highest_signal > best_highest_signal) {
	// 		best_highest_signal = new_highest_signal;

	// 		scope->check_match = false;
	// 		scope->match_factor_index = -1;

	// 		if (new_network != NULL) {
	// 			Factor* new_factor = new Factor();
	// 			new_factor->inputs = new_network_inputs;
	// 			new_factor->network = new_network;
	// 			new_factor->is_meaningful = true;

	// 			scope->factors.push_back(new_factor);

	// 			new_factor->link((int)scope->factors.size()-1);

	// 			Input new_input;
	// 			new_input.scope_context = {scope};
	// 			new_input.factor_index = (int)scope->factors.size()-1;
	// 			new_input.node_context = {-1};
	// 			new_input.obs_index = -1;

	// 			new_factor_inputs.push_back(new_input);
	// 			new_factor_input_averages.push_back(0.0);
	// 			new_factor_input_standard_deviations.push_back(1.0);
	// 			new_factor_weights.push_back(1.0);

	// 			new_network = NULL;
	// 		}

	// 		scope->score_average_val = new_average_score;
	// 		scope->score_inputs = new_factor_inputs;
	// 		scope->score_input_averages = new_factor_input_averages;
	// 		scope->score_input_standard_deviations = new_factor_input_standard_deviations;
	// 		scope->score_weights = new_factor_weights;
	// 	}

	// 	if (new_network != NULL) {
	// 		delete new_network;
	// 	}
	// }

	// vector<ScopeHistory*> existing_scope_histories;
	// for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
	// 	gather_existing_histories_helper(wrapper->solution->existing_scope_histories[h_index],
	// 									 scope,
	// 									 existing_scope_histories);
	// }

	// for (int t_index = 0; t_index < SPLIT_NUM_TRIES; t_index++) {
	// 	vector<Input> new_match_inputs;
	// 	Network* new_match_network = NULL;
	// 	bool split_is_success = split_helper(existing_scope_histories,
	// 										 scope->explore_scope_histories,
	// 										 new_match_inputs,
	// 										 new_match_network);

	// 	if (split_is_success) {
	// 		vector<ScopeHistory*> match_histories;
	// 		vector<double> match_target_vals;
	// 		for (int h_index = 0; h_index < (int)scope->explore_scope_histories.size(); h_index++) {
	// 			vector<double> input_vals(new_match_inputs.size());
	// 			vector<bool> input_is_on(new_match_inputs.size());
	// 			for (int i_index = 0; i_index < (int)new_match_inputs.size(); i_index++) {
	// 				double val;
	// 				bool is_on;
	// 				fetch_input_helper(scope->explore_scope_histories[h_index],
	// 								   new_match_inputs[i_index],
	// 								   0,
	// 								   val,
	// 								   is_on);
	// 				input_vals[i_index] = val;
	// 				input_is_on[i_index] = is_on;
	// 			}
	// 			new_match_network->activate(input_vals,
	// 										input_is_on);

	// 			if (new_match_network->output->acti_vals[0] > 0.0) {
	// 				match_histories.push_back(scope->explore_scope_histories[h_index]);
	// 				match_target_vals.push_back(scope->explore_target_val_histories[h_index]);
	// 			}
	// 		}

	// 		double new_average_score;
	// 		vector<Input> new_factor_inputs;
	// 		vector<double> new_factor_input_averages;
	// 		vector<double> new_factor_input_standard_deviations;
	// 		vector<double> new_factor_weights;
	// 		vector<Input> new_network_inputs;
	// 		Network* new_network = NULL;
	// 		double new_highest_signal;
	// 		bool is_success = train_score(match_histories,
	// 									  match_target_vals,
	// 									  new_average_score,
	// 									  new_factor_inputs,
	// 									  new_factor_input_averages,
	// 									  new_factor_input_standard_deviations,
	// 									  new_factor_weights,
	// 									  new_network_inputs,
	// 									  new_network,
	// 									  new_highest_signal);
	// 		if (is_success && new_highest_signal > best_highest_signal) {
	// 			best_highest_signal = new_highest_signal;

	// 			{
	// 				Factor* new_factor = new Factor();
	// 				new_factor->inputs = new_match_inputs;
	// 				new_factor->network = new_match_network;
	// 				new_factor->is_meaningful = true;

	// 				scope->factors.push_back(new_factor);

	// 				new_factor->link((int)scope->factors.size()-1);

	// 				scope->check_match = true;
	// 				scope->match_factor_index = (int)scope->factors.size()-1;

	// 				new_match_network = NULL;
	// 			}

	// 			if (new_network != NULL) {
	// 				Factor* new_factor = new Factor();
	// 				new_factor->inputs = new_network_inputs;
	// 				new_factor->network = new_network;
	// 				new_factor->is_meaningful = true;

	// 				scope->factors.push_back(new_factor);

	// 				new_factor->link((int)scope->factors.size()-1);

	// 				Input new_input;
	// 				new_input.scope_context = {scope};
	// 				new_input.factor_index = (int)scope->factors.size()-1;
	// 				new_input.node_context = {-1};
	// 				new_input.obs_index = -1;

	// 				new_factor_inputs.push_back(new_input);
	// 				new_factor_input_averages.push_back(0.0);
	// 				new_factor_input_standard_deviations.push_back(1.0);
	// 				new_factor_weights.push_back(1.0);

	// 				new_network = NULL;
	// 			}

	// 			scope->score_average_val = new_average_score;
	// 			scope->score_inputs = new_factor_inputs;
	// 			scope->score_input_averages = new_factor_input_averages;
	// 			scope->score_input_standard_deviations = new_factor_input_standard_deviations;
	// 			scope->score_weights = new_factor_weights;
	// 		}

	// 		if (new_network != NULL) {
	// 			delete new_network;
	// 		}
	// 	}

	// 	if (new_match_network != NULL) {
	// 		delete new_match_network;
	// 	}
	// }
}

void update_reward_signals(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)it->second;
				if (scope_node->explore_scope_histories.size() >= EXPLORE_TARGET_NUM_SAMPLES) {
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
