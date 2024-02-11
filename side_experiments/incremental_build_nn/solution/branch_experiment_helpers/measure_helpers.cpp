#include "branch_experiment.h"

using namespace std;

void BranchExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
			action_node->hook_indexes.push_back(i_index);
			action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
			action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
		} else {
			BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
			branch_node->hook_indexes.push_back(i_index);
			branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
			branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
		}
	}
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  input_vals,
					  context[context.size() - this->scope_context.size()].scope_history);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
			action_node->hook_indexes.clear();
			action_node->hook_scope_contexts.clear();
			action_node->hook_node_contexts.clear();
		} else {
			BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
			branch_node->hook_indexes.clear();
			branch_node->hook_scope_contexts.clear();
			branch_node->hook_node_contexts.clear();
		}
	}

	double existing_predicted_score = this->existing_average_score;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		existing_predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
	}
	if (this->existing_network != NULL) {
		vector<vector<double>> existing_network_input_vals(this->existing_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
			existing_network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
				existing_network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
			}
		}
		this->existing_network->activate(existing_network_input_vals);
		existing_predicted_score += this->existing_network->output->acti_vals[0];
	}

	double new_predicted_score = this->new_average_score;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		new_predicted_score += input_vals[i_index] * this->new_linear_weights[i_index];
	}
	if (this->new_network != NULL) {
		vector<vector<double>> new_network_input_vals(this->new_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
			new_network_input_vals[i_index] = vector<double>(this->new_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->new_network_input_indexes[i_index].size(); s_index++) {
				new_network_input_vals[i_index][s_index] = input_vals[this->new_network_input_indexes[i_index][s_index]];
			}
		}
		this->new_network->activate(new_network_input_vals);
		new_predicted_score += this->new_network->output->acti_vals[0];
	}

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch = new_predicted_score > existing_predicted_score;
	#endif /* MDEBUG */

	if (decision_is_branch) {
		this->branch_count++;

		BranchExperimentInstanceHistory* instance_history = new BranchExperimentInstanceHistory(this);
		history = instance_history;

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
				instance_history->step_histories.push_back(action_node_history);
				this->best_actions[s_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					action_node_history);
			} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_existing_scopes[s_index]);
				instance_history->step_histories.push_back(scope_node_history);
				this->best_existing_scopes[s_index]->potential_activate(
					problem,
					context,
					run_helper,
					scope_node_history);
			} else {
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_potential_scopes[s_index]);
				instance_history->step_histories.push_back(scope_node_history);
				this->best_potential_scopes[s_index]->potential_activate(
					problem,
					context,
					run_helper,
					scope_node_history);
			}
		}

		if (this->best_exit_depth == 0) {
			curr_node = this->best_exit_node;
		} else {
			exit_depth = this->best_exit_depth-1;
			exit_node = this->best_exit_node;
		}
	} else {
		this->original_count++;
	}
}

void BranchExperiment::measure_backprop(double target_val,
										RunHelper& run_helper) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		this->combined_score /= solution->curr_num_datapoints;

		// cout << "Branch" << endl;
		// cout << "measure" << endl;
		// cout << "this->scope_context:" << endl;
		// for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		// 	cout << c_index << ": " << this->scope_context[c_index] << endl;
		// }
		// cout << "this->node_context:" << endl;
		// for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
		// 	if (this->node_context[c_index] == NULL) {
		// 		cout << c_index << ": -1" << endl;
		// 	} else {
		// 		cout << c_index << ": " << this->node_context[c_index] << endl;
		// 	}
		// }
		// cout << "new explore path:";
		// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
		// 		cout << " " << this->best_actions[s_index]->action.move;
		// 	} else {
		// 		cout << " S";
		// 	}
		// }
		// cout << endl;

		// cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
		// if (this->best_exit_node == NULL) {
		// 	cout << "this->best_exit_node_id: " << -1 << endl;
		// } else {
		// 	cout << "this->best_exit_node_id: " << this->best_exit_node->id << endl;
		// }

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double combined_improvement = this->combined_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));

		// cout << "this->combined_score: " << this->combined_score << endl;
		// cout << "this->existing_average_score: " << this->existing_average_score << endl;
		// cout << "score_standard_deviation: " << score_standard_deviation << endl;
		// cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

		double branch_weight = (double)this->branch_count / (double)(this->original_count + this->branch_count);

		// cout << "branch_weight: " << branch_weight << endl;

		// cout << endl;

		if (branch_weight > 0.01 && combined_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			if (this->parent_pass_through_experiment != NULL) {
				#if defined(MDEBUG) && MDEBUG
				if (this->original_count == 0) {
					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else {
					this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
					this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

					this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
					this->state_iter = 0;
				}
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				this->combined_score = 0.0;
				this->original_count = 0;
				this->branch_count = 0;

				this->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

				this->state = BRANCH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;
				this->state_iter = 0;
			}
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
