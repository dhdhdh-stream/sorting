#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPERIMENTS = 4;
#else
/**
 * - limit as can be too difficult to recover from bad commit
 *   - difficult to select good possibilities during explore as all instances now committing
 *     - so cannot use commit existing predicted score
 */
const int NUM_EXPERIMENTS = 10;
#endif /* MDEBUG */

void CommitExperiment::experiment_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		CommitExperimentHistory* history) {
	run_helper.has_explore = true;

	curr_node = this->best_exit_next_node;

	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		switch (this->new_nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		}

		if (n_index == this->experiment_index) {
			if (history->branch_experiment_history == NULL) {
				history->branch_experiment_history = new BranchExperimentHistory(this->curr_experiment);
			}

			bool is_selected = this->curr_experiment->commit_activate(
				curr_node,
				problem,
				run_helper,
				scope_history,
				history->branch_experiment_history);
			if (is_selected) {
				break;
			}
		}
	}
}

void CommitExperiment::experiment_backprop(
		double target_val,
		RunHelper& run_helper,
		CommitExperimentHistory* history) {
	this->curr_experiment->commit_backprop(target_val,
										   run_helper,
										   history->branch_experiment_history);

	bool is_done = false;
	if (this->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
		delete this->curr_experiment;
		this->curr_experiment = NULL;

		this->state_iter++;
		if (this->state_iter >= NUM_EXPERIMENTS) {
			is_done = true;
		} else {
			uniform_int_distribution<int> experiment_node_distribution(0, this->new_nodes.size() / 2 - 1);
			this->experiment_index = 2 * experiment_node_distribution(generator) + 1;

			this->curr_experiment = new BranchExperiment(this->scope_context,
														 this->new_nodes[this->experiment_index],
														 false);
			this->curr_experiment->parent_experiment = this;
		}
	} else if (this->curr_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
		if (this->best_experiment == NULL) {
			this->best_experiment = this->curr_experiment;
			this->curr_experiment = NULL;
		} else {
			double best_average_score = this->best_experiment->combined_score
				/ this->best_experiment->state_iter;
			double curr_average_score = this->curr_experiment->combined_score
				/ this->curr_experiment->state_iter;
			if (curr_average_score > best_average_score) {
				delete this->best_experiment;
				this->best_experiment = this->curr_experiment;
				this->curr_experiment = NULL;
			} else {
				delete this->curr_experiment;
				this->curr_experiment = NULL;
			}
		}

		this->state_iter++;
		if (this->state_iter >= NUM_EXPERIMENTS) {
			is_done = true;
		} else {
			uniform_int_distribution<int> experiment_node_distribution(0, this->new_nodes.size() / 2 - 1);
			this->experiment_index = 2 * experiment_node_distribution(generator) + 1;

			this->curr_experiment = new BranchExperiment(this->scope_context,
														 this->new_nodes[this->experiment_index],
														 false);
			this->curr_experiment->parent_experiment = this;
		}
	}

	if (is_done) {
		if (this->best_experiment == NULL) {
			this->result = EXPERIMENT_RESULT_FAIL;
		} else {
			double new_average_score = this->best_experiment->combined_score
				/ this->best_experiment->state_iter;
			if (new_average_score > this->o_existing_average_score) {
				cout << "selected BranchExperiment" << endl;
				cout << "this->best_experiment->scope_context->id: " << this->best_experiment->scope_context->id << endl;
				cout << "this->best_experiment->node_context->id: " << this->best_experiment->node_context->id << endl;
				cout << "this->best_experiment->is_branch: " << this->best_experiment->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_experiment->best_step_types.size(); s_index++) {
					if (this->best_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_experiment->best_actions[s_index].move;
					} else {
						cout << " E" << this->best_experiment->best_scopes[s_index]->id;
					}
				}
				cout << endl;

				if (this->best_experiment->best_exit_next_node == NULL) {
					cout << "this->best_experiment->best_exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->best_experiment->best_exit_next_node->id: " << this->best_experiment->best_exit_next_node->id << endl;
				}

				cout << "this->best_experiment->new_average_score: " << this->best_experiment->new_average_score << endl;
				cout << "this->best_experiment->select_percentage: " << this->best_experiment->select_percentage << endl;

				cout << "this->best_experiment->existing_average_score: " << this->best_experiment->existing_average_score << endl;

				cout << "this->o_existing_average_score: " << this->existing_average_score << endl;
				cout << "new_average_score: " << new_average_score << endl;

				cout << endl;

				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
