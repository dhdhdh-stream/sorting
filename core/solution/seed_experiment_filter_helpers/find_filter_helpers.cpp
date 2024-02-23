#include "seed_experiment_filter.h"

#include "action_node.h"
#include "constants.h"
#include "scope_node.h"
#include "seed_experiment.h"

using namespace std;

void SeedExperimentFilter::find_filter_activate(AbstractNode*& curr_node,
												Problem* problem,
												vector<ContextLayer>& context,
												int& exit_depth,
												AbstractNode*& exit_node,
												RunHelper& run_helper) {
	SeedExperimentOverallHistory* overall_history = (SeedExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->instance_count++;

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
