#include "signal_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "signal_instance.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void SignalExperiment::set_actions(SolutionWrapper* wrapper) {
	uniform_int_distribution<int> action_distribution(0, 2);
	/**
	 * TODO: add easy way to fetch action
	 */
	map<int, Signal*>::iterator it = wrapper->signals.find(this->scope_context_id);
	if (it == wrapper->signals.end()) {
		geometric_distribution<int> num_actions_distribution(0.2);
		int num_pre = num_actions_distribution(generator);
		for (int a_index = 0; a_index < num_pre; a_index++) {
			this->pre_actions.push_back(action_distribution(generator));
		}
		int num_post = 5 + num_actions_distribution(generator);
		for (int a_index = 0; a_index < num_post; a_index++) {
			this->post_actions.push_back(action_distribution(generator));
		}
	} else {
		for (int s_index = 0; s_index < (int)it->second->instances.size(); s_index++) {
			this->signals.push_back(new SignalInstance(it->second->instances[s_index]));
		}

		/**
		 * - simply try one change pre, one change post
		 */
		geometric_distribution<int> exit_distribution(0.2);
		geometric_distribution<int> length_distribution(0.3);
		/**
		 * - length smaller than exit to try to reduce actions
		 */

		uniform_int_distribution<int> pre_index_distribution(0, it->second->signal_pre_actions.size());
		int pre_index = pre_index_distribution(generator);
		int pre_exit_index;
		while (true) {
			pre_exit_index = pre_index + exit_distribution(generator);
			if (pre_exit_index <= (int)it->second->signal_pre_actions.size()) {
				break;
			}
		}
		this->pre_actions = it->second->signal_pre_actions;
		this->pre_actions.erase(this->pre_actions.begin() + pre_index,
			this->pre_actions.begin() + pre_exit_index);
		int pre_insert_length = length_distribution(generator);
		for (int a_index = 0; a_index < pre_insert_length; a_index++) {
			this->pre_actions.insert(this->pre_actions.begin() + pre_index, action_distribution(generator));
		}
		for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
			this->signals[s_index]->insert(true,
										   pre_index,
										   pre_exit_index,
										   pre_insert_length);
		}

		uniform_int_distribution<int> post_index_distribution(0, it->second->signal_post_actions.size());
		int post_index = post_index_distribution(generator);
		int post_exit_index;
		while (true) {
			post_exit_index = post_index + exit_distribution(generator);
			if (post_exit_index <= (int)it->second->signal_post_actions.size()) {
				break;
			}
		}
		this->post_actions = it->second->signal_post_actions;
		this->post_actions.erase(this->post_actions.begin() + post_index,
			this->post_actions.begin() + post_exit_index);
		int post_insert_length = length_distribution(generator);
		for (int a_index = 0; a_index < post_insert_length; a_index++) {
			this->post_actions.insert(this->post_actions.begin() + post_index, action_distribution(generator));
		}
		for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
			this->signals[s_index]->insert(false,
										   post_index,
										   post_exit_index,
										   post_insert_length);
		}
	}

	this->new_scores = vector<vector<double>>(wrapper->solutions.size());
	this->existing_pre_obs = vector<vector<vector<vector<double>>>>(wrapper->solutions.size());
	this->existing_post_obs = vector<vector<vector<vector<double>>>>(wrapper->solutions.size());
	this->existing_scores = vector<vector<double>>(wrapper->solutions.size());
}

void SignalExperiment::set_explore(SolutionWrapper* wrapper) {
	Scope* scope = wrapper->solutions[this->solution_index]->scopes[this->scope_context_id];

	vector<pair<AbstractNode*,bool>> possible_explore_nodes;
	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_START:
			possible_explore_nodes.push_back({it->second, false});
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;
				if (action_node->average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({action_node, false});
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)it->second;
				if (scope_node->average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({scope_node, false});
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				if (branch_node->original_average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({branch_node, false});
				}
				if (branch_node->branch_average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({branch_node, true});
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({obs_node, false});
				}
			}
			break;
		}
	}

	uniform_int_distribution<int> explore_distribution(0, possible_explore_nodes.size()-1);
	int explore_random_index = explore_distribution(generator);
	this->explore_node = possible_explore_nodes[explore_random_index].first;
	this->explore_is_branch = possible_explore_nodes[explore_random_index].second;

	vector<AbstractNode*> possible_exits;

	AbstractNode* starting_node;
	switch (this->explore_node->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->explore_node;
			starting_node = start_node->next_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->explore_node;
			starting_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->explore_node;
			starting_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->explore_node;
			if (this->explore_is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->explore_node;
			starting_node = obs_node->next_node;
		}
		break;
	}

	scope->random_exit_activate(
		starting_node,
		possible_exits);

	int exit_random_index;
	geometric_distribution<int> exit_distribution(0.2);
	while (true) {
		exit_random_index = exit_distribution(generator);
		if (exit_random_index < (int)possible_exits.size()) {
			break;
		}
	}
	this->exit_next_node = possible_exits[exit_random_index];

	#if defined(MDEBUG) && MDEBUG
	uniform_int_distribution<int> new_scope_distribution(0, 1);
	#else
	uniform_int_distribution<int> new_scope_distribution(0, 4);
	#endif /* MDEBUG */
	if (new_scope_distribution(generator) == 0) {
		this->new_scope = create_new_scope(scope);
	}
	if (this->new_scope != NULL) {
		this->step_types.push_back(STEP_TYPE_SCOPE);
		this->actions.push_back(-1);
		this->scopes.push_back(this->new_scope);
	} else {
		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.2);
		if (exit_random_index == 0) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}

		/**
		 * - always give raw actions a large weight
		 *   - existing scopes often learned to avoid certain patterns
		 *     - which can prevent innovation
		 */
		uniform_int_distribution<int> scope_distribution(0, 1);
		vector<Scope*> possible_scopes;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index]->nodes.size() > 1) {
				possible_scopes.push_back(scope->child_scopes[c_index]);
			}
		}
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (scope_distribution(generator) == 0 && possible_scopes.size() > 0) {
				this->step_types.push_back(STEP_TYPE_SCOPE);
				this->actions.push_back(-1);

				uniform_int_distribution<int> child_scope_distribution(0, possible_scopes.size()-1);
				this->scopes.push_back(possible_scopes[child_scope_distribution(generator)]);
			} else {
				this->step_types.push_back(STEP_TYPE_ACTION);

				this->actions.push_back(-1);

				this->scopes.push_back(NULL);
			}
		}
	}

	this->explore_node->experiment = this;

	switch (this->explore_node->type) {
	case NODE_TYPE_START:
		this->average_instances_per_run = scope->average_instances_per_run;
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->explore_node;
			this->average_instances_per_run = action_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->explore_node;
			this->average_instances_per_run = scope_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->explore_node;
			if (this->explore_is_branch) {
				this->average_instances_per_run = branch_node->branch_average_instances_per_run;
			} else {
				this->average_instances_per_run = branch_node->original_average_instances_per_run;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->explore_node;
			this->average_instances_per_run = obs_node->average_instances_per_run;
		}
		break;
	}
}
