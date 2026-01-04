/**
 * - learn before measuring diff to handle e.g., XORs
 */

#include "solution_helpers.h"

#include <algorithm>
#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"
#include "tunnel.h"

using namespace std;

const int TRUE_WEIGHT = 4;
const double MIN_TRUE_RATIO = 0.25;

const int FIND_NUM_TRIES = 100;

void gather_helper(ScopeHistory* scope_history,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
		case NODE_TYPE_ACTION:
		case NODE_TYPE_OBS:
			{
				uniform_int_distribution<int> select_distribution(0, node_count);
				node_count++;
				if (select_distribution(generator) == 0) {
					explore_node = node;
					explore_is_branch = false;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				gather_helper(scope_node_history->scope_history,
							  node_count,
							  explore_node,
							  explore_is_branch);

				{
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				if (branch_node_history->is_branch) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = true;
					}
				} else {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = node;
						explore_is_branch = false;
					}
				}
			}
			break;
		}
	}
}

void select_parent_tunnel_helper(Scope* scope_context,
								 Scope*& parent_tunnel_parent,
								 int& parent_tunnel_index) {
	uniform_int_distribution<int> stack_trace_distribution(0, scope_context->explore_stack_traces.size()-1);
	int stack_trace_index = stack_trace_distribution(generator);
	vector<pair<Scope*, int>> possible_parents;
	for (int l_index = 0; l_index < (int)scope_context->explore_stack_traces[stack_trace_index].size()-1; l_index++) {
		/**
		 * - don't include scope_context's tunnels for now
		 */
		Scope* scope = scope_context->explore_stack_traces[stack_trace_index][l_index]->scope;
		for (int t_index = 0; t_index < (int)scope->tunnels.size(); t_index++) {
			if (scope->tunnels[t_index]->is_long_term()) {
				possible_parents.push_back({scope, t_index});
			}
		}
	}
	bool is_true;
	if (TRUE_WEIGHT / (TRUE_WEIGHT + possible_parents.size()) < MIN_TRUE_RATIO) {
		uniform_int_distribution<int> distribution(0, 3);
		if (distribution(generator) == 0) {
			is_true = true;
		} else {
			is_true = false;
		}
	} else {
		uniform_int_distribution<int> distribution(0, TRUE_WEIGHT + possible_parents.size() - 1);
		if (distribution(generator) < TRUE_WEIGHT) {
			is_true = true;
		} else {
			is_true = false;
		}
	}
	if (is_true) {
		parent_tunnel_parent = NULL;
		parent_tunnel_index = -1;
	} else {
		uniform_int_distribution<int> parent_distribution(0, possible_parents.size() - 1);
		int index = parent_distribution(generator);
		parent_tunnel_parent = possible_parents[index].first;
		parent_tunnel_index = possible_parents[index].second;
	}
}

void gather_training_data_helper(Scope* scope_context,
								 vector<vector<double>>& obs_vals,
								 vector<double>& target_vals) {
	Scope* parent_tunnel_parent;
	int parent_tunnel_index;
	select_parent_tunnel_helper(scope_context,
								parent_tunnel_parent,
								parent_tunnel_index);

	for (int h_index = 0; h_index < (int)scope_context->explore_stack_traces.size(); h_index++) {
		if (parent_tunnel_parent == NULL) {
			obs_vals.push_back(scope_context->explore_stack_traces[h_index].back()->obs_history);
			target_vals.push_back(scope_context->explore_target_val_histories[h_index]);
		} else {
			for (int l_index = 0; l_index < (int)scope_context->explore_stack_traces[h_index].size(); l_index++) {
				ScopeHistory* scope_history = scope_context->explore_stack_traces[h_index][l_index];
				if (scope_history->scope == parent_tunnel_parent) {
					if (!scope_history->tunnel_is_init[parent_tunnel_index]) {
						scope_history->tunnel_is_init[parent_tunnel_index] = true;
						Tunnel* tunnel = scope_history->scope->tunnels[parent_tunnel_index];
						scope_history->tunnel_vals[parent_tunnel_index] = tunnel->get_signal(scope_history->obs_history);
					}
					obs_vals.push_back(scope_context->explore_stack_traces[h_index].back()->obs_history);
					target_vals.push_back(scope_history->tunnel_vals[parent_tunnel_index]);

					break;
				}
			}
		}
	}
}

void gather_tunnel_data_helper(ScopeHistory* scope_history,
							   Scope* scope_context,
							   vector<vector<double>>& obs) {
	Scope* scope = scope_history->scope;

	if (scope == scope_context) {
		obs.push_back(scope_history->obs_history);
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == scope_context) {
				is_child = true;
				break;
			}
		}
		if (is_child) {
			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
				if (it->second->node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					gather_tunnel_data_helper(scope_node_history->scope_history,
											  scope_context,
											  obs);
				}
			}
		}
	}
}

Tunnel* create_obs_candidate(vector<vector<double>>& obs_vals,
							 vector<double>& target_vals) {
	geometric_distribution<int> num_obs_distribution(0.5);
	int num_obs;
	while (true) {
		num_obs = 1 + num_obs_distribution(generator);
		if (num_obs <= (int)obs_vals[0].size()) {
			break;
		}
	}

	vector<int> remaining_indexes(obs_vals[0].size());
	for (int i_index = 0; i_index < (int)obs_vals[0].size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	vector<int> obs_indexes;
	for (int o_index = 0; o_index < num_obs; o_index++) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);
		obs_indexes.push_back(remaining_indexes[index]);
		remaining_indexes.erase(remaining_indexes.begin() + index);
	}

	vector<vector<double>> inputs(obs_vals.size());
	for (int h_index = 0; h_index < (int)obs_vals.size(); h_index++) {
		inputs[h_index] = vector<double>(num_obs);
		for (int o_index = 0; o_index < num_obs; o_index++) {
			inputs[h_index][o_index] = obs_vals[h_index][obs_indexes[o_index]];
		}
	}

	Network* signal_network = new Network(num_obs,
										  NETWORK_SIZE_SMALL);
	uniform_int_distribution<int> sample_distribution(0, inputs.size()-1);
	#if defined(MDEBUG) && MDEBUG
	for (int iter_index = 0; iter_index < 30; iter_index++) {
	#else
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
	#endif /* MDEBUG */
		int index = sample_distribution(generator);

		signal_network->activate(inputs[index]);

		double error = target_vals[index] - signal_network->output->acti_vals[0];

		signal_network->backprop(error);
	}

	Tunnel* new_tunnel = new Tunnel(obs_indexes,
									false,
									NULL,
									signal_network);

	return new_tunnel;
}

Tunnel* create_pattern_candidate(vector<vector<double>>& starting_existing_obs_vals,
								 vector<vector<double>>& ending_existing_obs_vals,
								 vector<vector<double>>& explore_obs_vals,
								 vector<double>& explore_target_vals) {
	// geometric_distribution<int> num_obs_distribution(0.3);
	geometric_distribution<int> num_obs_distribution(0.2);
	int num_obs;
	while (true) {
		num_obs = 2 + num_obs_distribution(generator);
		if (num_obs <= (int)explore_obs_vals[0].size()) {
			break;
		}
	}

	vector<int> remaining_indexes(explore_obs_vals[0].size());
	for (int i_index = 0; i_index < (int)explore_obs_vals[0].size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	vector<int> obs_indexes;
	for (int o_index = 0; o_index < num_obs; o_index++) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);
		obs_indexes.push_back(remaining_indexes[index]);
		remaining_indexes.erase(remaining_indexes.begin() + index);
	}

	vector<vector<double>> explore_inputs(explore_obs_vals.size());
	for (int h_index = 0; h_index < (int)explore_obs_vals.size(); h_index++) {
		explore_inputs[h_index] = vector<double>(num_obs);
		for (int o_index = 0; o_index < num_obs; o_index++) {
			explore_inputs[h_index][o_index] = explore_obs_vals[h_index][obs_indexes[o_index]];
		}
	}

	vector<vector<double>> existing_inputs;
	for (int h_index = 0; h_index < (int)starting_existing_obs_vals.size(); h_index++) {
		vector<double> inputs(num_obs);
		for (int o_index = 0; o_index < num_obs; o_index++) {
			inputs[o_index] = starting_existing_obs_vals[h_index][obs_indexes[o_index]];
		}
		existing_inputs.push_back(inputs);
	}
	for (int h_index = 0; h_index < (int)ending_existing_obs_vals.size(); h_index++) {
		vector<double> inputs(num_obs);
		for (int o_index = 0; o_index < num_obs; o_index++) {
			inputs[o_index] = ending_existing_obs_vals[h_index][obs_indexes[o_index]];
		}
		existing_inputs.push_back(inputs);
	}

	uniform_int_distribution<int> explore_sample_distribution(0, explore_inputs.size()-1);
	uniform_int_distribution<int> existing_sample_distribution(0, existing_inputs.size()-1);

	Network* similarity_network = new Network(num_obs,
											  NETWORK_SIZE_SMALL);

	uniform_int_distribution<int> existing_distribution(0, 1);
	#if defined(MDEBUG) && MDEBUG
	for (int iter_index = 0; iter_index < 40; iter_index++) {
	#else
	for (int iter_index = 0; iter_index < 40000; iter_index++) {
	#endif /* MDEBUG */
		if (existing_distribution(generator) == 0) {
			int index = existing_sample_distribution(generator);

			similarity_network->activate(existing_inputs[index]);

			if (similarity_network->output->acti_vals[0] < 1.0) {
				double error = 1.0 - similarity_network->output->acti_vals[0];
				similarity_network->backprop(error);
			}
		} else {
			int index = explore_sample_distribution(generator);

			similarity_network->activate(explore_inputs[index]);

			if (similarity_network->output->acti_vals[0] > 0.0) {
				double error = 0.0 - similarity_network->output->acti_vals[0];
				similarity_network->backprop(error);
			}
		}
	}

	vector<double> similarity_vals(explore_inputs.size());
	for (int h_index = 0; h_index < (int)explore_inputs.size(); h_index++) {
		similarity_network->activate(explore_inputs[h_index]);
		similarity_vals[h_index] = similarity_network->output->acti_vals[0];
	}

	Network* signal_network = new Network(num_obs,
										  NETWORK_SIZE_SMALL);
	#if defined(MDEBUG) && MDEBUG
	for (int iter_index = 0; iter_index < 30; iter_index++) {
	#else
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
	#endif /* MDEBUG */
		int index = explore_sample_distribution(generator);

		double similarity = similarity_vals[index];

		if (similarity > 0.0) {
			if (similarity > 1.0) {
				similarity = 1.0;
			}

			signal_network->activate(explore_inputs[index]);

			double error = explore_target_vals[index] - signal_network->output->acti_vals[0];
			double adjusted = similarity * error;

			signal_network->backprop(adjusted);
		}
	}

	Tunnel* new_tunnel = new Tunnel(obs_indexes,
									true,
									similarity_network,
									signal_network);

	return new_tunnel;
}

const int MAX_CANDIDATES = 5;
void find_potential_tunnels(vector<ScopeHistory*>& starting_scope_histories,
							vector<ScopeHistory*>& ending_scope_histories,
							SolutionWrapper* wrapper) {
	vector<pair<double, pair<int,Tunnel*>>> potential_candidates;
	for (int try_index = 0; try_index < FIND_NUM_TRIES; try_index++) {
		uniform_int_distribution<int> gather_sample_distribution(0, ending_scope_histories.size()-1);

		int node_count = 0;
		AbstractNode* explore_node = NULL;
		bool explore_is_branch = false;
		gather_helper(ending_scope_histories[gather_sample_distribution(generator)],
					  node_count,
					  explore_node,
					  explore_is_branch);

		Scope* scope_context = explore_node->parent;

		if (scope_context->explore_stack_traces.size() > 0) {
			vector<vector<double>> explore_obs_vals;
			vector<double> explore_target_vals;
			gather_training_data_helper(scope_context,
										explore_obs_vals,
										explore_target_vals);

			vector<vector<double>> starting_existing_obs_vals;
			for (int h_index = 0; h_index < (int)starting_scope_histories.size(); h_index++) {
				gather_tunnel_data_helper(starting_scope_histories[h_index],
										  scope_context,
										  starting_existing_obs_vals);
			}

			vector<vector<double>> ending_existing_obs_vals;
			for (int h_index = 0; h_index < (int)ending_scope_histories.size(); h_index++) {
				gather_tunnel_data_helper(ending_scope_histories[h_index],
										  scope_context,
										  ending_existing_obs_vals);
			}

			Tunnel* candidate;
			uniform_int_distribution<int> pattern_distribution(0, 1);
			// if (pattern_distribution(generator) == 0) {
			if (true) {
				candidate = create_pattern_candidate(starting_existing_obs_vals,
													 ending_existing_obs_vals,
													 explore_obs_vals,
													 explore_target_vals);
			} else {
				candidate = create_obs_candidate(explore_obs_vals,
												 explore_target_vals);
			}

			vector<double> starting_vals(starting_existing_obs_vals.size());
			for (int h_index = 0; h_index < (int)starting_existing_obs_vals.size(); h_index++) {
				starting_vals[h_index] = candidate->get_signal(starting_existing_obs_vals[h_index]);
			}
			vector<double> ending_vals(ending_existing_obs_vals.size());
			for (int h_index = 0; h_index < (int)ending_existing_obs_vals.size(); h_index++) {
				ending_vals[h_index] = candidate->get_signal(ending_existing_obs_vals[h_index]);
			}

			double starting_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)starting_vals.size(); h_index++) {
				starting_sum_vals += starting_vals[h_index];
			}
			double starting_val_average = starting_sum_vals / (double)starting_vals.size();

			double starting_sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)starting_vals.size(); h_index++) {
				starting_sum_variance += (starting_vals[h_index] - starting_val_average) * (starting_vals[h_index] - starting_val_average);
			}
			double starting_val_standard_deviation = sqrt(starting_sum_variance / (double)starting_vals.size());
			if (starting_val_standard_deviation < MIN_STANDARD_DEVIATION) {
				starting_val_standard_deviation = MIN_STANDARD_DEVIATION;
			}
			double starting_val_standard_error = starting_val_standard_deviation / sqrt((double)starting_vals.size());

			double ending_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)ending_vals.size(); h_index++) {
				ending_sum_vals += ending_vals[h_index];
			}
			double ending_val_average = ending_sum_vals / (double)ending_vals.size();

			double ending_sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)ending_vals.size(); h_index++) {
				ending_sum_variance += (ending_vals[h_index] - ending_val_average) * (ending_vals[h_index] - ending_val_average);
			}
			double ending_val_standard_deviation = sqrt(ending_sum_variance / (double)ending_vals.size());
			if (ending_val_standard_deviation < MIN_STANDARD_DEVIATION) {
				ending_val_standard_deviation = MIN_STANDARD_DEVIATION;
			}
			double ending_val_standard_error = ending_val_standard_deviation / sqrt((double)ending_vals.size());

			double denom = sqrt(starting_val_standard_error * starting_val_standard_error
				+ ending_val_standard_error * ending_val_standard_error);
			double t_score = (ending_val_average - starting_val_average) / denom;

			if (t_score >= 2.326) {
				cout << "t_score: " << t_score << endl;
				cout << "scope_context->id: " << scope_context->id << endl;
				candidate->print();

				potential_candidates.push_back({t_score, {scope_context->id, candidate}});
			} else {
				delete candidate;
			}
		}
	}

	if (potential_candidates.size() <= MAX_CANDIDATES) {
		for (int c_index = 0; c_index < (int)potential_candidates.size(); c_index++) {
			wrapper->candidates.push_back(potential_candidates[c_index].second);
		}
	} else {
		sort(potential_candidates.begin(), potential_candidates.end());

		for (int c_index = 0; c_index < (int)potential_candidates.size(); c_index++) {
			if (c_index + MAX_CANDIDATES >= (int)potential_candidates.size()) {
				wrapper->candidates.push_back(potential_candidates[c_index].second);
			} else {
				delete potential_candidates[c_index].second.second;
			}
		}
	}
}
