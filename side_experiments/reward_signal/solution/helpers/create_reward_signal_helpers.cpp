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

#include <limits>

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

void gather_existing_histories_helper(ScopeHistory* scope_history,
									  Scope* signal_scope,
									  vector<ScopeHistory*>& existing_scope_histories) {
	Scope* scope = scope_history->scope;

	if (scope == signal_scope) {
		existing_scope_histories.push_back(scope_history);
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == signal_scope) {
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
													 signal_scope,
													 existing_scope_histories);
				}
			}
		}
	}
}

void create_reward_signal_helper(Scope* scope,
								 SolutionWrapper* wrapper) {
	double best_highest_signal = numeric_limits<double>::lowest();
	if (scope->score_inputs.size() > 0) {
		for (int h_index = 0; h_index < (int)scope->explore_scope_histories.size(); h_index++) {
			ScopeHistory* scope_history = scope->explore_scope_histories[h_index];

			bool is_match;
			if (scope->check_match) {
				if (!scope_history->factor_initialized[scope->match_factor_index]) {
					double value = scope->factors[scope->match_factor_index]->back_activate(scope_history);
					scope_history->factor_initialized[scope->match_factor_index] = true;
					scope_history->factor_values[scope->match_factor_index] = value;
				}

				double val = scope_history->factor_values[scope->match_factor_index];

				if (val > 0.0) {
					is_match = true;
				}
			} else {
				is_match = true;
			}

			if (is_match) {
				double signal = calc_reward_signal(scope_history);
				if (signal > best_highest_signal) {
					best_highest_signal = signal;
				}
			}
		}
	}

	{
		double new_average_score;
		vector<Input> new_factor_inputs;
		vector<double> new_factor_input_averages;
		vector<double> new_factor_input_standard_deviations;
		vector<double> new_factor_weights;
		vector<Input> new_network_inputs;
		Network* new_network = NULL;
		double new_highest_signal;
		bool is_success = train_score(scope->explore_scope_histories,
									  scope->explore_target_val_histories,
									  new_average_score,
									  new_factor_inputs,
									  new_factor_input_averages,
									  new_factor_input_standard_deviations,
									  new_factor_weights,
									  new_network_inputs,
									  new_network,
									  new_highest_signal);
		if (is_success && new_highest_signal > best_highest_signal) {
			best_highest_signal = new_highest_signal;

			scope->check_match = false;
			scope->match_factor_index = -1;

			if (new_network != NULL) {
				Factor* new_factor = new Factor();
				new_factor->inputs = new_network_inputs;
				new_factor->network = new_network;
				new_factor->is_meaningful = true;

				scope->factors.push_back(new_factor);

				new_factor->link((int)scope->factors.size()-1);

				Input new_input;
				new_input.scope_context = {scope};
				new_input.factor_index = (int)scope->factors.size()-1;
				new_input.node_context = {-1};
				new_input.obs_index = -1;

				new_factor_inputs.push_back(new_input);
				new_factor_input_averages.push_back(0.0);
				new_factor_input_standard_deviations.push_back(1.0);
				new_factor_weights.push_back(1.0);

				new_network = NULL;
			}

			scope->score_average_val = new_average_score;
			scope->score_inputs = new_factor_inputs;
			scope->score_input_averages = new_factor_input_averages;
			scope->score_input_standard_deviations = new_factor_input_standard_deviations;
			scope->score_weights = new_factor_weights;
		}

		if (new_network != NULL) {
			delete new_network;
		}
	}

	vector<ScopeHistory*> existing_scope_histories;
	for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
		gather_existing_histories_helper(wrapper->solution->existing_scope_histories[h_index],
										 scope,
										 existing_scope_histories);
	}

	for (int t_index = 0; t_index < SPLIT_NUM_TRIES; t_index++) {
		vector<Input> new_match_inputs;
		Network* new_match_network = NULL;
		bool split_is_success = split_helper(existing_scope_histories,
											 scope->explore_scope_histories,
											 new_match_inputs,
											 new_match_network);

		if (split_is_success) {
			vector<ScopeHistory*> match_histories;
			vector<double> match_target_vals;
			for (int h_index = 0; h_index < (int)scope->explore_scope_histories.size(); h_index++) {
				vector<double> input_vals(new_match_inputs.size());
				vector<bool> input_is_on(new_match_inputs.size());
				for (int i_index = 0; i_index < (int)new_match_inputs.size(); i_index++) {
					double val;
					bool is_on;
					fetch_input_helper(scope->explore_scope_histories[h_index],
									   new_match_inputs[i_index],
									   0,
									   val,
									   is_on);
					input_vals[i_index] = val;
					input_is_on[i_index] = is_on;
				}
				new_match_network->activate(input_vals,
											input_is_on);

				if (new_match_network->output->acti_vals[0] > 0.0) {
					match_histories.push_back(scope->explore_scope_histories[h_index]);
					match_target_vals.push_back(scope->explore_target_val_histories[h_index]);
				}
			}

			double new_average_score;
			vector<Input> new_factor_inputs;
			vector<double> new_factor_input_averages;
			vector<double> new_factor_input_standard_deviations;
			vector<double> new_factor_weights;
			vector<Input> new_network_inputs;
			Network* new_network = NULL;
			double new_highest_signal;
			bool is_success = train_score(match_histories,
										  match_target_vals,
										  new_average_score,
										  new_factor_inputs,
										  new_factor_input_averages,
										  new_factor_input_standard_deviations,
										  new_factor_weights,
										  new_network_inputs,
										  new_network,
										  new_highest_signal);
			if (is_success && new_highest_signal > best_highest_signal) {
				best_highest_signal = new_highest_signal;

				{
					Factor* new_factor = new Factor();
					new_factor->inputs = new_match_inputs;
					new_factor->network = new_match_network;
					new_factor->is_meaningful = true;

					scope->factors.push_back(new_factor);

					new_factor->link((int)scope->factors.size()-1);

					scope->check_match = true;
					scope->match_factor_index = (int)scope->factors.size()-1;

					new_match_network = NULL;
				}

				if (new_network != NULL) {
					Factor* new_factor = new Factor();
					new_factor->inputs = new_network_inputs;
					new_factor->network = new_network;
					new_factor->is_meaningful = true;

					scope->factors.push_back(new_factor);

					new_factor->link((int)scope->factors.size()-1);

					Input new_input;
					new_input.scope_context = {scope};
					new_input.factor_index = (int)scope->factors.size()-1;
					new_input.node_context = {-1};
					new_input.obs_index = -1;

					new_factor_inputs.push_back(new_input);
					new_factor_input_averages.push_back(0.0);
					new_factor_input_standard_deviations.push_back(1.0);
					new_factor_weights.push_back(1.0);

					new_network = NULL;
				}

				scope->score_average_val = new_average_score;
				scope->score_inputs = new_factor_inputs;
				scope->score_input_averages = new_factor_input_averages;
				scope->score_input_standard_deviations = new_factor_input_standard_deviations;
				scope->score_weights = new_factor_weights;
			}

			if (new_network != NULL) {
				delete new_network;
			}
		}

		if (new_match_network != NULL) {
			delete new_match_network;
		}
	}
}
