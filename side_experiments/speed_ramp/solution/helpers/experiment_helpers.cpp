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

const int SIGNAL_EXPERIMENT_MIN_GENERALIZE_SUCCESSES = 3;

void gather_helper(ScopeHistory* scope_history,
				   set<pair<AbstractNode*, bool>>& possible_starts,
				   set<Scope*>& possible_signals) {
	Scope* scope = scope_history->scope;
	if (scope->num_generalize_successes >= SIGNAL_EXPERIMENT_MIN_GENERALIZE_SUCCESSES
			|| scope->id == 0) {
		if (scope->signal_experiment == NULL) {
			possible_signals.insert(scope);
		}
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		if (node->experiment == NULL) {
			switch (node->type) {
			case NODE_TYPE_START:
			case NODE_TYPE_ACTION:
			case NODE_TYPE_OBS:
				possible_starts.insert({node, false});
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					gather_helper(scope_node_history->scope_history,
								  possible_starts,
								  possible_signals);
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					possible_starts.insert({node, branch_node_history->is_branch});
				}
				break;
			}
		}
	}
}

void create_experiment(SolutionWrapper* wrapper) {
	set<pair<AbstractNode*, bool>> possible_starts;
	set<Scope*> possible_signals;
	gather_helper(wrapper->scope_histories[0],
				  possible_starts,
				  possible_signals);

	if (possible_starts.size() > 0 || possible_signals.size() > 0) {
		/**
		 * - prioritize exploring new nodes
		 * 
		 * - even if changes made later, unlikely to influence old
		 *   - old still needs to also match everything else
		 */
		
		while (true) {
			uniform_real_distribution<double> distribution(0.0, 1.0);

			vector<pair<AbstractNode*, bool>> selected_nodes;
			for (set<pair<AbstractNode*, bool>>::iterator it = possible_starts.begin();
					it != possible_starts.end(); it++) {
				switch ((*it).first->type) {
				case NODE_TYPE_START:
					{
						StartNode* start_node = (StartNode*)(*it).first;
						if (distribution(generator) <= 1.0 / (1.0 + sqrt(start_node->num_experiments))) {
							selected_nodes.push_back(*it);
						}
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)(*it).first;
						if (distribution(generator) <= 1.0 / (1.0 + sqrt(action_node->num_experiments))) {
							selected_nodes.push_back(*it);
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)(*it).first;
						if (distribution(generator) <= 1.0 / (1.0 + sqrt(scope_node->num_experiments))) {
							selected_nodes.push_back(*it);
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)(*it).first;
						if ((*it).second) {
							if (distribution(generator) <= 1.0 / (1.0 + sqrt(branch_node->branch_num_experiments))) {
								selected_nodes.push_back(*it);
							}
						} else {
							if (distribution(generator) <= 1.0 / (1.0 + sqrt(branch_node->original_num_experiments))) {
								selected_nodes.push_back(*it);
							}
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)(*it).first;
						if (distribution(generator) <= 1.0 / (1.0 + sqrt(obs_node->num_experiments))) {
							selected_nodes.push_back(*it);
						}
					}
					break;
				}
			}
			vector<Scope*> selected_signals;
			for (set<Scope*>::iterator it = possible_signals.begin();
					it != possible_signals.end(); it++) {
				if (distribution(generator) <= 1.0 / (1.0 + sqrt((*it)->num_experiments))) {
					selected_signals.push_back(*it);
				}
			}

			if (selected_nodes.size() > 0 || selected_signals.size() > 0) {
				uniform_int_distribution<int> select_distribution(0, selected_nodes.size() + selected_signals.size() -1);
				int select_index = select_distribution(generator);
				if (select_index < (int)selected_nodes.size()) {
					AbstractNode* node_context = selected_nodes[select_index].first;
					bool is_branch = selected_nodes[select_index].second;

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
						scope_context,
						node_context,
						is_branch,
						exit_next_node);
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
				} else {
					Scope* scope = selected_signals[select_index - selected_nodes.size()];
					scope->signal_experiment = new SignalExperiment(scope);
					scope->num_experiments++;
				}

				break;
			}
		}

		
	}
}
