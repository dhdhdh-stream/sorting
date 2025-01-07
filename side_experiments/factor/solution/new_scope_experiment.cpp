#include "new_scope_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

const int NEW_SCOPE_MIN_NUM_NODES = 3;
const int CREATE_NEW_SCOPE_NUM_TRIES = 50;

NewScopeExperiment::NewScopeExperiment(Scope* scope_context,
									   AbstractNode* node_context,
									   bool is_branch) {
	this->type = EXPERIMENT_TYPE_NEW_SCOPE;

	this->new_scope = NULL;
	for (int t_index = 0; t_index < CREATE_NEW_SCOPE_NUM_TRIES; t_index++) {
		vector<AbstractNode*> possible_starting_nodes;
		scope_context->random_exit_activate(
			scope_context->nodes[0],
			possible_starting_nodes);

		uniform_int_distribution<int> start_distribution(0, possible_starting_nodes.size()-1);
		AbstractNode* potential_starting_node = possible_starting_nodes[start_distribution(generator)];

		geometric_distribution<int> run_distribution(0.33);
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

							new_ending_node->ancestors.push_back(new_action_node);
						} else {
							new_action_node->next_node_id = it->second->id;
							new_action_node->next_node = it->second;

							it->second->ancestors.push_back(new_action_node);
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

							new_ending_node->ancestors.push_back(new_scope_node);
						} else {
							new_scope_node->next_node_id = it->second->id;
							new_scope_node->next_node = it->second;

							it->second->ancestors.push_back(new_scope_node);
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* original_branch_node = (BranchNode*)(*node_it);
						BranchNode* new_branch_node = (BranchNode*)node_mappings[original_branch_node];

						for (int f_index = 0; f_index < (int)original_branch_node->factor_ids.size(); f_index++) {
							AbstractNode* original_input_node = scope_context->nodes[
								original_branch_node->factor_ids[f_index].first];
							map<AbstractNode*, AbstractNode*>::iterator input_it = node_mappings
								.find(original_input_node);
							if (input_it != node_mappings.end()) {
								new_branch_node->factor_ids.push_back(
									{input_it->id, original_branch_node->factor_ids[f_index].second});
								new_branch_node->factor_weights.push_back(
									original_branch_node->factor_weights[f_index]);
							}
						}

						map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
							.find(original_branch_node->original_next_node);
						if (original_it == node_mappings.end()) {
							new_branch_node->original_next_node_id = new_ending_node->id;
							new_branch_node->original_next_node = new_ending_node;

							new_ending_node->ancestors.push_back(new_branch_node);
						} else {
							new_branch_node->original_next_node_id = original_it->second->id;
							new_branch_node->original_next_node = original_it->second;

							original_it->second->ancestors.push_back(new_branch_node);
						}
						map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
							.find(original_branch_node->branch_next_node);
						if (branch_it == node_mappings.end()) {
							new_branch_node->branch_next_node_id = new_ending_node->id;
							new_branch_node->branch_next_node = new_ending_node;

							new_ending_node->ancestors.push_back(new_branch_node);
						} else {
							new_branch_node->branch_next_node_id = branch_it->second->id;
							new_branch_node->branch_next_node = branch_it->second;

							branch_it->second->ancestors.push_back(new_branch_node);
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
							new_factor->parent = new_obs_node;
							new_factor->index = f_index;

							new_factor->network = new Network(original_factor->network);
							for (int i_index = (int)original_factor->inputs.size()-1; i_index >= 0; i_index--) {
								AbstractNode* original_input_node = scope_context->nodes[
									original_factor->inputs[i_index].first.second[0]];
								map<AbstractNode*, AbstractNode*>::iterator it = node_mappings.find(original_input_node);
								if (it == node_mappings.end()) {
									new_factor->network->remove_input(i_index);
								} else {
									pair<pair<vector<Scope*>,vector<int>>,pair<int,int>> new_input = original_factor->inputs[i_index];
									new_input.first.first[0] = this->new_scope;
									new_input.first.second[0] = it->second->id;
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

							new_ending_node->ancestors.push_back(new_obs_node);
						} else {
							new_obs_node->next_node_id = it->second->id;
							new_obs_node->next_node = it->second;

							it->second->ancestors.push_back(new_obs_node);
						}
					}
					break;
				}
			}

			break;
		}
	}

	if (this->new_scope != NULL) {
		this->scope_context = scope_context;

		vector<AbstractNode*> possible_exits;

		AbstractNode* random_start_node;
		switch (node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node_context;
				random_start_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node_context;
				random_start_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node_context;
				if (is_branch) {
					random_start_node = branch_node->branch_next_node;
				} else {
					random_start_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node_context;
				random_start_node = obs_node->next_node;
			}
			break;
		}

		this->scope_context->random_exit_activate(
			random_start_node,
			possible_exits);

		uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
		AbstractNode* exit_next_node = possible_exits[exit_distribution(generator)];

		this->test_location_starts.push_back(node_context);
		this->test_location_is_branch.push_back(is_branch);
		this->test_location_exits.push_back(exit_next_node);
		this->test_location_states.push_back(LOCATION_STATE_MEASURE_EXISTING);
		this->test_location_existing_scores.push_back(0.0);
		this->test_location_new_scores.push_back(0.0);
		this->test_location_counts.push_back(0);

		this->average_remaining_experiments_from_start = 1.0;

		/**
		 * - added to node_context.experiments outside
		 */

		this->state = NEW_SCOPE_EXPERIMENT_STATE_EXPLORE;
		this->generalize_iter = -1;

		this->result = EXPERIMENT_RESULT_NA;
	} else {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}

NewScopeExperiment::~NewScopeExperiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}

	for (int s_index = 0; s_index < (int)this->successful_scope_nodes.size(); s_index++) {
		/**
		 * - prevent recursive delete
		 */
		this->successful_scope_nodes[s_index]->experiments.clear();

		delete this->successful_scope_nodes[s_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

void NewScopeExperiment::decrement(AbstractNode* experiment_node) {
	/**
	 * - simply try to remove experiments on new every decrement
	 */
	for (int t_index = (int)this->test_location_starts.size()-1; t_index >= 0; t_index--) {
		if (this->scope_context->nodes.find(this->test_location_starts[t_index]->id) == this->scope_context->nodes.end()) {
			this->test_location_starts.erase(this->test_location_starts.begin() + t_index);
		}
	}
	for (int s_index = (int)this->successful_location_starts.size()-1; s_index >= 0; s_index--) {
		if (this->scope_context->nodes.find(this->successful_location_starts[s_index]->id) == this->scope_context->nodes.end()) {
			this->successful_location_starts.erase(this->successful_location_starts.begin() + s_index);
		}
	}

	bool is_test;
	int location_index;
	for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
		if (this->test_location_starts[t_index] == experiment_node) {
			is_test = true;
			location_index = t_index;
			break;
		}
	}
	for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
		if (this->successful_location_starts[s_index] == experiment_node) {
			is_test = false;
			location_index = s_index;
			break;
		}
	}

	if (is_test) {
		this->test_location_starts.erase(this->test_location_starts.begin() + location_index);
	} else {
		this->successful_location_starts.erase(this->successful_location_starts.begin() + location_index);
	}

	if (this->test_location_starts.size() == 0
			&& this->successful_location_starts.size() == 0) {
		delete this;
	}
}

NewScopeExperimentHistory::NewScopeExperimentHistory(
		NewScopeExperiment* experiment) {
	this->experiment = experiment;

	this->test_location_index = -1;
}
