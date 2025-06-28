#include "solution_helpers.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

void node_history_insert(ScopeHistory* scope_history,
						 AbstractNode* explore_node,
						 bool is_branch,
						 ObsNode* new_node) {
	bool has_match = false;

	map<int, AbstractNodeHistory*>::iterator match_it = scope_history->node_histories.find(explore_node->id);
	if (match_it != scope_history->node_histories.end()) {
		if (explore_node->type == NODE_TYPE_BRANCH) {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)match_it->second;
			if (branch_node_history->is_branch == is_branch) {
				has_match = true;
			}
		} else {
			has_match = true;
		}
	}

	if (has_match) {
		vector<double> new_obs_history;
		for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
				it != scope_history->node_histories.end(); it++) {
			if (it->second->index == match_it->second->index + 1) {
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				new_obs_history = obs_node_history->obs_history;
			}

			if (it->second->index > match_it->second->index) {
				it->second->index++;
			}
		}

		ObsNodeHistory* new_history = new ObsNodeHistory(new_node);
		new_history->index = match_it->second->index + 1;
		scope_history->node_histories[new_node->id] = new_history;

		new_history->obs_history = new_obs_history;

		new_history->factor_initialized = vector<bool>(new_node->factors.size(), false);
		new_history->factor_values = vector<double>(new_node->factors.size());
	}
}

void existing_add_factor(vector<ScopeHistory*>& scope_histories,
						 vector<Input>& network_inputs,
						 Network* network,
						 Input& new_input,
						 AbstractExperiment* experiment) {
	Factor* new_factor = new Factor();
	new_factor->inputs = network_inputs;
	new_factor->network = network;
	if (experiment->node_context->type == NODE_TYPE_OBS) {
		ObsNode* obs_node = (ObsNode*)experiment->node_context;

		obs_node->factors.push_back(new_factor);

		new_input.scope_context = {obs_node->parent};
		new_input.node_context = {obs_node->id};
		new_input.factor_index = (int)obs_node->factors.size()-1;
		new_input.obs_index = -1;

		for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
			map<int, AbstractNodeHistory*>::iterator it = scope_histories[h_index]
				->node_histories.find(obs_node->id);
			if (it != scope_histories[h_index]->node_histories.end()) {
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				obs_node_history->factor_initialized.push_back(false);
				obs_node_history->factor_values.push_back(0.0);
			}
		}
	} else {
		ObsNode* new_obs_node = new ObsNode();
		new_obs_node->parent = experiment->scope_context;
		new_obs_node->id = experiment->scope_context->node_counter;
		experiment->scope_context->node_counter++;
		experiment->scope_context->nodes[new_obs_node->id] = new_obs_node;

		new_obs_node->factors.push_back(new_factor);

		switch (experiment->node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)experiment->node_context;

				new_obs_node->next_node_id = action_node->next_node_id;
				new_obs_node->next_node = action_node->next_node;

				for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
					if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
						action_node->next_node->ancestor_ids.erase(
							action_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				action_node->next_node->ancestor_ids.push_back(new_obs_node->id);

				action_node->next_node_id = new_obs_node->id;
				action_node->next_node = new_obs_node;

				new_obs_node->ancestor_ids.push_back(action_node->id);

				new_obs_node->average_hits_per_run = action_node->average_hits_per_run;
				new_obs_node->average_instances_per_run = action_node->average_instances_per_run;
				new_obs_node->average_score = action_node->average_score;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)experiment->node_context;

				new_obs_node->next_node_id = scope_node->next_node_id;
				new_obs_node->next_node = scope_node->next_node;

				for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
					if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
						scope_node->next_node->ancestor_ids.erase(
							scope_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				scope_node->next_node->ancestor_ids.push_back(new_obs_node->id);

				scope_node->next_node_id = new_obs_node->id;
				scope_node->next_node = new_obs_node;

				new_obs_node->ancestor_ids.push_back(scope_node->id);

				new_obs_node->average_hits_per_run = scope_node->average_hits_per_run;
				new_obs_node->average_instances_per_run = scope_node->average_instances_per_run;
				new_obs_node->average_score = scope_node->average_score;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)experiment->node_context;

				if (experiment->is_branch) {
					new_obs_node->next_node_id = branch_node->branch_next_node_id;
					new_obs_node->next_node = branch_node->branch_next_node;

					for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->branch_next_node->ancestor_ids.erase(
								branch_node->branch_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					branch_node->branch_next_node->ancestor_ids.push_back(new_obs_node->id);

					branch_node->branch_next_node_id = new_obs_node->id;
					branch_node->branch_next_node = new_obs_node;

					new_obs_node->average_hits_per_run = branch_node->branch_average_hits_per_run;
					new_obs_node->average_instances_per_run = branch_node->branch_average_instances_per_run;
					new_obs_node->average_score = branch_node->branch_average_score;
				} else {
					new_obs_node->next_node_id = branch_node->original_next_node_id;
					new_obs_node->next_node = branch_node->original_next_node;

					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(
								branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					branch_node->original_next_node->ancestor_ids.push_back(new_obs_node->id);

					branch_node->original_next_node_id = new_obs_node->id;
					branch_node->original_next_node = new_obs_node;

					new_obs_node->average_hits_per_run = branch_node->original_average_hits_per_run;
					new_obs_node->average_instances_per_run = branch_node->original_average_instances_per_run;
					new_obs_node->average_score = branch_node->original_average_score;
				}

				new_obs_node->ancestor_ids.push_back(branch_node->id);
			}
			break;
		}

		new_input.scope_context = {experiment->scope_context};
		new_input.node_context = {new_obs_node->id};
		new_input.factor_index = 0;
		new_input.obs_index = -1;

		for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
			node_history_insert(scope_histories[h_index],
								experiment->node_context,
								experiment->is_branch,
								new_obs_node);
		}

		experiment->node_context->experiment = NULL;

		experiment->node_context = new_obs_node;
		experiment->is_branch = false;
		experiment->node_context->experiment = experiment;
	}
}
