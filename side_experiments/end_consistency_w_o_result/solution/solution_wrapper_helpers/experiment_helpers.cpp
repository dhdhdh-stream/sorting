#include "solution_wrapper.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

const int RUN_TIMESTEPS = 100;

const int SNAPSHOT_ITERS = 10;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->curr_experiment != NULL) {
		switch (this->curr_experiment->type) {
		case EXPERIMENT_TYPE_BRANCH:
			{
				BranchExperiment* branch_experiment = (BranchExperiment*)this->curr_experiment;
				this->experiment_history = new BranchExperimentHistory(branch_experiment);
			}
			break;
		}
	}

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
	this->experiment_context.push_back(NULL);

	scope_history->pre_obs = this->problem->get_observations();
}

tuple<bool,bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	bool fetch_action = false;
	while (!is_next) {
		if (this->node_context.back() == NULL
				&& this->experiment_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				if (this->experiment_context[this->experiment_context.size() - 2] != NULL) {
					AbstractExperiment* experiment = this->experiment_context[this->experiment_context.size() - 2]->experiment;
					experiment->experiment_exit_step(this);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->experiment_exit_step(this);
				}
			}
		} else if (this->experiment_context.back() != NULL) {
			AbstractExperiment* experiment = this->experiment_context.back()->experiment;
			experiment->experiment_step(obs,
										action,
										is_next,
										fetch_action,
										this);
		} else {
			this->node_context.back()->experiment_step(obs,
													   action,
													   is_next,
													   this);
		}
	}

	return tuple<bool,bool,int>{is_done, fetch_action, action};
}

void SolutionWrapper::set_action(int action) {
	AbstractExperiment* experiment = this->experiment_context.back()->experiment;
	experiment->set_action(action,
						   this);
}

void SolutionWrapper::experiment_end(double result) {
	this->scope_histories[0]->post_obs = this->problem->get_observations();

	if (this->has_explore) {
		this->post_scope_histories.push_back(this->scope_histories[0]->copy_signal());
	}

	if (this->curr_experiment == NULL) {
		create_experiment(this->scope_histories[0],
						  this);
	} else {
		this->experiment_history->experiment->backprop(
			result,
			this);

		delete this->experiment_history;
		this->experiment_history = NULL;

		if (this->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
			this->curr_experiment->clean();
			delete this->curr_experiment;

			this->curr_experiment = NULL;

			if (this->best_experiments.size() == 1) {
				if (this->curr_tries >= 10) {
					cout << "reset" << endl;

					delete this->best_experiments[0];
					this->best_experiments.clear();
				}
			}
		} else if (this->curr_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
			this->curr_experiment->clean();

			if (this->best_experiments.size() == 0) {
				this->curr_target = this->curr_experiment->improvement;
				this->curr_tries = 0;
			}

			this->best_experiments.push_back(this->curr_experiment);
			this->curr_experiment = NULL;

			if (this->best_experiments.size() >= EXPERIMENTS_PER_ITER) {
				for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
					this->solution->scopes[s_index]->update_signals();
				}

				double best_consistency = calc_consistency(this->best_experiments[0]);
				int best_index = 0;
				// {
				// 	cout << "0" << endl;
				// 	cout << "consistency: " << best_consistency << endl;
				// 	cout << "improvement: " << this->best_experiments[0]->improvement << endl;
				// 	this->best_experiments[0]->print();
				// }
				for (int e_index = 1; e_index < (int)this->best_experiments.size(); e_index++) {
					double curr_consistency = calc_consistency(this->best_experiments[e_index]);
					// cout << e_index << endl;
					// cout << "consistency: " << curr_consistency << endl;
					// cout << "improvement: " << this->best_experiments[e_index]->improvement << endl;
					// this->best_experiments[e_index]->print();
					if (curr_consistency > best_consistency) {
						best_consistency = curr_consistency;
						best_index = e_index;
					}
				}

				Scope* last_updated_scope = this->best_experiments[best_index]->scope_context;

				this->best_experiments[best_index]->add(this);

				this->solution->curr_score = this->best_experiments[best_index]->calc_new_score();

				for (int e_index = 0; e_index < (int)this->best_experiments.size(); e_index++) {
					delete this->best_experiments[e_index];
				}
				this->best_experiments.clear();

				clean_scope(last_updated_scope);

				this->solution->clean_scopes();

				this->solution->timestamp++;
				// if (this->solution->timestamp >= RUN_TIMESTEPS) {
				// 	this->solution->timestamp = -1;
				// }

				if (this->solution->timestamp % SNAPSHOT_ITERS == 0) {
					double curr_score = 0.0;
					for (int i_index = 0; i_index < 3; i_index++) {
						curr_score += this->solution->improvement_history[this->solution->improvement_history.size()-1 - i_index];
					}

					double existing_score = 0.0;
					if (this->solution_snapshot->improvement_history.size() > 0) {
						for (int i_index = 0; i_index < 3; i_index++) {
							existing_score += this->solution_snapshot->improvement_history[this->solution_snapshot->improvement_history.size()-1 - i_index];
						}
					}

					if (curr_score > existing_score) {
						delete this->solution_snapshot;
						this->solution_snapshot = new Solution(this->solution);
					} else {
						delete this->solution;
						this->solution = new Solution(this->solution_snapshot);
					}
				}
			}
		}
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
