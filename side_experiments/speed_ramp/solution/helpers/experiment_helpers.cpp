#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "signal_experiment.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

/**
 * - prioritize exploring new nodes
 * 
 * - even if changes made later, unlikely to influence old
 *   - old still needs to also match everything else
 */
void gather_helper(ScopeHistory* scope_history,
				   AbstractNode*& node_context,
				   bool& is_branch,
				   Scope*& signal_scope,
				   int& potential_seen) {
	Scope* scope = scope_history->scope;
	if (scope->signal_experiment == NULL) {
		uniform_int_distribution<int> select_distribution(0, potential_seen);
		if (select_distribution(generator) == 0) {
			node_context = NULL;

			signal_scope = scope;
		}
		potential_seen++;
	}

	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		if (node->experiment == NULL) {
			switch (node->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)node;
					if (distribution(generator) <= 1.0 / (1.0 + sqrt(start_node->num_experiments))) {
						uniform_int_distribution<int> select_distribution(0, potential_seen);
						if (select_distribution(generator) == 0) {
							node_context = node;
							is_branch = false;

							signal_scope = NULL;
						}
						potential_seen++;
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)node;
					if (distribution(generator) <= 1.0 / (1.0 + sqrt(action_node->num_experiments))) {
						uniform_int_distribution<int> select_distribution(0, potential_seen);
						if (select_distribution(generator) == 0) {
							node_context = node;
							is_branch = false;

							signal_scope = NULL;
						}
						potential_seen++;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					gather_helper(scope_node_history->scope_history,
								  node_context,
								  is_branch,
								  signal_scope,
								  potential_seen);

					ScopeNode* scope_node = (ScopeNode*)node;
					if (distribution(generator) <= 1.0 / (1.0 + sqrt(scope_node->num_experiments))) {
						uniform_int_distribution<int> select_distribution(0, potential_seen);
						if (select_distribution(generator) == 0) {
							node_context = node;
							is_branch = false;

							signal_scope = NULL;
						}
						potential_seen++;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)node;
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch) {
						if (distribution(generator) <= 1.0 / (1.0 + sqrt(branch_node->branch_num_experiments))) {
							uniform_int_distribution<int> select_distribution(0, potential_seen);
							if (select_distribution(generator) == 0) {
								node_context = node;
								is_branch = true;

								signal_scope = NULL;
							}
							potential_seen++;
						}
					} else {
						if (distribution(generator) <= 1.0 / (1.0 + sqrt(branch_node->original_num_experiments))) {
							uniform_int_distribution<int> select_distribution(0, potential_seen);
							if (select_distribution(generator) == 0) {
								node_context = node;
								is_branch = false;

								signal_scope = NULL;
							}
							potential_seen++;
						}
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node;
					if (distribution(generator) <= 1.0 / (1.0 + sqrt(obs_node->num_experiments))) {
						uniform_int_distribution<int> select_distribution(0, potential_seen);
						if (select_distribution(generator) == 0) {
							node_context = node;
							is_branch = false;

							signal_scope = NULL;
						}
						potential_seen++;
					}
				}
				break;
			}
		}
	}
}

void create_experiment(SolutionWrapper* wrapper) {
	AbstractNode* node_context = NULL;
	bool is_branch;
	Scope* signal_scope = NULL;
	int potential_seen = 0;
	gather_helper(wrapper->scope_histories[0],
				  node_context,
				  is_branch,
				  signal_scope,
				  potential_seen);

	if (node_context != NULL) {
		double constant;
		vector<Input> inputs;
		vector<double> input_averages;
		vector<double> input_standard_deviations;
		vector<double> weights;
		vector<Input> network_inputs;
		Network* network = NULL;
		bool is_success = train_existing_helper(node_context,
												is_branch,
												wrapper,
												constant,
												inputs,
												input_averages,
												input_standard_deviations,
												weights,
												network_inputs,
												network);
		if (is_success) {
			Scope* scope_context = node_context->parent;

			vector<AbstractNode*> possible_exits;

			AbstractNode* starting_node;
			switch (node_context->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)node_context;
					starting_node = start_node->next_node;
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)node_context;
					starting_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)node_context;
					starting_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)node_context;
					if (is_branch) {
						starting_node = branch_node->branch_next_node;
					} else {
						starting_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node_context;
					starting_node = obs_node->next_node;
				}
				break;
			}

			scope_context->random_exit_activate(
				starting_node,
				possible_exits);

			int random_index;
			geometric_distribution<int> exit_distribution(0.1);
			while (true) {
				random_index = exit_distribution(generator);
				if (random_index < (int)possible_exits.size()) {
					break;
				}
			}
			AbstractNode* exit_next_node = possible_exits[random_index];

			ExploreExperiment* new_experiment = new ExploreExperiment(
				node_context,
				is_branch,
				exit_next_node,
				constant,
				inputs,
				input_averages,
				input_standard_deviations,
				weights,
				network_inputs,
				network);
			node_context->experiment = new_experiment;

			switch (node_context->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)node_context;
					start_node->num_experiments++;
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)node_context;
					action_node->num_experiments++;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)node_context;
					scope_node->num_experiments++;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)node_context;
					if (is_branch) {
						branch_node->branch_num_experiments++;
					} else {
						branch_node->original_num_experiments++;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node_context;
					obs_node->num_experiments++;
				}
				break;
			}
		}
	} else if (signal_scope != NULL) {
		signal_scope->signal_experiment = new SignalExperiment(signal_scope);
	}
}
