// #if defined(MDEBUG) && MDEBUG

// #include "info_pass_through_experiment.h"

// #include <iostream>

// #include "action_node.h"
// #include "branch_node.h"
// #include "constants.h"
// #include "info_branch_node.h"
// #include "info_scope.h"
// #include "info_scope_node.h"
// #include "network.h"
// #include "problem.h"
// #include "scope.h"
// #include "utilities.h"

// using namespace std;

// void InfoPassThroughExperiment::capture_verify_activate(
// 		AbstractNode*& curr_node,
// 		Problem* problem,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper,
// 		InfoPassThroughExperimentHistory* history) {
// 	if (this->verify_problems[this->state_iter] == NULL) {
// 		this->verify_problems[this->state_iter] = problem->copy_and_reset();
// 	}
// 	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

// 	if (this->info_scope == NULL) {
// 		if (this->step_types.size() == 0) {
// 			curr_node = this->exit_next_node;
// 		} else {
// 			if (this->step_types[0] == STEP_TYPE_ACTION) {
// 				curr_node = this->actions[0];
// 			} else {
// 				curr_node = this->scopes[0];
// 			}
// 		}
// 	} else {
// 		ScopeHistory* inner_scope_history;
// 		bool inner_is_positive;
// 		this->info_scope->activate(problem,
// 								   run_helper,
// 								   inner_scope_history,
// 								   inner_is_positive);

// 		delete inner_scope_history;

// 		if ((this->is_negate && !inner_is_positive)
// 				|| (!this->is_negate && inner_is_positive)) {
// 			if (this->step_types.size() == 0) {
// 				curr_node = this->exit_next_node;
// 			} else {
// 				if (this->step_types[0] == STEP_TYPE_ACTION) {
// 					curr_node = this->actions[0];
// 				} else {
// 					curr_node = this->scopes[0];
// 				}
// 			}
// 		}
// 	}
// }

// void InfoPassThroughExperiment::capture_verify_back_activate(
// 		Problem* problem,
// 		ScopeHistory*& subscope_history,
// 		bool& result_is_positive,
// 		RunHelper& run_helper) {
// 	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
// 	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
// 		map<AbstractNode*, AbstractNodeHistory*>::iterator it = subscope_history->node_histories.find(
// 			this->input_node_contexts[i_index]);
// 		if (it != subscope_history->node_histories.end()) {
// 			switch (this->input_node_contexts[i_index]->type) {
// 			case NODE_TYPE_ACTION:
// 				{
// 					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
// 					input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
// 				}
// 				break;
// 			case NODE_TYPE_BRANCH:
// 				{
// 					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
// 					if (branch_node_history->is_branch) {
// 						input_vals[i_index] = 1.0;
// 					} else {
// 						input_vals[i_index] = -1.0;
// 					}
// 				}
// 				break;
// 			case NODE_TYPE_INFO_SCOPE:
// 				{
// 					InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
// 					if (info_scope_node_history->is_positive) {
// 						input_vals[i_index] = 1.0;
// 					} else {
// 						input_vals[i_index] = -1.0;
// 					}
// 				}
// 				break;
// 			case NODE_TYPE_INFO_BRANCH:
// 				{
// 					InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
// 					if (info_branch_node_history->is_branch) {
// 						input_vals[i_index] = 1.0;
// 					} else {
// 						input_vals[i_index] = -1.0;
// 					}
// 				}
// 				break;
// 			}
// 		}
// 	}

// 	double negative_score = this->negative_average_score;
// 	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
// 		negative_score += input_vals[i_index] * this->negative_linear_weights[i_index];
// 	}
// 	if (this->negative_network != NULL) {
// 		vector<vector<double>> negative_network_input_vals(this->negative_network_input_indexes.size());
// 		for (int i_index = 0; i_index < (int)this->negative_network_input_indexes.size(); i_index++) {
// 			negative_network_input_vals[i_index] = vector<double>(this->negative_network_input_indexes[i_index].size());
// 			for (int v_index = 0; v_index < (int)this->negative_network_input_indexes[i_index].size(); v_index++) {
// 				negative_network_input_vals[i_index][v_index] = input_vals[this->negative_network_input_indexes[i_index][v_index]];
// 			}
// 		}
// 		this->negative_network->activate(negative_network_input_vals);
// 		negative_score += this->negative_network->output->acti_vals[0];
// 	}

// 	double positive_score = this->positive_average_score;
// 	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
// 		positive_score += input_vals[i_index] * this->positive_linear_weights[i_index];
// 	}
// 	if (this->positive_network != NULL) {
// 		vector<vector<double>> positive_network_input_vals(this->positive_network_input_indexes.size());
// 		for (int i_index = 0; i_index < (int)this->positive_network_input_indexes.size(); i_index++) {
// 			positive_network_input_vals[i_index] = vector<double>(this->positive_network_input_indexes[i_index].size());
// 			for (int v_index = 0; v_index < (int)this->positive_network_input_indexes[i_index].size(); v_index++) {
// 				positive_network_input_vals[i_index][v_index] = input_vals[this->positive_network_input_indexes[i_index][v_index]];
// 			}
// 		}
// 		this->positive_network->activate(positive_network_input_vals);
// 		positive_score += this->positive_network->output->acti_vals[0];
// 	}

// 	this->verify_negative_scores.push_back(negative_score);
// 	this->verify_positive_scores.push_back(positive_score);

// 	cout << "subscope_history->node_histories" << endl;
// 	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = subscope_history->node_histories.begin();
// 			it != subscope_history->node_histories.end(); it++) {
// 		cout << it->first->id << endl;
// 	}

// 	cout << "input_vals:" << endl;
// 	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
// 		cout << i_index << ": " << input_vals[i_index] << endl;
// 	}
// 	cout << "negative_score: " << negative_score << endl;
// 	cout << "positive_score: " << positive_score << endl;
// 	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
// 	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
// 	problem->print();

// 	if (run_helper.curr_run_seed%2 == 0) {
// 		result_is_positive = true;
// 	} else {
// 		result_is_positive = false;
// 	}
// 	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
// }

// void InfoPassThroughExperiment::capture_verify_backprop() {
// 	this->state_iter++;
// 	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
// 		this->result = EXPERIMENT_RESULT_SUCCESS;
// 	}
// }

// #endif /* MDEBUG */