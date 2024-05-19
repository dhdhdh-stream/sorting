// #include "new_info_experiment.h"

// #include "action_node.h"
// #include "branch_node.h"
// #include "constants.h"
// #include "globals.h"
// #include "info_scope_node.h"
// #include "network.h"
// #include "scope.h"
// #include "scope_node.h"
// #include "solution.h"
// #include "solution_helpers.h"
// #include "utilities.h"

// using namespace std;

// bool NewInfoExperiment::root_verify_activate(
// 		AbstractNode*& curr_node,
// 		Problem* problem,
// 		RunHelper& run_helper) {
// 	if (this->is_pass_through) {
// 		if (this->best_step_types.size() == 0) {
// 			curr_node = this->best_exit_next_node;
// 		} else {
// 			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
// 				curr_node = this->best_actions[0];
// 			} else {
// 				curr_node = this->best_scopes[0];
// 			}
// 		}

// 		return true;
// 	} else {
// 		run_helper.num_decisions++;

// 		vector<ContextLayer> inner_context;
// 		inner_context.push_back(ContextLayer());

// 		inner_context.back().scope = this->new_info_subscope;
// 		inner_context.back().node = NULL;

// 		ScopeHistory* scope_history = new ScopeHistory(this->new_info_subscope);
// 		inner_context.back().scope_history = scope_history;

// 		this->new_info_subscope->activate(problem,
// 										  inner_context,
// 										  run_helper,
// 										  scope_history);

// 		vector<double> input_vals(this->input_node_contexts.size(), 0.0);
// 		for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
// 			map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(
// 				this->input_node_contexts[i_index]);
// 			if (it != scope_history->node_histories.end()) {
// 				switch (this->input_node_contexts[i_index]->type) {
// 				case NODE_TYPE_ACTION:
// 					{
// 						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
// 						input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
// 					}
// 					break;
// 				case NODE_TYPE_INFO_SCOPE:
// 					{
// 						InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
// 						if (info_scope_node_history->is_positive) {
// 							input_vals[i_index] = 1.0;
// 						} else {
// 							input_vals[i_index] = -1.0;
// 						}
// 					}
// 					break;
// 				}
// 			}
// 		}

// 		delete scope_history;

// 		double existing_predicted_score = this->existing_average_score;
// 		for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
// 			existing_predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
// 		}
// 		if (this->existing_network != NULL) {
// 			vector<vector<double>> existing_network_input_vals(this->existing_network_input_indexes.size());
// 			for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
// 				existing_network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
// 				for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
// 					existing_network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
// 				}
// 			}
// 			this->existing_network->activate(existing_network_input_vals);
// 			existing_predicted_score += this->existing_network->output->acti_vals[0];
// 		}

// 		double new_predicted_score = this->new_average_score;
// 		for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
// 			new_predicted_score += input_vals[i_index] * this->new_linear_weights[i_index];
// 		}
// 		if (this->new_network != NULL) {
// 			vector<vector<double>> new_network_input_vals(this->new_network_input_indexes.size());
// 			for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
// 				new_network_input_vals[i_index] = vector<double>(this->new_network_input_indexes[i_index].size());
// 				for (int s_index = 0; s_index < (int)this->new_network_input_indexes[i_index].size(); s_index++) {
// 					new_network_input_vals[i_index][s_index] = input_vals[this->new_network_input_indexes[i_index][s_index]];
// 				}
// 			}
// 			this->new_network->activate(new_network_input_vals);
// 			new_predicted_score += this->new_network->output->acti_vals[0];
// 		}

// 		#if defined(MDEBUG) && MDEBUG
// 		bool decision_is_branch;
// 		if (run_helper.curr_run_seed%2 == 0) {
// 			decision_is_branch = true;
// 		} else {
// 			decision_is_branch = false;
// 		}
// 		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
// 		#else
// 		bool decision_is_branch = new_predicted_score >= existing_predicted_score;
// 		#endif /* MDEBUG */

// 		if (decision_is_branch) {
// 			if (this->best_step_types.size() == 0) {
// 				curr_node = this->best_exit_next_node;
// 			} else {
// 				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
// 					curr_node = this->best_actions[0];
// 				} else {
// 					curr_node = this->best_scopes[0];
// 				}
// 			}

// 			return true;
// 		} else {
// 			return false;
// 		}
// 	}
// }
