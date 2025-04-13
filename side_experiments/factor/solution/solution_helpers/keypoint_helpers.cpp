#include "solution_helpers.h"

#include <algorithm>

#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "keypoint.h"
#include "network.h"
#include "nn_helpers.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "obs_node.h"

using namespace std;

const double KEYPOINT_MAX_MISGUESS_RATIO = 0.1;

void gather_possible_scopes_helper(ScopeHistory* scope_history,
								   map<Scope*, vector<ScopeHistory*>>& possible_scopes) {
	map<Scope*, vector<ScopeHistory*>>::iterator it = possible_scopes.find(scope_history->scope);
	if (it == possible_scopes.end()) {
		possible_scopes[scope_history->scope] = {scope_history};
	} else {
		it->second.push_back(scope_history);
	}

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
			gather_possible_scopes_helper(scope_node_history->scope_history,
										  possible_scopes);
		}
	}
}

void keypoint_gather_inputs_helper(ScopeHistory* scope_history,
								   ObsNode* explore_node,
								   int explore_obs_index,
								   vector<Input>& inputs,
								   vector<vector<double>>& input_histories,
								   vector<double>& target_val_histories,
								   int num_miss) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
			keypoint_gather_inputs_helper(scope_node_history->scope_history,
										  explore_node,
										  explore_obs_index,
										  inputs,
										  input_histories,
										  target_val_histories,
										  num_miss);
		}
	}

	if (scope_history->scope == explore_node->parent) {
		map<int, AbstractNodeHistory*>::iterator keypoint_it = scope_history
			->node_histories.find(explore_node->id);
		if (keypoint_it != scope_history->node_histories.end()) {
			bool has_dependencies = true;
			vector<double> input_vals(inputs.size());
			for (int i_index = 0; i_index < (int)inputs.size(); i_index++) {
				bool hit;
				fetch_input_helper(scope_history,
								   inputs[i_index],
								   0,
								   hit,
								   input_vals[i_index]);
				if (!hit) {
					has_dependencies = false;
					break;
				}
			}

			if (has_dependencies) {
				input_histories.push_back(input_vals);
				target_val_histories.push_back(
					((ObsNodeHistory*)(keypoint_it->second))->obs_history[explore_obs_index]);
			} else {
				num_miss++;
			}
		}
	}
}

void keypoint_experiment(vector<ScopeHistory*>& scope_histories) {
	map<Scope*, vector<ScopeHistory*>> possible_scopes;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		gather_possible_scopes_helper(scope_histories[h_index],
									  possible_scopes);
	}
	uniform_int_distribution<int> scope_distribution(0, possible_scopes.size()-1);
	map<Scope*, vector<ScopeHistory*>>::iterator it = next(possible_scopes.begin(), scope_distribution(generator));
	uniform_int_distribution<int> scope_history_distribution(0, it->second.size()-1);
	ScopeHistory* explore_scope_history = it->second[scope_history_distribution(generator)];
	Scope* explore_scope = explore_scope_history->scope;

	vector<AbstractNodeHistory*> possible_nodes;
	for (map<int, AbstractNodeHistory*>::iterator h_it = explore_scope_history->node_histories.begin();
			h_it != explore_scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->type == NODE_TYPE_OBS) {
			possible_nodes.push_back(h_it->second);
		}
	}
	uniform_int_distribution<int> node_distribution(0, possible_nodes.size()-1);
	ObsNodeHistory* explore_node_history = (ObsNodeHistory*)possible_nodes[node_distribution(generator)];
	ObsNode* explore_node = (ObsNode*)explore_node_history->node;

	uniform_int_distribution<int> obs_distribution(0, problem_type->num_obs()-1);
	int explore_obs_index = obs_distribution(generator);

	vector<Input> inputs;

	geometric_distribution<int> num_input_distribution(0.3);
	int num_inputs = num_input_distribution(generator);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		vector<Scope*> scope_context;
		vector<int> node_context;
		int node_count = 0;
		Input new_input;
		for (map<int, AbstractNodeHistory*>::iterator it = explore_scope_history->node_histories.begin();
				it != explore_scope_history->node_histories.end(); it++) {
			if (it->second->index < explore_node_history->index) {
				AbstractNode* node = it->second->node;
				switch (node->type) {
				case NODE_TYPE_SCOPE:
					{
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

						scope_context.push_back(explore_scope);
						node_context.push_back(it->first);

						gather_possible_helper(scope_node_history->scope_history,
											   scope_context,
											   node_context,
											   node_count,
											   new_input);

						scope_context.pop_back();
						node_context.pop_back();
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							scope_context.push_back(explore_scope);
							node_context.push_back(it->first);

							new_input.scope_context = scope_context;
							new_input.node_context = node_context;
							new_input.factor_index = -1;
							new_input.obs_index = -1;

							scope_context.pop_back();
							node_context.pop_back();
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)node;

						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							scope_context.push_back(explore_scope);
							node_context.push_back(it->first);

							new_input.scope_context = scope_context;
							new_input.node_context = node_context;
							new_input.factor_index = -1;
							new_input.obs_index = obs_distribution(generator);

							scope_context.pop_back();
							node_context.pop_back();
						}

						for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
							if (obs_node->factors[f_index]->inputs.size() > 0) {
								uniform_int_distribution<int> select_distribution(0, node_count);
								node_count++;
								if (select_distribution(generator) == 0) {
									scope_context.push_back(explore_scope);
									node_context.push_back(it->first);

									new_input.scope_context = scope_context;
									new_input.node_context = node_context;
									new_input.factor_index = f_index;
									new_input.obs_index = -1;

									scope_context.pop_back();
									node_context.pop_back();
								}
							}
						}
					}
					break;
				}
			}
		}

		bool is_existing = false;
		for (int i_index = 0; i_index < (int)inputs.size(); i_index++) {
			if (new_input == inputs[i_index]) {
				is_existing = true;
				break;
			}
		}
		if (!is_existing) {
			inputs.push_back(new_input);
		}
	}

	vector<vector<double>> input_histories;
	vector<double> target_val_histories;
	int num_miss = 0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		keypoint_gather_inputs_helper(scope_histories[h_index],
									  explore_node,
									  explore_obs_index,
									  inputs,
									  input_histories,
									  target_val_histories,
									  num_miss);
	}

	int num_hit = (int)input_histories.size();
	int num_total = (int)input_histories.size() + num_miss;
	double new_availability = (double)num_hit / (double)num_total;

	bool higher_availability = false;
	if (explore_node->keypoints[explore_obs_index] == NULL) {
		higher_availability = true;
	} else {
		if (new_availability > explore_node->keypoints[explore_obs_index]->availability) {
			higher_availability = true;
		}
	}

	if (higher_availability) {
		{
			default_random_engine generator_copy = generator;
			shuffle(input_histories.begin(), input_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(target_val_histories.begin(), target_val_histories.end(), generator_copy);
		}

		Network* network = new Network(inputs.size());

		train_network(input_histories,
					  target_val_histories,
					  network);

		double new_average_misguess;
		double new_misguess_standard_deviation;
		measure_network(input_histories,
						target_val_histories,
						network,
						new_average_misguess,
						new_misguess_standard_deviation);

		double obs_misguess = sqrt(solution->obs_variances[explore_obs_index]);
		if (new_average_misguess < KEYPOINT_MAX_MISGUESS_RATIO * obs_misguess) {
			if (explore_node->keypoints[explore_obs_index] != NULL) {
				delete explore_node->keypoints[explore_obs_index];
			}

			Keypoint* new_keypoint = new Keypoint();
			new_keypoint->inputs = inputs;
			new_keypoint->network = network;
			new_keypoint->misguess_standard_deviation = new_misguess_standard_deviation;
			new_keypoint->availability = new_availability;
			explore_node->keypoints[explore_obs_index] = new_keypoint;
		} else {
			delete network;
		}
	}
}
