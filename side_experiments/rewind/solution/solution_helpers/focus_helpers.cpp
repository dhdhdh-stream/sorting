#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "minesweeper.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 10;
#else
const int MEASURE_ITERS = 1000;
#endif /* MDEBUG */

const double MIN_AVG_HITS = 2.0;

void hit_helper(ScopeHistory* scope_history,
				int& hit_count,
				SolutionWrapper* wrapper) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
			if (scope_node_history->scope_history->scope == wrapper->focus_scope) {
				hit_count++;
			} else {
				hit_helper(scope_node_history->scope_history,
						   hit_count,
						   wrapper);
			}
		}
	}
}

double measure_avg_hits_helper(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	int hit_count = 0;
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		vector<ScopeHistory*> scope_histories;
		vector<AbstractNode*> node_context;
		int num_actions = 1;

		ScopeHistory* scope_history = new ScopeHistory(wrapper->solution->scopes[0]);
		scope_histories.push_back(scope_history);
		node_context.push_back(wrapper->solution->scopes[0]->nodes[0]);

		while (true) {
			vector<double> obs = problem->get_observations();

			int action;
			bool is_next = false;
			bool is_done = false;
			while (!is_next) {
				if (node_context.back() == NULL) {
					if (scope_histories.size() == 1) {
						is_next = true;
						is_done = true;
					} else {
						ScopeNode* scope_node = (ScopeNode*)node_context[node_context.size() - 2];
						scope_node->exit_step(scope_histories,
											  node_context);
					}
				} else {
					node_context.back()->step(obs,
											  action,
											  is_next,
											  scope_histories,
											  node_context,
											  num_actions);
				}
			}

			if (is_done) {
				break;
			} else {
				problem->perform_action(action);
			}
		}

		hit_helper(scope_histories[0],
				   hit_count,
				   wrapper);

		delete scope_histories[0];

		delete problem;
	}

	delete problem_type;

	return (double)hit_count / (double)MEASURE_ITERS;
}

void gather_helper(ScopeHistory* scope_history,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch) {
	if (scope_history->scope->is_outer) {
		return;
	}

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
		case NODE_TYPE_ACTION:
		case NODE_TYPE_OBS:
			if (node->experiment == NULL) {
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

				if (node->experiment == NULL) {
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
			if (node->experiment == NULL) {
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

void add_scope_node_helper(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	Problem* problem = problem_type->get_problem();

	vector<ScopeHistory*> scope_histories;
	vector<AbstractNode*> node_context;
	int num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(wrapper->solution->scopes[0]);
	scope_histories.push_back(scope_history);
	node_context.push_back(wrapper->solution->scopes[0]->nodes[0]);

	while (true) {
		vector<double> obs = problem->get_observations();

		int action;
		bool is_next = false;
		bool is_done = false;
		while (!is_next) {
			if (node_context.back() == NULL) {
				if (scope_histories.size() == 1) {
					is_next = true;
					is_done = true;
				} else {
					ScopeNode* scope_node = (ScopeNode*)node_context[node_context.size() - 2];
					scope_node->exit_step(scope_histories,
										  node_context);
				}
			} else {
				node_context.back()->step(obs,
										  action,
										  is_next,
										  scope_histories,
										  node_context,
										  num_actions);
			}
		}

		if (is_done) {
			break;
		} else {
			problem->perform_action(action);
		}
	}

	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	gather_helper(scope_history,
				  node_count,
				  explore_node,
				  explore_is_branch);

	if (explore_node != NULL) {
		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->parent = explore_node->parent;
		new_scope_node->id = explore_node->parent->node_counter;
		explore_node->parent->node_counter++;
		explore_node->parent->nodes[new_scope_node->id] = new_scope_node;

		new_scope_node->scope = wrapper->focus_scope;

		switch (explore_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)explore_node;

				for (int a_index = 0; a_index < (int)start_node->next_node->ancestor_ids.size(); a_index++) {
					if (start_node->next_node->ancestor_ids[a_index] == start_node->id) {
						start_node->next_node->ancestor_ids.erase(
							start_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				new_scope_node->next_node_id = start_node->next_node->id;
				new_scope_node->next_node = start_node->next_node;

				start_node->next_node->ancestor_ids.push_back(new_scope_node->id);

				start_node->next_node_id = new_scope_node->id;
				start_node->next_node = new_scope_node;

				new_scope_node->ancestor_ids.push_back(start_node->id);
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)explore_node;

				for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
					if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
						action_node->next_node->ancestor_ids.erase(
							action_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				new_scope_node->next_node_id = action_node->next_node->id;
				new_scope_node->next_node = action_node->next_node;

				action_node->next_node->ancestor_ids.push_back(new_scope_node->id);

				action_node->next_node_id = new_scope_node->id;
				action_node->next_node = new_scope_node;

				new_scope_node->ancestor_ids.push_back(action_node->id);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)explore_node;

				for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
					if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
						scope_node->next_node->ancestor_ids.erase(
							scope_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				new_scope_node->next_node_id = scope_node->next_node->id;
				new_scope_node->next_node = scope_node->next_node;

				scope_node->next_node->ancestor_ids.push_back(new_scope_node->id);

				scope_node->next_node_id = new_scope_node->id;
				scope_node->next_node = new_scope_node;

				new_scope_node->ancestor_ids.push_back(scope_node->id);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)explore_node;

				if (explore_is_branch) {
					for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->branch_next_node->ancestor_ids.erase(
								branch_node->branch_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					new_scope_node->next_node_id = branch_node->branch_next_node->id;
					new_scope_node->next_node = branch_node->branch_next_node;

					branch_node->branch_next_node->ancestor_ids.push_back(new_scope_node->id);

					branch_node->branch_next_node_id = new_scope_node->id;
					branch_node->branch_next_node = new_scope_node;

					new_scope_node->ancestor_ids.push_back(branch_node->id);
				} else {
					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(
								branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					new_scope_node->next_node_id = branch_node->original_next_node->id;
					new_scope_node->next_node = branch_node->original_next_node;

					branch_node->original_next_node->ancestor_ids.push_back(new_scope_node->id);

					branch_node->original_next_node_id = new_scope_node->id;
					branch_node->original_next_node = new_scope_node;

					new_scope_node->ancestor_ids.push_back(branch_node->id);
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)explore_node;

				for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
					if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
						obs_node->next_node->ancestor_ids.erase(
							obs_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				new_scope_node->next_node_id = obs_node->next_node->id;
				new_scope_node->next_node = obs_node->next_node;

				obs_node->next_node->ancestor_ids.push_back(new_scope_node->id);

				obs_node->next_node_id = new_scope_node->id;
				obs_node->next_node = new_scope_node;

				new_scope_node->ancestor_ids.push_back(obs_node->id);
			}
			break;
		}
	}

	delete problem;

	delete problem_type;
}

void add_focus_helper(SolutionWrapper* wrapper) {
	Scope* new_scope = new Scope();
	new_scope->is_outer = false;
	new_scope->id = wrapper->solution->scopes.size();
	new_scope->node_counter = 0;

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		wrapper->solution->scopes[s_index]->child_scopes.push_back(new_scope);
	}

	wrapper->solution->scopes.push_back(new_scope);

	StartNode* start_node = new StartNode();
	start_node->parent = new_scope;
	start_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[start_node->id] = start_node;

	ObsNode* end_node = new ObsNode();
	end_node->parent = new_scope;
	end_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[end_node->id] = end_node;

	start_node->next_node_id = end_node->id;
	start_node->next_node = end_node;

	end_node->ancestor_ids.push_back(start_node->id);

	end_node->next_node_id = -1;
	end_node->next_node = NULL;

	while (true) {
		add_scope_node_helper(wrapper);

		double avg_hits = measure_avg_hits_helper(wrapper);
		if (avg_hits >= MIN_AVG_HITS) {
			break;
		}
	}

	wrapper->focus_scope = new_scope;
}
