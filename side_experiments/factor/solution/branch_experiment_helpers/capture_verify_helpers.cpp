#if defined(MDEBUG) && MDEBUG

#include "branch_experiment.h"

using namespace std;

void BranchExperiment::capture_verify_activate(AbstractNode*& curr_node,
											   Problem* problem,
											   vector<ContextLayer>& context,
											   RunHelper& run_helper,
											   ScopeHistory* scope_history) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	run_helper.has_explore = true;

	map<pair<int,int>, double> factors;
	gather_factors(run_helper,
				   scope_history,
				   factors);

	double sum_vals = 0.0;
	for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
		map<pair<int,int>, double>::iterator it = factors.find(this->new_factor_ids[f_index]);
		if (it != factors.end()) {
			sum_vals += this->new_factor_weights[f_index] * it->second;
		}
	}

	this->verify_scores.push_back(sum_vals);

	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	cout << "context scope" << endl;
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		cout << c_index << ": " << context[c_index].scope->id << endl;
	}
	cout << "context node" << endl;
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		cout << c_index << ": " << context[c_index].node_id << endl;
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
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->best_actions[s_index]);
			} else {
				context.back().node_id = -1;

				this->best_scopes[s_index]->activate(problem,
					context,
					run_helper);
			}

			run_helper.num_actions += 2;
		}
	}
}

void BranchExperiment::capture_verify_backprop() {
	if (this->verify_problems[this->state_iter] != NULL) {
		this->state_iter++;
		if (this->state_iter >= NUM_VERIFY_SAMPLES) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		}
	}
}

#endif /* MDEBUG */