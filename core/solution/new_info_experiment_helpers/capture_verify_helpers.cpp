#if defined(MDEBUG) && MDEBUG

#include "new_info_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "utilities.h"

using namespace std;

bool NewInfoExperiment::capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
	run_helper.num_decisions++;

	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	vector<ContextLayer> inner_context;
	inner_context.push_back(ContextLayer());

	inner_context.back().scope = this->new_info_subscope;
	inner_context.back().node = NULL;

	ScopeHistory* scope_history = new ScopeHistory(this->new_info_subscope);
	inner_context.back().scope_history = scope_history;

	this->new_info_subscope->activate(problem,
									  inner_context,
									  run_helper,
									  scope_history);

	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(
			this->input_node_contexts[i_index]);
		if (it != scope_history->node_histories.end()) {
			switch (this->input_node_contexts[i_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_INFO_SCOPE:
				{
					InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
					if (info_scope_node_history->is_positive) {
						input_vals[i_index] = 1.0;
					} else {
						input_vals[i_index] = -1.0;
					}
				}
				break;
			}
		}
	}

	delete scope_history;

	double existing_predicted_score = this->existing_average_score;
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
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
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
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

	this->verify_negative_scores.push_back(existing_predicted_score);
	this->verify_positive_scores.push_back(new_predicted_score);

	cout << "input_vals:" << endl;
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		cout << input_vals[i_index] << endl;
	}
	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

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

void NewInfoExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		if (this->parent_experiment == NULL) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			vector<AbstractExperiment*> verify_experiments;
			verify_experiments.insert(verify_experiments.begin(), this);
			AbstractExperiment* curr_experiment = this->parent_experiment;
			while (true) {
				if (curr_experiment->parent_experiment == NULL) {
					/**
					 * - don't include root
					 */
					break;
				} else {
					verify_experiments.insert(verify_experiments.begin(), curr_experiment);
					curr_experiment = curr_experiment->parent_experiment;
				}
			}

			this->root_experiment->verify_experiments = verify_experiments;

			this->root_experiment->o_target_val_histories.reserve(VERIFY_1ST_NUM_DATAPOINTS);

			this->root_experiment->root_state = ROOT_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;

			this->state = NEW_INFO_EXPERIMENT_STATE_ROOT_VERIFY;
		}
	}
}

#endif /* MDEBUG */