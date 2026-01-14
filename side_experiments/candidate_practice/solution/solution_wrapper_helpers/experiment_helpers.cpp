#include "solution_wrapper.h"

#include <iostream>

#include "constants.h"
#include "experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "tunnel.h"
#include "utilities.h"

using namespace std;

const int RUN_TIMESTEPS = 100;

const int SNAPSHOT_ITERS = 10;
const int MAX_SNAPSHOTS = 5;
const int MAX_SNAPSHOT_RESETS = 4;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->curr_experiment != NULL) {
		switch (this->curr_experiment->type) {
		case EXPERIMENT_TYPE_EXPERIMENT:
			{
				Experiment* experiment = (Experiment*)this->curr_experiment;
				this->experiment_history = new ExperimentHistory(experiment);
			}
			break;
		}
	}

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
	this->experiment_context.push_back(NULL);
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
	vector<double> obs = this->problem->get_observations();
	this->scope_histories.back()->obs_history = obs;

	if (this->curr_experiment == NULL) {
		create_experiment(this->scope_histories[0],
						  this);

		delete this->scope_histories[0];
	} else {
		this->experiment_history->experiment->backprop(
		result,
		this);

		delete this->experiment_history;
		this->experiment_history = NULL;

		switch (this->curr_experiment->type) {
		case EXPERIMENT_TYPE_EXPERIMENT:
			if (this->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
				this->curr_experiment->clean();
				delete this->curr_experiment;

				this->curr_experiment = NULL;
			} else if (this->curr_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				this->curr_experiment->clean();

				Scope* last_updated_scope = this->curr_experiment->scope_context;

				this->curr_experiment->add(this);

				this->solution->curr_score = this->curr_experiment->calc_new_score();

				if (this->solution->existing_scope_histories.size() != 0) {
					for (int c_index = this->candidates.size()-1; c_index >= 0; c_index--) {
						if (this->candidates[c_index].second->is_fail()) {
							cout << "remove" << endl;
							this->candidates[c_index].second->print();

							delete this->candidates[c_index].second;
							this->candidates.erase(this->candidates.begin() + c_index);
						}
					}

					find_potential_tunnels(this->curr_experiment->scope_context,
										   this->solution->existing_scope_histories,
										   this->curr_experiment->new_scope_histories,
										   this->curr_experiment->branch_stack_traces,
										   this);
				}

				for (int h_index = 0; h_index < (int)this->solution->existing_scope_histories.size(); h_index++) {
					delete this->solution->existing_scope_histories[h_index];
				}
				this->solution->existing_scope_histories.clear();
				this->solution->existing_target_val_histories.clear();

				this->solution->existing_scope_histories = this->curr_experiment->new_scope_histories;
				this->curr_experiment->new_scope_histories.clear();
				this->solution->existing_target_val_histories = this->curr_experiment->new_target_val_histories;

				delete this->curr_experiment;
				this->curr_experiment = NULL;

				clean_scope(last_updated_scope);

				this->solution->clean_scopes();

				this->solution->timestamp++;
				// if (this->solution->timestamp >= RUN_TIMESTEPS) {
				// 	this->solution->timestamp = -1;
				// }

				if (this->solution->timestamp % SNAPSHOT_ITERS == 0) {
					if (this->solution->curr_score > this->solution_snapshots.back()->curr_score) {
						this->solution_snapshots.push_back(new Solution(this->solution));
						this->num_resets.push_back(0);
						if (this->solution_snapshots.size() > MAX_SNAPSHOTS) {
							delete this->solution_snapshots[0];
							this->solution_snapshots.erase(this->solution_snapshots.begin());

							this->num_resets.erase(this->num_resets.begin());
						}
					} else {
						this->num_resets.back()++;
						if (this->solution_snapshots.size() > 1
								&& this->num_resets.back() >= MAX_SNAPSHOT_RESETS) {
							delete this->solution_snapshots.back();
							this->solution_snapshots.erase(this->solution_snapshots.end()-1);

							this->num_resets.erase(this->num_resets.end()-1);
						}

						this->reset_count++;

						delete this->solution;
						this->solution = new Solution(this->solution_snapshots.back());
					}
				}
			}
			break;
		}
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
