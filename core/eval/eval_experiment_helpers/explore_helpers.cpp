#include "eval_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int EXPLORE_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 40;
const int EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void EvalExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem) {
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		problem->perform_action(this->actions[a_index]->action);
	}

	curr_node = this->exit_next_node;
}

void EvalExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->new_score += target_val - this->original_average_score;

	this->sub_state_iter++;
	if (this->sub_state_iter == INITIAL_NUM_SAMPLES_PER_ITER) {
		if (this->new_score < 0.0) {
			for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
				delete this->actions[a_index];
			}

			this->new_score = 0.0;
			this->actions.clear();

			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				vector<AbstractNode*> possible_exits;

				if (this->node_context->type == NODE_TYPE_ACTION
						&& ((ActionNode*)this->node_context)->next_node == NULL) {
					possible_exits.push_back(NULL);
				}

				AbstractNode* starting_node;
				if (this->node_context->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->node_context;
					starting_node = action_node->next_node;
				} else {
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						starting_node = branch_node->branch_next_node;
					} else {
						starting_node = branch_node->original_next_node;
					}
				}

				solution->eval->subscope->random_exit_activate(
					starting_node,
					possible_exits);

				uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
				int random_index = distribution(generator);
				this->exit_next_node = possible_exits[random_index];

				int new_num_steps;
				uniform_int_distribution<int> uniform_distribution(0, 1);
				geometric_distribution<int> geometric_distribution(0.5);
				if (random_index == 0) {
					new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
				} else {
					new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
				}

				for (int s_index = 0; s_index < new_num_steps; s_index++) {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->action = problem_type->random_action();
					this->actions.push_back(new_action_node);
				}

				this->sub_state_iter = 0;
			}
		}
	} else if (this->sub_state_iter >= NUM_DATAPOINTS) {
		if (this->new_score >= 0.0) {
			this->new_decision_scope_histories.reserve(NUM_DATAPOINTS);
			this->new_final_scope_histories.reserve(NUM_DATAPOINTS);
			this->new_target_val_histories.reserve(NUM_DATAPOINTS);

			this->state = EVAL_EXPERIMENT_STATE_CAPTURE_NEW;
			this->state_iter = 0;
		} else {
			for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
				delete this->actions[a_index];
			}

			this->new_score = 0.0;
			this->actions.clear();

			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				vector<AbstractNode*> possible_exits;

				if (this->node_context->type == NODE_TYPE_ACTION
						&& ((ActionNode*)this->node_context)->next_node == NULL) {
					possible_exits.push_back(NULL);
				}

				AbstractNode* starting_node;
				if (this->node_context->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->node_context;
					starting_node = action_node->next_node;
				} else {
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						starting_node = branch_node->branch_next_node;
					} else {
						starting_node = branch_node->original_next_node;
					}
				}

				solution->eval->subscope->random_exit_activate(
					starting_node,
					possible_exits);

				uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
				int random_index = distribution(generator);
				this->exit_next_node = possible_exits[random_index];

				int new_num_steps;
				uniform_int_distribution<int> uniform_distribution(0, 1);
				geometric_distribution<int> geometric_distribution(0.5);
				if (random_index == 0) {
					new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
				} else {
					new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
				}

				for (int s_index = 0; s_index < new_num_steps; s_index++) {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->action = problem_type->random_action();
					this->actions.push_back(new_action_node);
				}

				this->sub_state_iter = 0;
			}
		}
	}
}
