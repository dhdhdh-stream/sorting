#if defined(MDEBUG) && MDEBUG

#include "branch_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::capture_verify_activate(AbstractNode*& curr_node,
											   Problem* problem,
											   RunHelper& run_helper,
											   ScopeHistory* scope_history) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;
	this->verify_can_random[this->state_iter] = run_helper.can_random;

	double sum_vals = this->new_average_score;
	for (int f_index = 0; f_index < (int)this->new_factors.size(); f_index++) {
		double val;
		fetch_input(run_helper,
					scope_history,
					this->new_factors[f_index],
					val);
		sum_vals += this->new_factor_weights[f_index] * val;
	}

	this->verify_scores.push_back(sum_vals);

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

	cout << "decision_is_branch: " << decision_is_branch << endl;

	if (decision_is_branch) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				while (run_helper.is_random()) {
					problem->perform_action(Action(run_helper.curr_run_seed%problem_type->num_possible_actions()));
					run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
				}

				if (!run_helper.is_random()) {
					problem->perform_action(this->best_actions[s_index]);
				}
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);

				inner_scope_history->input_history = vector<double>(this->best_scopes[s_index]->num_inputs, 0.0);
				map<Scope*, vector<Input>>::iterator it = this->scope_context->child_scope_inputs.find(this->best_scopes[s_index]);
				if (it != this->scope_context->child_scope_inputs.end()) {
					for (int i_index = 0; i_index < this->best_scopes[s_index]->num_inputs; i_index++) {
						fetch_input(run_helper,
									scope_history,
									it->second[i_index],
									inner_scope_history->input_history[i_index]);
					}
				}

				this->best_scopes[s_index]->activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}

			run_helper.num_actions += 2;
		}

		curr_node = this->best_exit_next_node;
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