#include "solution_wrapper.h"

#include <iostream>

#include "chase_experiment.h"
#include "constants.h"
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

const int ITERS_PER_TUNNEL = 10;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->curr_experiment != NULL) {
		this->experiment_history = new ChaseExperimentHistory(this->curr_experiment);
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

		if (this->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
			this->curr_experiment->clean();
			delete this->curr_experiment;

			this->curr_experiment = NULL;
		} else if (this->curr_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
			this->curr_experiment->clean();

			if (this->best_experiment == NULL) {
				this->best_experiment = this->curr_experiment;
			} else {
				if (this->curr_experiment->improvement > this->best_experiment->improvement) {
					delete this->best_experiment;
					this->best_experiment = this->curr_experiment;
				} else {
					delete this->curr_experiment;
				}
			}

			this->curr_experiment = NULL;

			this->improvement_iter++;
			if (this->improvement_iter >= IMPROVEMENTS_PER_ITER) {
				Scope* last_updated_scope = this->best_experiment->scope_context;

				this->best_experiment->add(this);

				this->solution->curr_score = this->best_experiment->calc_new_score();

				for (int h_index = 0; h_index < (int)this->solution->existing_scope_histories.size(); h_index++) {
					delete this->solution->existing_scope_histories[h_index];
				}
				this->solution->existing_scope_histories.clear();
				this->solution->existing_target_val_histories.clear();

				this->solution->existing_scope_histories = this->best_experiment->new_scope_histories;
				this->best_experiment->new_scope_histories.clear();
				this->solution->existing_target_val_histories = this->best_experiment->new_target_val_histories;

				delete this->best_experiment;
				this->best_experiment = NULL;

				clean_scope(last_updated_scope);

				this->solution->clean_scopes();

				if (this->solution->curr_tunnel != NULL) {
					this->solution->curr_tunnel->ending_true = this->solution->curr_score;
				}
				this->solution->tunnel_iter++;
				// if (this->solution->tunnel_iter >= ITERS_PER_TUNNEL) {
				if (this->solution->tunnel_iter >= ITERS_PER_TUNNEL
						|| this->solution->curr_tunnel == NULL) {
					if (this->solution->curr_tunnel != NULL) {
						this->solution->tunnel_history.push_back(this->solution->curr_tunnel);
					}
					this->solution->curr_tunnel = create_obs_candidate(
						this->solution->obs_histories,
						this->solution->target_val_histories,
						this);

					this->solution->tunnel_iter = 0;
				}

				this->solution->timestamp++;
				if (this->solution->timestamp >= RUN_TIMESTEPS) {
					this->solution->timestamp = -1;
				}

				this->improvement_iter = 0;
			}
		}
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
