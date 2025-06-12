#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "network.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

const int NEW_SCOPE_MIN_NODES = 20;
const int NEW_SCOPE_MIN_NUM_NODES = 3;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 10;
const int NEW_SCOPE_NUM_LOCATIONS = 2;
#else
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 200;
const int NEW_SCOPE_NUM_LOCATIONS = 2;
#endif /* MDEBUG */

void ancestor_helper(AbstractNode* curr_node,
					 set<AbstractNode*>& ancestors) {
	for (int a_index = 0; a_index < (int)curr_node->ancestor_ids.size(); a_index++) {
		AbstractNode* next_node = curr_node->parent->nodes[curr_node->ancestor_ids[a_index]];
		set<AbstractNode*>::iterator it = ancestors.find(next_node);
		if (it == ancestors.end()) {
			ancestors.insert(next_node);

			ancestor_helper(next_node,
							ancestors);
		}
	}
}

void children_helper(AbstractNode* curr_node,
					 set<AbstractNode*>& children) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)curr_node;
			set<AbstractNode*>::iterator it = children.find(action_node->next_node);
			if (it == children.end()) {
				children.insert(action_node->next_node);

				children_helper(action_node->next_node,
								children);
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)curr_node;
			set<AbstractNode*>::iterator it = children.find(scope_node->next_node);
			if (it == children.end()) {
				children.insert(scope_node->next_node);

				children_helper(scope_node->next_node,
								children);
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)curr_node;
			set<AbstractNode*>::iterator original_it = children.find(branch_node->original_next_node);
			if (original_it == children.end()) {
				children.insert(branch_node->original_next_node);

				children_helper(branch_node->original_next_node,
								children);
			}
			set<AbstractNode*>::iterator branch_it = children.find(branch_node->branch_next_node);
			if (branch_it == children.end()) {
				children.insert(branch_node->branch_next_node);

				children_helper(branch_node->branch_next_node,
								children);
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)curr_node;
			if (obs_node->next_node != NULL) {
				set<AbstractNode*>::iterator it = children.find(obs_node->next_node);
				if (it == children.end()) {
					children.insert(obs_node->next_node);

					children_helper(obs_node->next_node,
									children);
				}
			}
		}
		break;
	}
}

void Scope::check_new_scope(set<Scope*>& updated_scopes) {
	if (this->new_scope != NULL) {
		if (this->successful_experiments.size() == 0
				&& this->generalize_iter > 0) {
			for (int e_index = 0; e_index < (int)this->test_experiments.size(); e_index++) {
				delete this->test_experiments[e_index];
			}
			this->test_experiments.clear();

			delete this->new_scope;
			this->new_scope = NULL;
		} else if (this->successful_experiments.size() >= NEW_SCOPE_NUM_LOCATIONS) {
			this->new_scope->id = solution->scopes.size();
			solution->scopes.push_back(this->new_scope);

			clean_scope(this->new_scope);

			this->child_scopes.push_back(this->new_scope);

			if (this->new_scope->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
				this->new_scope->exceeded = true;

				check_generalize(this->new_scope);
			}

			for (int e_index = 0; e_index < (int)this->test_experiments.size(); e_index++) {
				delete this->test_experiments[e_index];
			}
			this->test_experiments.clear();
			for (int e_index = 0; e_index < (int)this->successful_experiments.size(); e_index++) {
				this->successful_experiments[e_index]->add();
				delete this->successful_experiments[e_index];
			}
			this->successful_experiments.clear();

			updated_scopes.insert(this);

			this->new_scope = NULL;
		}
	}

	if (this->nodes.size() >= NEW_SCOPE_MIN_NODES
			&& this->new_scope == NULL) {
		uniform_int_distribution<int> node_distribution(0, this->nodes.size()-1);
		AbstractNode* potential_start_node = next(this->nodes.begin(), node_distribution(generator))->second;
		AbstractNode* potential_end_node = next(this->nodes.begin(), node_distribution(generator))->second;

		set<AbstractNode*> children;
		children_helper(potential_start_node,
						children);

		set<AbstractNode*> ancestors;
		ancestor_helper(potential_end_node,
						ancestors);

		set<AbstractNode*> potential_included_nodes;
		for (set<AbstractNode*>::iterator it = children.begin(); it != children.end(); it++) {
			if (ancestors.find(*it) != ancestors.end()) {
				potential_included_nodes.insert(*it);
			}
		}
		potential_included_nodes.insert(potential_start_node);
		potential_included_nodes.insert(potential_end_node);

		int num_meaningful_nodes = 0;
		for (set<AbstractNode*>::iterator it = potential_included_nodes.begin();
				it != potential_included_nodes.end(); it++) {
			switch ((*it)->type) {
			case NODE_TYPE_ACTION:
			case NODE_TYPE_SCOPE:
				num_meaningful_nodes++;
				break;
			}
		}
		if (num_meaningful_nodes >= NEW_SCOPE_MIN_NUM_NODES) {
			Scope* new_scope = new Scope();
			new_scope->id = -1;

			new_scope->node_counter = 0;

			ObsNode* starting_node = new ObsNode();
			starting_node->parent = new_scope;
			starting_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[starting_node->id] = starting_node;

			map<AbstractNode*, AbstractNode*> node_mappings;
			for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
					node_it != potential_included_nodes.end(); node_it++) {
				switch ((*node_it)->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* original_action_node = (ActionNode*)(*node_it);

						ActionNode* new_action_node = new ActionNode();
						new_action_node->parent = new_scope;
						new_action_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_action_node->id] = new_action_node;

						new_action_node->action = original_action_node->action;

						node_mappings[original_action_node] = new_action_node;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* original_scope_node = (ScopeNode*)(*node_it);

						ScopeNode* new_scope_node = new ScopeNode();
						new_scope_node->parent = new_scope;
						new_scope_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_scope_node->id] = new_scope_node;

						new_scope_node->scope = original_scope_node->scope;

						node_mappings[original_scope_node] = new_scope_node;
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* original_obs_node = (ObsNode*)(*node_it);

						ObsNode* new_obs_node = new ObsNode();
						new_obs_node->parent = new_scope;
						new_obs_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_obs_node->id] = new_obs_node;

						node_mappings[original_obs_node] = new_obs_node;
					}
					break;
				}
			}
			for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
					node_it != potential_included_nodes.end(); node_it++) {
				if ((*node_it)->type == NODE_TYPE_BRANCH) {
					BranchNode* original_branch_node = (BranchNode*)(*node_it);

					map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings.find(original_branch_node->original_next_node);
					map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings.find(original_branch_node->branch_next_node);
					if (original_it != node_mappings.end()
							&& branch_it != node_mappings.end()) {
						BranchNode* new_branch_node = new BranchNode();
						new_branch_node->parent = new_scope;
						new_branch_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_branch_node->id] = new_branch_node;

						node_mappings[original_branch_node] = new_branch_node;
					} else if (original_it != node_mappings.end()) {
						node_mappings[original_branch_node] = original_it->second;
					} else if (branch_it != node_mappings.end()) {
						node_mappings[original_branch_node] = branch_it->second;
					}
				}
			}

			starting_node->next_node_id = node_mappings[potential_start_node]->id;
			starting_node->next_node = node_mappings[potential_start_node];

			starting_node->next_node->ancestor_ids.push_back(starting_node->id);

			ObsNode* new_ending_node = new ObsNode();
			new_ending_node->parent = new_scope;
			new_ending_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[new_ending_node->id] = new_ending_node;
			new_ending_node->next_node_id = -1;
			new_ending_node->next_node = NULL;

			for (map<AbstractNode*, AbstractNode*>::iterator node_it = node_mappings.begin();
					node_it != node_mappings.end(); node_it++) {
				switch (node_it->first->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* original_action_node = (ActionNode*)node_it->first;
						ActionNode* new_action_node = (ActionNode*)node_mappings[original_action_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_action_node->next_node);
						if (it == node_mappings.end()) {
							new_action_node->next_node_id = new_ending_node->id;
							new_action_node->next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_action_node->id);
						} else {
							new_action_node->next_node_id = it->second->id;
							new_action_node->next_node = it->second;

							it->second->ancestor_ids.push_back(new_action_node->id);
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* original_scope_node = (ScopeNode*)node_it->first;
						ScopeNode* new_scope_node = (ScopeNode*)node_mappings[original_scope_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_scope_node->next_node);
						if (it == node_mappings.end()) {
							new_scope_node->next_node_id = new_ending_node->id;
							new_scope_node->next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_scope_node->id);
						} else {
							new_scope_node->next_node_id = it->second->id;
							new_scope_node->next_node = it->second;

							it->second->ancestor_ids.push_back(new_scope_node->id);
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					if (node_it->first->type == node_it->second->type) {
						BranchNode* original_branch_node = (BranchNode*)node_it->first;
						BranchNode* new_branch_node = (BranchNode*)node_mappings[original_branch_node];

						new_branch_node->average_val = original_branch_node->average_val;

						for (int f_index = 0; f_index < (int)original_branch_node->factor_ids.size(); f_index++) {
							AbstractNode* original_input_node = this->nodes[
								original_branch_node->factor_ids[f_index].first];
							map<AbstractNode*, AbstractNode*>::iterator input_it = node_mappings
								.find(original_input_node);
							if (input_it != node_mappings.end()) {
								new_branch_node->factor_ids.push_back(
									{input_it->second->id, original_branch_node->factor_ids[f_index].second});
								new_branch_node->factor_weights.push_back(
									original_branch_node->factor_weights[f_index]);
							}
						}

						map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
							.find(original_branch_node->original_next_node);
						if (original_it == node_mappings.end()) {
							new_branch_node->original_next_node_id = new_ending_node->id;
							new_branch_node->original_next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_branch_node->id);
						} else {
							new_branch_node->original_next_node_id = original_it->second->id;
							new_branch_node->original_next_node = original_it->second;

							original_it->second->ancestor_ids.push_back(new_branch_node->id);
						}
						map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
							.find(original_branch_node->branch_next_node);
						if (branch_it == node_mappings.end()) {
							new_branch_node->branch_next_node_id = new_ending_node->id;
							new_branch_node->branch_next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_branch_node->id);
						} else {
							new_branch_node->branch_next_node_id = branch_it->second->id;
							new_branch_node->branch_next_node = branch_it->second;

							branch_it->second->ancestor_ids.push_back(new_branch_node->id);
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* original_obs_node = (ObsNode*)node_it->first;
						ObsNode* new_obs_node = (ObsNode*)node_mappings[original_obs_node];

						for (int f_index = 0; f_index < (int)original_obs_node->factors.size(); f_index++) {
							Factor* original_factor = original_obs_node->factors[f_index];
							Factor* new_factor = new Factor();

							new_factor->network = new Network(original_factor->network);
							for (int i_index = (int)original_factor->inputs.size()-1; i_index >= 0; i_index--) {
								AbstractNode* original_input_node = this->nodes[
									original_factor->inputs[i_index].node_context[0]];
								map<AbstractNode*, AbstractNode*>::iterator it = node_mappings.find(original_input_node);
								if (it == node_mappings.end()) {
									new_factor->network->remove_input(i_index);
								} else {
									Input new_input = original_factor->inputs[i_index];
									new_input.scope_context[0] = new_scope;
									new_input.node_context[0] = it->second->id;
									new_factor->inputs.insert(new_factor->inputs.begin(), new_input);
								}
							}

							new_obs_node->factors.push_back(new_factor);
						}

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_obs_node->next_node);
						if (it == node_mappings.end()) {
							new_obs_node->next_node_id = new_ending_node->id;
							new_obs_node->next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_obs_node->id);
						} else {
							new_obs_node->next_node_id = it->second->id;
							new_obs_node->next_node = it->second;

							it->second->ancestor_ids.push_back(new_obs_node->id);
						}
					}
					break;
				}
			}

			new_scope->child_scopes = this->child_scopes;

			this->new_scope = new_scope;
			this->generalize_iter = 0;
		}
	}
}
