#include "signal_helpers.h"

#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "signal.h"
#include "signal_node.h"

using namespace std;

const int SAMPLES_PER_RUN = 5;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_MIN_SAMPLES = 20;
#else
const int TRAIN_MIN_SAMPLES = 20000;
#endif /* MDEBUG */
const int ITERS_PER_SAMPLE = 2;

void random_explore_index_helper(ScopeHistory* scope_history,
								 vector<int>& explore_index) {
	uniform_int_distribution<int> distribution(0, scope_history->node_histories.size()-1);
	int index = distribution(generator);

	explore_index.push_back(index);

	AbstractNodeHistory* node_history = next(scope_history->node_histories.begin(), index)->second;
	if (node_history->node->type == NODE_TYPE_SCOPE) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
		random_explore_index_helper(scope_node_history->scope_history,
									explore_index);
	}
}

void random_signal_samples_helper(ScopeHistory* scope_history,
								  map<Scope*, pair<int, ScopeHistory*>>& samples) {
	Scope* scope = scope_history->scope;

	map<Scope*, pair<int, ScopeHistory*>>::iterator it = samples.find(scope);
	if (it == samples.end()) {
		samples[scope] = {1, scope_history};
	} else {
		uniform_int_distribution<int> distribution(0, it->second.first);
		if (distribution(generator) == 0) {
			it->second.second = scope_history;
		}
		it->second.first++;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			random_signal_samples_helper(scope_node_history->scope_history,
										 samples);
		}
	}
}

void pre_signal_add_sample(ScopeHistory* scope_history,
						   vector<int>& explore_index,
						   double target_val,
						   SolutionWrapper* wrapper) {
	Signal* signal = scope_history->scope->pre_signal;

	vector<vector<double>> node_input_vals;
	vector<vector<bool>> node_input_is_on;
	for (int n_index = 0; n_index < (int)signal->nodes.size(); n_index++) {
		vector<double> input_vals(signal->nodes[n_index]->inputs.size());
		vector<bool> input_is_on(signal->nodes[n_index]->inputs.size());
		for (int i_index = 0; i_index < (int)signal->nodes[n_index]->inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   signal->nodes[n_index]->inputs[i_index],
							   explore_index,
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		node_input_vals.push_back(input_vals);
		node_input_is_on.push_back(input_is_on);
	}

	vector<vector<double>> potential_input_vals;
	vector<vector<bool>> potential_input_is_on;
	for (int p_index = 0; p_index < (int)signal->potential_inputs.size(); p_index++) {
		vector<double> input_vals(signal->potential_inputs[p_index].size());
		vector<bool> input_is_on(signal->potential_inputs[p_index].size());
		for (int i_index = 0; i_index < (int)signal->potential_inputs[p_index].size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   signal->potential_inputs[p_index][i_index],
							   explore_index,
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		potential_input_vals.push_back(input_vals);
		potential_input_is_on.push_back(input_is_on);
	}

	if (signal->input_val_histories.size() < TRAIN_MIN_SAMPLES) {
		signal->input_val_histories.push_back(node_input_vals);
		signal->input_is_on_histories.push_back(node_input_is_on);
		signal->target_val_histories.push_back(target_val);

		signal->potential_input_val_histories.push_back(potential_input_vals);
		signal->potential_input_is_on_histories.push_back(potential_input_is_on);
	} else {
		signal->input_val_histories[signal->history_index] = node_input_vals;
		signal->input_is_on_histories[signal->history_index] = node_input_is_on;
		signal->target_val_histories[signal->history_index] = target_val;

		signal->potential_input_val_histories[signal->history_index] = potential_input_vals;
		signal->potential_input_is_on_histories[signal->history_index] = potential_input_is_on;

		signal->history_index++;
		if (signal->history_index >= SIGNAL_NUM_SAMPLES) {
			signal->history_index = 0;
		}
	}
	signal->potential_count++;

	if (signal->input_val_histories.size() >= TRAIN_MIN_SAMPLES) {
		uniform_int_distribution<int> sample_distribution(1, TRAIN_MIN_SAMPLES);
		for (int s_index = 0; s_index < ITERS_PER_SAMPLE; s_index++) {
			int index = signal->history_index - sample_distribution(generator);
			if (index < 0) {
				index += SIGNAL_NUM_SAMPLES;
			}
			signal->backprop_helper(index);
		}
	}

	if (signal->potential_count >= SIGNAL_NUM_SAMPLES) {
		update_pre_signal(scope_history->scope,
						  wrapper);
	}
}

void update_existing_signals_helper(ScopeHistory* scope_history,
									double target_val,
									SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < SAMPLES_PER_RUN; s_index++) {
		map<Scope*, pair<int, ScopeHistory*>> samples;
		random_signal_samples_helper(scope_history,
									 samples);

		for (map<Scope*, pair<int, ScopeHistory*>>::iterator it = samples.begin();
				it != samples.end(); it++) {
			vector<int> explore_index;
			random_explore_index_helper(it->second.second,
										explore_index);

			pre_signal_add_sample(it->second.second,
								  explore_index,
								  target_val,
								  wrapper);
		}
	}
}
