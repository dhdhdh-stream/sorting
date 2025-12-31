#include "solution_wrapper.h"

#include <iostream>

#include "candidate_experiment.h"
#include "chase_experiment.h"
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

#if defined(MDEBUG) && MDEBUG
const int ITERS_PER_TUNNEL = 2;
const int TUNNEL_NUM_CANDIDATES = 2;
#else
const int ITERS_PER_TUNNEL = 10;
const int TUNNEL_NUM_CANDIDATES = 10;
#endif /* MDEBUG */

const int TUNNEL_EARLY_FAIL_MIN_TRIES = 20;
const double TUNNEL_EARLY_FAIL_MIN_RATIO = 0.7;

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
		case EXPERIMENT_TYPE_CANDIDATE:
			{
				CandidateExperiment* candidate_experiment = (CandidateExperiment*)this->curr_experiment;
				this->experiment_history = new CandidateExperimentHistory(candidate_experiment);
			}
			break;
		case EXPERIMENT_TYPE_CHASE:
			{
				ChaseExperiment* chase_experiment = (ChaseExperiment*)this->curr_experiment;
				this->experiment_history = new ChaseExperimentHistory(chase_experiment);
			}
			break;
		}
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

			bool is_next = false;
			switch (this->curr_experiment->type) {
			case EXPERIMENT_TYPE_EXPERIMENT:
				if (this->best_experiment == NULL) {
					this->best_experiment = this->curr_experiment;
				} else {
					Experiment* curr = (Experiment*)this->curr_experiment;
					Experiment* best = (Experiment*)this->best_experiment;
					if (curr->improvement > best->improvement) {
						delete this->best_experiment;
						this->best_experiment = this->curr_experiment;
					} else {
						delete this->curr_experiment;
					}
				}

				this->improvement_iter++;
				if (this->improvement_iter >= IMPROVEMENTS_PER_ITER) {
					is_next = true;
				}

				break;
			case EXPERIMENT_TYPE_CANDIDATE:
				this->best_experiment = this->curr_experiment;
				is_next = true;
				break;
			case EXPERIMENT_TYPE_CHASE:
				if (this->best_experiment == NULL) {
					this->best_experiment = this->curr_experiment;
				} else {
					ChaseExperiment* curr = (ChaseExperiment*)this->curr_experiment;
					ChaseExperiment* best = (ChaseExperiment*)this->best_experiment;
					if (curr->improvement > best->improvement) {
						delete this->best_experiment;
						this->best_experiment = this->curr_experiment;
					} else {
						delete this->curr_experiment;
					}
				}

				this->improvement_iter++;
				if (this->improvement_iter >= IMPROVEMENTS_PER_ITER) {
					is_next = true;
				}

				break;
			}

			this->curr_experiment = NULL;

			if (is_next) {
				if (this->solution == this->curr_solution) {
					/**
					 * - 1st iter
					 */
					this->solution = new Solution(this->curr_solution);
					this->potential_solution = this->curr_solution;
				}

				Scope* last_updated_scope = this->best_experiment->scope_context;

				this->best_experiment->add(this);

				this->curr_solution->curr_score = this->best_experiment->calc_new_score();

				for (int h_index = 0; h_index < (int)this->curr_solution->existing_scope_histories.size(); h_index++) {
					delete this->curr_solution->existing_scope_histories[h_index];
				}
				this->curr_solution->existing_scope_histories.clear();
				this->curr_solution->existing_target_val_histories.clear();

				this->curr_solution->existing_scope_histories = this->best_experiment->new_scope_histories;
				this->best_experiment->new_scope_histories.clear();
				this->curr_solution->existing_target_val_histories = this->best_experiment->new_target_val_histories;

				delete this->best_experiment;
				this->best_experiment = NULL;

				clean_scope(last_updated_scope);

				this->curr_solution->clean_scopes();

				this->curr_solution->timestamp++;
				if (this->curr_solution->timestamp >= RUN_TIMESTEPS) {
					this->curr_solution->timestamp = -1;
				}

				if (this->potential_solution->timestamp >= this->solution->timestamp + ITERS_PER_TUNNEL) {
					update_tunnel_try_history(this);

					if (this->best_solution == NULL) {
						this->best_solution = this->potential_solution;
					} else {
						if (this->potential_solution->curr_score > this->best_solution->curr_score) {
							delete this->best_solution;
							this->best_solution = this->potential_solution;
						} else {
							delete this->potential_solution;
						}
					}
					this->potential_solution = NULL;

					this->tunnel_iter++;
					if (this->tunnel_iter >= TUNNEL_NUM_CANDIDATES) {
						for (int h_index = 0; h_index < (int)this->best_solution->existing_scope_histories.size(); h_index++) {
							measure_tunnel_vals_helper(this->best_solution->existing_scope_histories[h_index]);
						}
						for (int s_index = 0; s_index < (int)this->best_solution->scopes.size(); s_index++) {
							for (int t_index = 0; t_index < (int)this->best_solution->scopes[s_index]->tunnels.size(); t_index++) {
								Tunnel* tunnel = this->best_solution->scopes[s_index]->tunnels[t_index];
								tunnel->update_vals((int)this->best_solution->existing_scope_histories.size());
							}
						}

						delete this->solution;
						this->solution = this->best_solution;
						this->best_solution = NULL;

						this->curr_tunnel_parent = NULL;

						this->tunnel_iter = 0;
					} else {
						this->curr_solution = this->solution;

						set_tunnel(this);
					}
				}

				this->improvement_iter = 0;
			}
		}
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
