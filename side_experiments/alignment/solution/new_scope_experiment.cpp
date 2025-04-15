#include "new_scope_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "input.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

const int NEW_SCOPE_MIN_NUM_NODES = 4;
const int CREATE_NEW_SCOPE_NUM_TRIES = 50;

NewScopeExperiment::NewScopeExperiment(Scope* scope_context,
									   AbstractNode* node_context,
									   bool is_branch,
									   AbstractNode* exit_next_node) {
	this->type = EXPERIMENT_TYPE_NEW_SCOPE;

	this->new_scope = NULL;
	for (int t_index = 0; t_index < CREATE_NEW_SCOPE_NUM_TRIES; t_index++) {
		vector<AbstractNode*> possible_starting_nodes;
		for (map<int, AbstractNode*>::iterator it = scope_context->nodes.begin();
				it != scope_context->nodes.end(); it++) {
			if (it->second->experiment == NULL
					&& it->second->average_instances_per_run > 0.5) {
				possible_starting_nodes.push_back(it->second);
			}
		}

		if (possible_starting_nodes.size() > 0) {
			uniform_int_distribution<int> start_distribution(0, possible_starting_nodes.size()-1);
			AbstractNode* potential_starting_node = possible_starting_nodes[start_distribution(generator)];

			geometric_distribution<int> run_distribution(0.2);
			int num_runs = 1 + run_distribution(generator);

			set<AbstractNode*> potential_included_nodes;

			/**
			 * TODO: scale by possible length
			 */
			geometric_distribution<int> following_distribution(0.1);
			for (int r_index = 0; r_index < num_runs; r_index++) {
				int num_following = 1 + following_distribution(generator);
				scope_context->random_continue(
					potential_starting_node,
					num_following,
					potential_included_nodes);
			}

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
				this->new_scope = new Scope();
				this->new_scope->id = -1;

				this->new_scope->node_counter = 0;

				ObsNode* starting_node = new ObsNode();
				starting_node->parent = this->new_scope;
				starting_node->id = this->new_scope->node_counter;
				this->new_scope->node_counter++;
				this->new_scope->nodes[starting_node->id] = starting_node;

				map<AbstractNode*, AbstractNode*> node_mappings;
				for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
						node_it != potential_included_nodes.end(); node_it++) {
					switch ((*node_it)->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNode* original_action_node = (ActionNode*)(*node_it);

							ActionNode* new_action_node = new ActionNode();
							new_action_node->parent = this->new_scope;
							new_action_node->id = this->new_scope->node_counter;
							this->new_scope->node_counter++;
							this->new_scope->nodes[new_action_node->id] = new_action_node;

							new_action_node->action = original_action_node->action;

							node_mappings[original_action_node] = new_action_node;
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* original_scope_node = (ScopeNode*)(*node_it);

							ScopeNode* new_scope_node = new ScopeNode();
							new_scope_node->parent = this->new_scope;
							new_scope_node->id = this->new_scope->node_counter;
							this->new_scope->node_counter++;
							this->new_scope->nodes[new_scope_node->id] = new_scope_node;

							new_scope_node->scope = original_scope_node->scope;

							node_mappings[original_scope_node] = new_scope_node;
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* original_branch_node = (BranchNode*)(*node_it);

							BranchNode* new_branch_node = new BranchNode();
							new_branch_node->parent = this->new_scope;
							new_branch_node->id = this->new_scope->node_counter;
							this->new_scope->node_counter++;
							this->new_scope->nodes[new_branch_node->id] = new_branch_node;

							node_mappings[original_branch_node] = new_branch_node;
						}
						break;
					case NODE_TYPE_OBS:
						{
							ObsNode* original_obs_node = (ObsNode*)(*node_it);

							ObsNode* new_obs_node = new ObsNode();
							new_obs_node->parent = this->new_scope;
							new_obs_node->id = this->new_scope->node_counter;
							this->new_scope->node_counter++;
							this->new_scope->nodes[new_obs_node->id] = new_obs_node;

							node_mappings[original_obs_node] = new_obs_node;
						}
						break;
					}
				}

				starting_node->next_node_id = node_mappings[potential_starting_node]->id;
				starting_node->next_node = node_mappings[potential_starting_node];

				starting_node->next_node->ancestor_ids.push_back(starting_node->id);

				ObsNode* new_ending_node = new ObsNode();
				new_ending_node->parent = this->new_scope;
				new_ending_node->id = this->new_scope->node_counter;
				this->new_scope->node_counter++;
				this->new_scope->nodes[new_ending_node->id] = new_ending_node;
				new_ending_node->next_node_id = -1;
				new_ending_node->next_node = NULL;

				for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
						node_it != potential_included_nodes.end(); node_it++) {
					switch ((*node_it)->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNode* original_action_node = (ActionNode*)(*node_it);
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
							ScopeNode* original_scope_node = (ScopeNode*)(*node_it);
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
						{
							BranchNode* original_branch_node = (BranchNode*)(*node_it);
							BranchNode* new_branch_node = (BranchNode*)node_mappings[original_branch_node];

							new_branch_node->average_val = original_branch_node->average_val;

							for (int f_index = 0; f_index < (int)original_branch_node->factor_ids.size(); f_index++) {
								AbstractNode* original_input_node = scope_context->nodes[
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
							ObsNode* original_obs_node = (ObsNode*)(*node_it);
							ObsNode* new_obs_node = (ObsNode*)node_mappings[original_obs_node];

							for (int f_index = 0; f_index < (int)original_obs_node->factors.size(); f_index++) {
								Factor* original_factor = original_obs_node->factors[f_index];
								Factor* new_factor = new Factor();

								new_factor->network = new Network(original_factor->network);
								for (int i_index = (int)original_factor->inputs.size()-1; i_index >= 0; i_index--) {
									AbstractNode* original_input_node = scope_context->nodes[
										original_factor->inputs[i_index].node_context[0]];
									map<AbstractNode*, AbstractNode*>::iterator it = node_mappings.find(original_input_node);
									if (it == node_mappings.end()) {
										new_factor->network->remove_input(i_index);
									} else {
										Input new_input = original_factor->inputs[i_index];
										new_input.scope_context[0] = this->new_scope;
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

				break;
			}
		}
	}

	if (this->new_scope != NULL) {
		this->scope_context = scope_context;
		this->node_context = node_context;
		this->is_branch = is_branch;
		this->exit_next_node = exit_next_node;

		this->state = NEW_SCOPE_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;

		this->sum_score = 0.0;

		this->result = EXPERIMENT_RESULT_NA;
	} else {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}

NewScopeExperiment::~NewScopeExperiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

void NewScopeExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

NewScopeExperimentHistory::NewScopeExperimentHistory(
		NewScopeExperiment* experiment) {
	this->experiment = experiment;
}
