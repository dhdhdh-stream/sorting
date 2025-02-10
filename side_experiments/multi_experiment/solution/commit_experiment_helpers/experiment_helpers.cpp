#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPERIMENTS = 4;
#else
const int NUM_EXPERIMENTS = 40;
#endif /* MDEBUG */

void CommitExperiment::experiment_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		CommitExperimentHistory* history) {
	curr_node = this->best_exit_next_node;

	ScopeHistory* temp_history = new ScopeHistory(this->scope_context);

	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		switch (this->new_nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  temp_history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  temp_history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  temp_history);
			}
			break;
		}

		if (run_helper.early_exit) {
			break;
		}

		if (n_index == this->experiment_index) {
			BranchExperimentHistory* branch_experiment_history = new BranchExperimentHistory(this->curr_experiment);
			history->branch_experiment_history = branch_experiment_history;

			bool is_selected = this->curr_experiment->commit_activate(
				curr_node,
				problem,
				run_helper,
				scope_history,
				temp_history,
				branch_experiment_history);
			if (is_selected) {
				break;
			}
		}
	}

	delete temp_history;
}

void CommitExperiment::experiment_backprop(
		CommitExperimentHistory* history) {
	if (history->branch_experiment_history != NULL) {
		history->branch_experiment_history->impact = history->impact;

		this->curr_experiment->backprop(history->branch_experiment_history);
	}
}

void CommitExperiment::experiment_update() {
	this->curr_experiment->update();

	if (this->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
		delete this->curr_experiment;

		uniform_int_distribution<int> experiment_node_distribution(0, this->new_nodes.size() / 2 - 1);
		this->experiment_index = 2 * experiment_node_distribution(generator) + 1;

		this->curr_experiment = new BranchExperiment(this->scope_context,
													 this->new_nodes[this->experiment_index],
													 false);
		this->curr_experiment->parent_experiment = this;
		cout << "inner BranchExperiment" << endl;
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
			double new_average_score = this->best_experiment->combined_score
				/ this->best_experiment->state_iter;
			if (new_average_score > this->existing_average_score) {
				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		} else {
			uniform_int_distribution<int> experiment_node_distribution(0, this->new_nodes.size() / 2 - 1);
			this->experiment_index = 2 * experiment_node_distribution(generator) + 1;

			this->curr_experiment = new BranchExperiment(this->scope_context,
														 this->new_nodes[this->experiment_index],
														 false);
			this->curr_experiment->parent_experiment = this;
			cout << "inner BranchExperiment" << endl;
		}
	}
}
