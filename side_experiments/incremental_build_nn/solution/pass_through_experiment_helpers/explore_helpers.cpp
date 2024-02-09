#include "pass_through_experiment.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 2;
const int NUM_SAMPLES_PER_ITER = 2;
#else
const int EXPLORE_ITERS = 100;
const int NUM_SAMPLES_PER_ITER = 20;
#endif /* MDEBUG */

void PassThroughExperiment::explore_initial_activate(AbstractNode*& curr_node,
													 Problem* problem,
													 vector<ContextLayer>& context,
													 int& exit_depth,
													 AbstractNode*& exit_node,
													 RunHelper& run_helper) {
	// exit
	vector<pair<int,AbstractNode*>> possible_exits;
	gather_possible_exits(possible_exits,
						  context,
						  this->scope_context,
						  this->node_context);

	uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
	int random_index = distribution(generator);
	this->curr_exit_depth = possible_exits[random_index].first;
	this->curr_exit_node = possible_exits[random_index].second;

	// new path
	int new_num_steps;
	uniform_int_distribution<int> uniform_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.5);
	if (this->curr_exit_depth == 0
			&& this->curr_exit_node == curr_node) {
		new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
	} else {
		new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
	}

	uniform_int_distribution<int> new_scope_distribution(0, 3);
	uniform_int_distribution<int> random_scope_distribution(0, 3);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		ScopeNode* new_scope_node = NULL;
		if (new_scope_distribution(generator) == 0) {
			if (random_scope_distribution(generator) == 0) {
				uniform_int_distribution<int> distribution(0, solution->scopes.size()-1);
				Scope* scope = next(solution->scopes.begin(), distribution(generator))->second;
				scope_node = create_scope(scope);
			} else {
				scope_node = create_scope(context[context.size() - this->scope_context.size()].scope);
			}
		}
		if (new_scope_node != NULL) {
			this->curr_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
			this->curr_actions.push_back(NULL);
			this->curr_existing_scopes.push_back(NULL);

			this->curr_potential_scopes.push_back(new_scope_node);

			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(new_scope_node);
			new_scope_node->potential_activate(problem,
											   context,
											   run_helper,
											   scope_node_history);
			delete scope_node_history;
		} else {
			ScopeNode* new_existing_scope_node = reuse_existing(problem);
			if (new_existing_scope_node != NULL) {
				this->curr_step_types.push_back(STEP_TYPE_EXISTING_SCOPE);
				this->curr_actions.push_back(NULL);

				this->curr_existing_scopes.push_back(new_existing_scope_node);

				this->curr_potential_scopes.push_back(NULL);

				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(new_existing_scope_node);
				new_existing_scope_node->potential_activate(problem,
															context,
															run_helper,
															scope_node_history);
				delete scope_node_history;
			} else {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem->random_action();
				this->curr_actions.push_back(new_action_node);

				this->curr_existing_scopes.push_back(NULL);
				this->curr_potential_scopes.push_back(NULL);

				ActionNodeHistory* action_node_history = new ActionNodeHistory(new_action_node);
				new_action_node->activate(curr_node,
										  problem,
										  context,
										  exit_depth,
										  exit_node,
										  run_helper,
										  action_node_history);
				delete action_node_history;
			}
		}
	}

	if (this->curr_exit_depth == 0) {
		curr_node = this->curr_exit_node;
	} else {
		exit_depth = this->curr_exit_depth-1;
		exit_node = this->curr_exit_node;
	}
}

void PassThroughExperiment::explore_activate(AbstractNode*& curr_node,
											 Problem* problem,
											 vector<ContextLayer>& context,
											 int& exit_depth,
											 AbstractNode*& exit_node,
											 RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
		if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->curr_actions[s_index]);
			this->curr_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
			delete action_node_history;
		} else if (this->curr_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->curr_existing_scopes[s_index]);
			this->curr_existing_scopes[s_index]->potential_activate(
				problem,
				context,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->curr_potential_scopes[s_index]);
			this->curr_potential_scopes[s_index]->potential_activate(
				problem,
				context,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		}
	}

	if (this->curr_exit_depth == 0) {
		curr_node = this->curr_exit_node;
	} else {
		exit_depth = this->curr_exit_depth-1;
		exit_node = this->curr_exit_node;
	}
}

void PassThroughExperiment::explore_backprop(double target_val) {
	this->curr_score += target_val - this->existing_average_score;

	this->sub_state_iter++;
	if (this->sub_state_iter >= NUM_SAMPLES_PER_ITER) {
		this->curr_score /= NUM_SAMPLES_PER_ITER;
		#if defined(MDEBUG) && MDEBUG
		if (true) {
		#else
		if (this->curr_score > this->best_score) {
		#endif /* MDEBUG */
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
					delete this->best_existing_scopes[s_index];
				} else {
					delete this->best_potential_scopes[s_index]->scope;
					delete this->best_potential_scopes[s_index];
				}
			}

			this->best_score = curr_score;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_existing_scopes = this->curr_existing_scopes;
			this->best_potential_scopes = this->curr_potential_scopes;
			this->best_exit_depth = this->curr_exit_depth;
			this->best_exit_node = this->curr_exit_node;

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_existing_scopes.clear();
			this->curr_potential_scopes.clear();
		} else {
			for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
				if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->curr_actions[s_index];
				} else if (this->curr_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
					delete this->curr_existing_scopes[s_index];
				} else {
					delete this->curr_potential_scopes[s_index]->scope;
					delete this->curr_potential_scopes[s_index];
				}
			}

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_potential_scopes.clear();
			this->curr_existing_scopes.clear();
		}

		this->state_iter++;
		this->sub_state_iter = 0;
		if (this->state_iter >= EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->best_score > 0.0) {
			#endif /* MDEBUG */
				Scope* containing_scope = solution->scopes[this->scope_context.back()];
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->parent = containing_scope;
						this->best_actions[s_index]->id = containing_scope->node_counter;
						containing_scope->node_counter++;
					} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
						this->best_existing_scopes[s_index]->parent = containing_scope;
						this->best_existing_scopes[s_index]->id = containing_scope->node_counter;
						containing_scope->node_counter++;
					} else {
						this->best_potential_scopes[s_index]->parent = containing_scope;
						this->best_potential_scopes[s_index]->id = containing_scope->node_counter;
						containing_scope->node_counter++;

						int new_scope_id = solution->scope_counter;
						solution->scope_counter++;
						this->best_potential_scopes[s_index]->scope->id = new_scope_id;

						for (map<int, AbstractNode*>::iterator it = this->best_potential_scopes[s_index]->scope->nodes.begin();
								it != this->best_potential_scopes[s_index]->scope->nodes.end(); it++) {
							if (it->second->type == NODE_TYPE_BRANCH) {
								BranchNode* branch_node = (BranchNode*)it->second;
								branch_node->scope_context_ids = vector<int>{new_scope_id};
								branch_node->scope_context = vector<Scope*>{this->best_potential_scopes[s_index]->scope};
								branch_node->node_context_ids = vector<int>{branch_node->id};
								branch_node->node_context = vector<AbstractNode*>{branch_node}
								for (int i_index = 0; i_index < (int)branch_node->input_scope_context_ids.size(); i_index++) {
									if (branch_node->input_scope_context_ids[i_index].size() > 0) {
										branch_node->input_scope_context_ids[i_index][0] = new_scope_id;
									}
								}
							}
						}
					}
				}

				this->o_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW;
				this->state_iter = 0;
			}
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
