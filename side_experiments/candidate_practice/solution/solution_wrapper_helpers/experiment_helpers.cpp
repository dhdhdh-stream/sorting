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

#if defined(MDEBUG) && MDEBUG
const int ITERS_PER_TUNNEL = 2;
const int TUNNEL_NUM_CANDIDATES = 2;
#else
const int ITERS_PER_TUNNEL = 10;
const int TUNNEL_NUM_CANDIDATES = 10;
#endif /* MDEBUG */

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

	ScopeHistory* scope_history = new ScopeHistory(this->curr_solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->curr_solution->scopes[0]->nodes[0]);
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

				this->curr_solution->curr_score = this->best_experiment->calc_new_score();

				delete this->best_experiment;
				this->best_experiment = NULL;

				clean_scope(last_updated_scope);

				this->curr_solution->clean_scopes();

				this->curr_solution->timestamp++;
				if (this->curr_solution->timestamp >= RUN_TIMESTEPS) {
					this->curr_solution->timestamp = -1;
				}

				if (this->curr_tunnel != NULL) {
					this->curr_tunnel->ending_true = this->curr_solution->curr_score;
				}
				bool is_next_tunnel;
				if (this->prev_solution == NULL) {
					if (this->curr_solution->timestamp >= ITERS_PER_TUNNEL) {
						is_next_tunnel = true;
					} else {
						is_next_tunnel = false;
					}
				} else {
					if (this->curr_solution->timestamp >= this->prev_solution->timestamp + ITERS_PER_TUNNEL) {
						is_next_tunnel = true;
					} else {
						is_next_tunnel = false;
					}
				}
				if (is_next_tunnel) {
					if (this->prev_solution == NULL) {
						this->prev_solution = new Solution(this->curr_solution);
					} else {
						if (this->best_solution == NULL) {
							this->best_solution = this->curr_solution;
						} else {
							if (this->curr_solution->curr_score > this->best_solution->curr_score) {
								delete this->best_solution;
								this->best_solution = this->curr_solution;
							} else {
								delete this->curr_solution;
							}
							this->curr_solution = NULL;
						}

						this->tunnel_iter++;
						if (this->tunnel_iter >= TUNNEL_NUM_CANDIDATES) {
							this->prev_solution = this->best_solution;
							this->best_solution = NULL;

							this->tunnel_iter = 0;
						}

						this->curr_solution = new Solution(this->prev_solution);
					}

					if (this->curr_tunnel != NULL) {
						delete this->curr_tunnel;
					}
					this->curr_tunnel = create_obs_candidate(
						this->prev_solution->obs_histories,
						this->prev_solution->target_val_histories,
						this);

					this->tunnel_iter = 0;
				}

				this->improvement_iter = 0;
			}
		}
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
