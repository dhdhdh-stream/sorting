#include "seed_experiment_filter.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "seed_experiment.h"

using namespace std;

void SeedExperimentFilter::train_filter_activate(AbstractNode*& curr_node,
												 Problem* problem,
												 vector<ContextLayer>& context,
												 int& exit_depth,
												 AbstractNode*& exit_node,
												 RunHelper& run_helper) {
	bool is_target = false;
	SeedExperimentOverallHistory* overall_history = (SeedExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->instance_count++;
	if (!overall_history->has_target) {
		double target_probability;
		if (overall_history->instance_count > this->parent->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->parent->average_instances_per_run - overall_history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		overall_history->has_target = true;

		this->parent->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->scope_context.size()].scope_history));

		curr_node = this->seed_next_node;
	} else {
		for (int s_index = 0; s_index < (int)this->filter_step_types.size(); s_index++) {
			if (this->filter_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = new ActionNodeHistory(this->filter_actions[s_index]);
				this->filter_actions[s_index]->activate(curr_node,
														problem,
														context,
														exit_depth,
														exit_node,
														run_helper,
														action_node_history);
				delete action_node_history;
			} else if (this->filter_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->filter_existing_scopes[s_index]);
				this->filter_existing_scopes[s_index]->activate(curr_node,
																problem,
																context,
																exit_depth,
																exit_node,
																run_helper,
																scope_node_history);
				delete scope_node_history;
			} else {
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->filter_potential_scopes[s_index]);
				this->filter_potential_scopes[s_index]->activate(curr_node,
																 problem,
																 context,
																 exit_depth,
																 exit_node,
																 run_helper,
																 scope_node_history);
				delete scope_node_history;
			}
		}

		if (this->filter_exit_depth == 0) {
			curr_node = this->filter_exit_next_node;
		} else {
			exit_depth = this->filter_exit_depth-1;
			exit_node = this->filter_exit_next_node;
		}
	}
}
