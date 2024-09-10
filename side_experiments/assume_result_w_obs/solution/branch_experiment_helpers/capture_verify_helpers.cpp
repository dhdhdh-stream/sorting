#if defined(MDEBUG) && MDEBUG

#include "branch_experiment.h"

#include <iostream>

using namespace std;

bool BranchExperiment::capture_verify_activate(AbstractNode*& curr_node,
											   Problem* problem,
											   vector<ContextLayer>& context,
											   RunHelper& run_helper) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	run_helper.num_actions++;

	run_helper.num_analyze += (int)this->input_types.size();

	vector<double> input_vals(this->input_types.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		switch (this->input_types[i_index]) {
		case INPUT_TYPE_GLOBAL:
			{
				vector<double> location = problem_type->relative_to_world(
					context.back().starting_location,
					this->input_locations[i_index]);

				map<vector<double>, double>::iterator it = run_helper.world_model.find(location);
				if (it != run_helper.world_model.end()) {
					input_vals[i_index] = it->second;
				}
			}
			break;
		case INPUT_TYPE_LOCAL:
			{
				vector<double> location = problem_type->relative_to_world(
					local_location,
					this->input_locations[i_index]);

				map<vector<double>, double>::iterator it = run_helper.world_model.find(location);
				if (it != run_helper.world_model.end()) {
					input_vals[i_index] = it->second;
				}
			}
			break;
		case INPUT_TYPE_HISTORY:
			{
				map<AbstractNode*, pair<vector<double>,vector<double>>>::iterator it
					= context.back().node_history.find(this->input_node_contexts[i_index]);
				if (it != context.back().node_history.end()) {
					input_vals[i_index] = it->second.second[this->input_obs_indexes[i_index]];
				}
			}
			break;
		}
	}
	this->network->activate(input_vals);
	double predicted_score = this->network->output->acti_vals[0];

	this->verify_scores.push_back(predicted_score);

	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	cout << "context scope" << endl;
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		cout << c_index << ": " << context[c_index].scope->id << endl;
	}
	cout << "context node" << endl;
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		cout << c_index << ": " << context[c_index].node->id << endl;
	}

	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	cout << "decision_is_branch: " << decision_is_branch << endl;

	if (decision_is_branch) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else if (this->best_step_types[0] == STEP_TYPE_SCOPE) {
				curr_node = this->best_scopes[0];
			} else if (this->best_step_types[0] == STEP_TYPE_RETURN) {
				curr_node = this->best_returns[0];
			} else {
				curr_node = this->best_absolute_returns[0];
			}
		}

		return true;
	}

	return false;
}

void BranchExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		this->result = EXPERIMENT_RESULT_SUCCESS;
	}
}

#endif /* MDEBUG */