#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::root_verify_activate(
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (this->is_pass_through) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}

		return true;
	} else {
		run_helper.num_decisions++;

		vector<double> existing_input_vals(this->existing_input_node_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->existing_input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = context.back().scope_history->node_histories.find(
				this->existing_input_node_contexts[i_index]);
			if (it != context.back().scope_history->node_histories.end()) {
				switch (it->first->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						existing_input_vals[i_index] = action_node_history->obs_snapshot[this->existing_input_obs_indexes[i_index]];
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
						existing_input_vals[i_index] = scope_node_history->obs_snapshot[this->existing_input_obs_indexes[i_index]];
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							existing_input_vals[i_index] = 1.0;
						} else {
							existing_input_vals[i_index] = -1.0;
						}
					}
					break;
				case NODE_TYPE_INFO_BRANCH:
					{
						InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
						if (info_branch_node_history->is_branch) {
							existing_input_vals[i_index] = 1.0;
						} else {
							existing_input_vals[i_index] = -1.0;
						}
					}
					break;
				}
			}
		}
		this->existing_network->activate(existing_input_vals);
		#if defined(MDEBUG) && MDEBUG
		#else
		double existing_predicted_score = this->existing_network->output->acti_vals[0];
		#endif /* MDEBUG */

		vector<double> new_input_vals(this->new_input_node_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->new_input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = context.back().scope_history->node_histories.find(
				this->new_input_node_contexts[i_index]);
			if (it != context.back().scope_history->node_histories.end()) {
				switch (it->first->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
						new_input_vals[i_index] = scope_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							new_input_vals[i_index] = 1.0;
						} else {
							new_input_vals[i_index] = -1.0;
						}
					}
					break;
				case NODE_TYPE_INFO_BRANCH:
					{
						InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
						if (info_branch_node_history->is_branch) {
							new_input_vals[i_index] = 1.0;
						} else {
							new_input_vals[i_index] = -1.0;
						}
					}
					break;
				}
			}
		}
		this->new_network->activate(new_input_vals);
		#if defined(MDEBUG) && MDEBUG
		#else
		double new_predicted_score = this->new_network->output->acti_vals[0];
		#endif /* MDEBUG */

		#if defined(MDEBUG) && MDEBUG
		bool decision_is_branch;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		bool decision_is_branch = new_predicted_score >= existing_predicted_score;
		#endif /* MDEBUG */

		if (decision_is_branch) {
			if (this->best_step_types.size() == 0) {
				curr_node = this->best_exit_next_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else {
					curr_node = this->best_scopes[0];
				}
			}

			return true;
		} else {
			return false;
		}
	}
}
