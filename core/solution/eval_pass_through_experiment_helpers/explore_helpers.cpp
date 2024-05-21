#include "eval_pass_through_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int EXPLORE_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 40;
const int EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void EvalPassThroughExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
	if (this->info_scope == NULL) {
		for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
			if (this->step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->actions[s_index]->action);
			} else {
				this->scopes[s_index]->explore_activate(
					problem,
					run_helper);
			}
		}

		curr_node = this->exit_next_node;
	} else {
		ScopeHistory* inner_scope_history;
		bool inner_is_positive;
		this->info_scope->activate(problem,
								   run_helper,
								   inner_scope_history,
								   inner_is_positive);

		delete inner_scope_history;

		if ((this->is_negate && !inner_is_positive)
				|| (!this->is_negate && inner_is_positive)) {
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->actions[s_index]->action);
				} else {
					this->scopes[s_index]->explore_activate(
						problem,
						run_helper);
				}
			}

			curr_node = this->exit_next_node;
		}
	}
}

void EvalPassThroughExperiment::explore_backprop(
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_scope_history->experiment_histories.back();

	double target_val;
	if (context.size() == 1) {
		double starting_score = 1.0;
		double ending_score = problem->score_result(run_helper.num_decisions);
		target_val = ending_score - starting_score;
	} else {
		context[context.size()-2].scope->eval->activate(
			problem,
			run_helper,
			history->outer_eval_history->end_scope_history);
		target_val = context[context.size()-2].scope->eval->calc_vs(
			run_helper,
			history->outer_eval_history);
	}

	this->new_score += target_val - this->existing_average_score;

	this->sub_state_iter++;
	if (this->sub_state_iter == INITIAL_NUM_SAMPLES_PER_ITER) {
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->new_score < 0.0) {
		#endif /* MDEBUG */
			int experiment_index;
			for (int e_index = 0; e_index < (int)this->node_context->experiments.size(); e_index++) {
				if (this->node_context->experiments[e_index] == this) {
					experiment_index = e_index;
					break;
				}
			}
			this->node_context->experiments.erase(this->node_context->experiments.begin() + experiment_index);
			this->node_context = NULL;

			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->actions[s_index];
				} else {
					delete this->scopes[s_index];
				}
			}

			this->step_types.clear();
			this->actions.clear();
			this->scopes.clear();

			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				vector<AbstractNode*> possible_starts;
				this->scope_context->random_exit_activate(
					this->scope_context->nodes[0],
					possible_starts);

				uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
				this->node_context = possible_starts[start_distribution(generator)];

				AbstractNode* starting_node;
				switch (this->node_context->type) {
				case NODE_TYPE_ACTION:
					{
						this->is_branch = false;

						ActionNode* action_node = (ActionNode*)this->node_context;
						starting_node = action_node->next_node;
					}
					break;
				case NODE_TYPE_INFO_SCOPE:
					{
						this->is_branch = false;

						InfoScopeNode* info_scope_node = (InfoScopeNode*)this->node_context;
						starting_node = info_scope_node->next_node;
					}
					break;
				case NODE_TYPE_INFO_BRANCH:
					{
						uniform_int_distribution<int> is_branch_distribution(0, 1);
						this->is_branch = is_branch_distribution(generator);

						InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
						if (this->is_branch) {
							starting_node = info_branch_node->branch_next_node;
						} else {
							starting_node = info_branch_node->original_next_node;
						}
					}
					break;
				}

				this->node_context->experiments.push_back(this);

				vector<AbstractNode*> possible_exits;

				if (this->node_context->type == NODE_TYPE_ACTION
						&& ((ActionNode*)this->node_context)->next_node == NULL) {
					possible_exits.push_back(NULL);
				}

				this->scope_context->random_exit_activate(
					starting_node,
					possible_exits);

				uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
				int random_index = distribution(generator);
				this->exit_next_node = possible_exits[random_index];

				this->info_scope = get_existing_info_scope();
				uniform_int_distribution<int> negate_distribution(0, 1);
				this->is_negate = negate_distribution(generator) == 0;

				int new_num_steps;
				uniform_int_distribution<int> uniform_distribution(0, 1);
				geometric_distribution<int> geometric_distribution(0.5);
				if (random_index == 0) {
					new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
				} else {
					new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
				}

				for (int s_index = 0; s_index < new_num_steps; s_index++) {
					InfoScopeNode* new_scope_node = create_existing_info_scope_node();
					if (new_scope_node != NULL) {
						this->step_types.push_back(STEP_TYPE_SCOPE);
						this->actions.push_back(NULL);

						this->scopes.push_back(new_scope_node);
					} else {
						this->step_types.push_back(STEP_TYPE_ACTION);

						ActionNode* new_action_node = new ActionNode();
						new_action_node->action = problem_type->random_action();
						this->actions.push_back(new_action_node);

						this->scopes.push_back(NULL);
					}
				}

				this->new_score = 0.0;

				this->sub_state_iter = 0;
			}
		}
	} else if (this->sub_state_iter >= NUM_DATAPOINTS) {
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->new_score >= 0.0) {
		#endif /* MDEBUG */
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					this->actions[s_index]->parent = this->scope_context;
					this->actions[s_index]->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
				} else {
					this->scopes[s_index]->parent = this->scope_context;
					this->scopes[s_index]->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
				}
			}

			int exit_node_id;
			AbstractNode* exit_node;
			if (this->exit_next_node == NULL) {
				ActionNode* new_ending_node = new ActionNode();
				new_ending_node->parent = this->scope_context;
				new_ending_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				new_ending_node->action = Action(ACTION_NOOP);

				new_ending_node->next_node_id = -1;
				new_ending_node->next_node = NULL;

				this->ending_node = new_ending_node;

				exit_node_id = new_ending_node->id;
				exit_node = new_ending_node;
			} else {
				exit_node_id = this->exit_next_node->id;
				exit_node = this->exit_next_node;
			}

			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				int next_node_id;
				AbstractNode* next_node;
				if (s_index == (int)this->step_types.size()-1) {
					next_node_id = exit_node_id;
					next_node = exit_node;
				} else {
					if (this->step_types[s_index+1] == STEP_TYPE_ACTION) {
						next_node_id = this->actions[s_index+1]->id;
						next_node = this->actions[s_index+1];
					} else {
						next_node_id = this->scopes[s_index+1]->id;
						next_node = this->scopes[s_index+1];
					}
				}

				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					this->actions[s_index]->next_node_id = next_node_id;
					this->actions[s_index]->next_node = next_node;
				} else if (this->step_types[s_index] == STEP_TYPE_SCOPE) {
					this->scopes[s_index]->next_node_id = next_node_id;
					this->scopes[s_index]->next_node = next_node;
				}
			}

			uniform_int_distribution<int> new_distribution(0, 3);
			if (!new_distribution(generator)) {
				this->score_input_node_contexts = this->eval_context->score_input_node_contexts;
				this->score_input_obs_indexes = this->eval_context->score_input_obs_indexes;

				this->score_network_input_indexes = this->eval_context->score_network_input_indexes;
				if (this->eval_context->score_network != NULL) {
					this->score_network = new Network(this->eval_context->score_network);
				}

				this->vs_input_is_start = this->eval_context->vs_input_is_start;
				this->vs_input_node_contexts = this->eval_context->vs_input_node_contexts;
				this->vs_input_obs_indexes = this->eval_context->vs_input_obs_indexes;

				this->vs_network_input_indexes = this->eval_context->vs_network_input_indexes;
				if (this->eval_context->vs_network != NULL) {
					this->vs_network = new Network(this->eval_context->vs_network);
				}
			}

			this->start_scope_histories.reserve(NUM_DATAPOINTS);
			this->end_scope_histories.reserve(NUM_DATAPOINTS);
			this->end_target_val_histories.reserve(NUM_DATAPOINTS);

			this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
		} else {
			int experiment_index;
			for (int e_index = 0; e_index < (int)this->node_context->experiments.size(); e_index++) {
				if (this->node_context->experiments[e_index] == this) {
					experiment_index = e_index;
					break;
				}
			}
			this->node_context->experiments.erase(this->node_context->experiments.begin() + experiment_index);
			this->node_context = NULL;

			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->actions[s_index];
				} else {
					delete this->scopes[s_index];
				}
			}

			this->step_types.clear();
			this->actions.clear();
			this->scopes.clear();

			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				vector<AbstractNode*> possible_starts;
				this->scope_context->random_exit_activate(
					this->scope_context->nodes[0],
					possible_starts);

				uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
				this->node_context = possible_starts[start_distribution(generator)];

				AbstractNode* starting_node;
				switch (this->node_context->type) {
				case NODE_TYPE_ACTION:
					{
						this->is_branch = false;

						ActionNode* action_node = (ActionNode*)this->node_context;
						starting_node = action_node->next_node;
					}
					break;
				case NODE_TYPE_INFO_SCOPE:
					{
						this->is_branch = false;

						InfoScopeNode* info_scope_node = (InfoScopeNode*)this->node_context;
						starting_node = info_scope_node->next_node;
					}
					break;
				case NODE_TYPE_INFO_BRANCH:
					{
						/**
						 * TODO: set this->is_branch more accurately
						 */
						uniform_int_distribution<int> is_branch_distribution(0, 1);
						this->is_branch = is_branch_distribution(generator);

						InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
						if (this->is_branch) {
							starting_node = info_branch_node->branch_next_node;
						} else {
							starting_node = info_branch_node->original_next_node;
						}
					}
					break;
				}

				this->node_context->experiments.push_back(this);

				vector<AbstractNode*> possible_exits;

				if (this->node_context->type == NODE_TYPE_ACTION
						&& ((ActionNode*)this->node_context)->next_node == NULL) {
					possible_exits.push_back(NULL);
				}

				this->scope_context->random_exit_activate(
					starting_node,
					possible_exits);

				uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
				int random_index = distribution(generator);
				this->exit_next_node = possible_exits[random_index];

				this->info_scope = get_existing_info_scope();
				uniform_int_distribution<int> negate_distribution(0, 1);
				this->is_negate = negate_distribution(generator) == 0;

				int new_num_steps;
				uniform_int_distribution<int> uniform_distribution(0, 1);
				geometric_distribution<int> geometric_distribution(0.5);
				if (random_index == 0) {
					new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
				} else {
					new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
				}

				for (int s_index = 0; s_index < new_num_steps; s_index++) {
					InfoScopeNode* new_scope_node = create_existing_info_scope_node();
					if (new_scope_node != NULL) {
						this->step_types.push_back(STEP_TYPE_SCOPE);
						this->actions.push_back(NULL);

						this->scopes.push_back(new_scope_node);
					} else {
						this->step_types.push_back(STEP_TYPE_ACTION);

						ActionNode* new_action_node = new ActionNode();
						new_action_node->action = problem_type->random_action();
						this->actions.push_back(new_action_node);

						this->scopes.push_back(NULL);
					}
				}

				this->new_score = 0.0;

				this->sub_state_iter = 0;
			}
		}
	}
}
