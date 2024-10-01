#include "markov_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "markov_node.h"
#include "network.h"
#include "problem.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

bool options_is_match(vector<int>& a_step_types,
					  vector<Action>& a_actions,
					  vector<Scope*>& a_scopes,
					  vector<int>& b_step_types,
					  vector<Action>& b_actions,
					  vector<Scope*>& b_scopes) {
	if (a_step_types.size() == b_step_types.size()) {
		for (int s_index = 0; s_index < (int)a_step_types.size(); s_index++) {
			if (a_step_types[s_index] != b_step_types[s_index]) {
				return false;
			} else {
				switch (a_step_types[s_index]) {
				case MARKOV_STEP_TYPE_ACTION:
					if (a_actions[s_index].move != b_actions[s_index].move) {
						return false;
					}
					break;
				case MARKOV_STEP_TYPE_SCOPE:
					if (a_scopes[s_index] != b_scopes[s_index]) {
						return false;
					}
					break;
				}
			}
		}

		return true;
	}

	return false;
}

MarkovExperiment::MarkovExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch) {
	this->type = EXPERIMENT_TYPE_MARKOV;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->average_remaining_experiments_from_start = 1.0;

	vector<AbstractNode*> possible_exits;

	if (this->node_context->type == NODE_TYPE_ACTION
			&& ((ActionNode*)this->node_context)->next_node == NULL) {
		possible_exits.push_back(NULL);
	}

	AbstractNode* starting_node;
	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			starting_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			starting_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_RETURN:
		{
			ReturnNode* return_node = (ReturnNode*)this->node_context;
			if (this->is_branch) {
				starting_node = return_node->passed_next_node;
			} else {
				starting_node = return_node->skipped_next_node;
			}
		}
		break;
	case NODE_TYPE_MARKOV:
		{
			MarkovNode* markov_node = (MarkovNode*)this->node_context;
			starting_node = markov_node->next_node;
		}
		break;
	}

	this->scope_context->random_exit_activate(
		starting_node,
		possible_exits);

	geometric_distribution<int> exit_distribution(0.3);
	int exit_index = exit_distribution(generator);
	if (exit_index > (int)possible_exits.size()-1) {
		exit_index = (int)possible_exits.size()-1;
	}
	this->exit_next_node = possible_exits[exit_index];

	uniform_int_distribution<int> individual_action_distribution(0, 19);
	uniform_int_distribution<int> on_path_distribution(0, 1);
	geometric_distribution<int> action_length_distribution(0.3);
	while (true) {
		if (individual_action_distribution(generator) == 0) {
			vector<int> new_step_types{MARKOV_STEP_TYPE_ACTION};
			vector<Action> new_actions{problem_type->random_action()};
			vector<Scope*> new_scopes{NULL};

			bool is_existing = false;
			for (int o_index = 0; o_index < (int)this->step_types.size(); o_index++) {
				if (options_is_match(this->step_types[o_index],
									 this->actions[o_index],
									 this->scopes[o_index],
									 new_step_types,
									 new_actions,
									 new_scopes)) {
					is_existing = true;
					break;
				}
			}
			if (!is_existing) {
				this->step_types.push_back(new_step_types);
				this->actions.push_back(new_actions);
				this->scopes.push_back(new_scopes);
				this->networks.push_back(new Network(MARKOV_NODE_ANALYZE_SIZE));
			}
		} else {
			if (this->exit_next_node != NULL
					&& on_path_distribution(generator) == 0) {
				int action_length = action_length_distribution(generator);
				if (action_length > exit_index+1) {
					action_length = exit_index+1;
				}

				vector<int> new_step_types;
				vector<Action> new_actions;
				vector<Scope*> new_scopes;
				uniform_int_distribution<int> option_start_distribution(0, exit_index+1 - action_length);
				int option_start_index = option_start_distribution(generator);
				for (int n_index = 0; n_index < action_length; n_index++) {
					switch (possible_exits[option_start_index + n_index]->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)possible_exits[option_start_index + n_index];

							if (action_node->action.move != ACTION_NOOP) {
								new_step_types.push_back(MARKOV_STEP_TYPE_ACTION);
								new_actions.push_back(action_node->action);
								new_scopes.push_back(NULL);
							}
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)possible_exits[option_start_index + n_index];

							new_step_types.push_back(MARKOV_STEP_TYPE_SCOPE);
							new_actions.push_back(Action());
							new_scopes.push_back(scope_node->scope);
						}
						break;
					case NODE_TYPE_MARKOV:
						{
							MarkovNode* markov_node = (MarkovNode*)possible_exits[option_start_index + n_index];

							uniform_int_distribution<int> option_distribution(-1, markov_node->step_types.size()-1);
							int option_index = option_distribution(generator);
							if (option_index != -1) {
								new_step_types.insert(new_step_types.end(),
									markov_node->step_types[option_index].begin(), markov_node->step_types[option_index].end());
								new_actions.insert(new_actions.end(),
									markov_node->actions[option_index].begin(), markov_node->actions[option_index].end());
								new_scopes.insert(new_scopes.end(),
									markov_node->scopes[option_index].begin(), markov_node->scopes[option_index].end());
							}
						}
						break;
					}
				}

				if (new_step_types.size() > 0) {
					bool is_existing = false;
					for (int o_index = 0; o_index < (int)this->step_types.size(); o_index++) {
						if (options_is_match(this->step_types[o_index],
											 this->actions[o_index],
											 this->scopes[o_index],
											 new_step_types,
											 new_actions,
											 new_scopes)) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						this->step_types.push_back(new_step_types);
						this->actions.push_back(new_actions);
						this->scopes.push_back(new_scopes);
						this->networks.push_back(new Network(MARKOV_NODE_ANALYZE_SIZE));
					}
				}
			} else {
				ActionNode* action_node = (ActionNode*)this->scope_context->nodes[0];
				AbstractNode* starting_node = action_node->next_node;

				vector<AbstractNode*> path;
				this->scope_context->random_exit_activate(
					starting_node,
					path);

				if (path.size() > 0) {
					int action_length = action_length_distribution(generator);
					if (action_length > (int)path.size()) {
						action_length = (int)path.size();
					}

					vector<int> new_step_types;
					vector<Action> new_actions;
					vector<Scope*> new_scopes;
					uniform_int_distribution<int> option_start_distribution(0, path.size() - action_length);
					int option_start_index = option_start_distribution(generator);
					for (int n_index = 0; n_index < action_length; n_index++) {
						switch (path[option_start_index + n_index]->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNode* action_node = (ActionNode*)path[option_start_index + n_index];

								if (action_node->action.move != ACTION_NOOP) {
									new_step_types.push_back(MARKOV_STEP_TYPE_ACTION);
									new_actions.push_back(action_node->action);
									new_scopes.push_back(NULL);
								}
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* scope_node = (ScopeNode*)path[option_start_index + n_index];

								new_step_types.push_back(MARKOV_STEP_TYPE_SCOPE);
								new_actions.push_back(Action());
								new_scopes.push_back(scope_node->scope);
							}
							break;
						case NODE_TYPE_MARKOV:
							{
								MarkovNode* markov_node = (MarkovNode*)path[option_start_index + n_index];

								uniform_int_distribution<int> option_distribution(-1, markov_node->step_types.size()-1);
								int option_index = option_distribution(generator);
								if (option_index != -1) {
									new_step_types.insert(new_step_types.end(),
										markov_node->step_types[option_index].begin(), markov_node->step_types[option_index].end());
									new_actions.insert(new_actions.end(),
										markov_node->actions[option_index].begin(), markov_node->actions[option_index].end());
									new_scopes.insert(new_scopes.end(),
										markov_node->scopes[option_index].begin(), markov_node->scopes[option_index].end());
								}
							}
							break;
						}
					}

					if (new_step_types.size() > 0) {
						bool is_existing = false;
						for (int o_index = 0; o_index < (int)this->step_types.size(); o_index++) {
							if (options_is_match(this->step_types[o_index],
												 this->actions[o_index],
												 this->scopes[o_index],
												 new_step_types,
												 new_actions,
												 new_scopes)) {
								is_existing = true;
								break;
							}
						}
						if (!is_existing) {
							this->step_types.push_back(new_step_types);
							this->actions.push_back(new_actions);
							this->scopes.push_back(new_scopes);
							this->networks.push_back(new Network(MARKOV_NODE_ANALYZE_SIZE));
						}
					}
				}
			}
		}

		if (this->actions.size() >= MARKOV_EXPERIMENT_NUM_OPTIONS) {
			break;
		}
	}

	this->halt_network = new Network(MARKOV_NODE_ANALYZE_SIZE);

	this->state = MARKOV_EXPERIMENT_STATE_TRAIN;
	this->state_iter = 0;
	this->sub_state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

MarkovExperiment::~MarkovExperiment() {
	for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
		delete this->networks[n_index];
	}

	if (this->halt_network != NULL) {
		delete this->halt_network;
	}
}

void MarkovExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

MarkovExperimentHistory::MarkovExperimentHistory(MarkovExperiment* experiment) {
	this->experiment = experiment;
}
