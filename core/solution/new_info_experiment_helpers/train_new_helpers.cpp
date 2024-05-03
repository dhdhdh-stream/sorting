#include "new_info_experiment.h"

using namespace std;

void NewInfoExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		NewInfoExperimentHistory* history) {
	run_helper.num_decisions++;

	this->num_instances_until_target--;
	if (this->num_instances_until_target == 0) {
		history->instance_count++;

		vector<ContextLayer> inner_context;
		inner_context.push_back(this->new_info_subscope);

		inner_context.back().scope = this->new_info_subscope;
		inner_context.back().node = NULL;

		ScopeHistory* scope_history = new ScopeHistory(this->new_info_subscope);
		context.back().scope_history = scope_history;

		this->new_info_subscope->activate(problem,
										  inner_context,
										  run_helper,
										  scope_history);

		this->i_scope_histories.push_back(scope_history);

		for (int s_index = 0; s_index < (int)this->best_sequence_step_types.size(); s_index++) {
			if (this->best_sequence_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->best_sequence_actions[s_index]->action);
			} else {
				this->best_sequence_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper);
			}
		}

		curr_node = this->best_sequence_exit_next_node;

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);
	}
}

void NewInfoExperiment::train_new_backprop() {

}
