#include "action_node.h"

#include "abstract_experiment.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	double score = problem->perform_action(this->action);
	run_helper.num_actions++;
	double individual_impact = score / run_helper.num_actions;
	for (int h_index = 0; h_index < (int)run_helper.experiment_histories.size(); h_index++) {
		run_helper.experiment_histories[h_index]->impact += individual_impact;
	}
	if (score < 0.0) {
		run_helper.early_exit = true;
	}

	if (!run_helper.early_exit) {
		curr_node = this->next_node;

		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				this,
				false,
				curr_node,
				problem,
				run_helper,
				scope_history);
			if (is_selected) {
				break;
			}
		}
	}
}
