#include "solution_wrapper.h"

#include <iostream>

#include "constants.h"
#include "experiment.h"
#include "explore_experiment.h"
#include "explore_instance.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

const int RUN_TIMESTEPS = 100;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

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

	this->experiment_history->experiment->backprop(
		result,
		this);

	delete this->experiment_history;
	this->experiment_history = NULL;

	switch (this->state) {
	case STATE_EXPLORE:
		if (this->curr_explore_experiment->result == EXPERIMENT_RESULT_FAIL) {
			this->curr_explore_experiment->clean();
			this->explore_experiments.push_back(this->curr_explore_experiment);

			this->curr_explore_experiment = NULL;

			this->state_iter++;
			if (this->state_iter >= NUM_EXPLORE_EXPERIMENTS) {
				for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
					this->solution->scopes[s_index]->update_signals();
				}

				for (int s_index = 0; s_index < (int)this->best_explore_instances.size(); s_index++) {
					double sum_val = 0.0;
					int count = 0;
					calc_consistency_helper(this->best_explore_instances[s_index]->scope_history,
											sum_val,
											count);

					if (count == 0) {
						this->best_explore_instances[s_index]->consistency = 0.0;
					} else {
						this->best_explore_instances[s_index]->consistency = sum_val / count;
					}

					// cout << s_index << endl;
					// cout << "consistency: " << this->best_explore_instances[s_index]->consistency << endl;

					if ((this->consistent_explore_instances.back() == NULL
							|| this->consistent_explore_instances.back()->consistency < this->best_explore_instances[s_index]->consistency)) {
						if (this->consistent_explore_instances.back() != NULL) {
							delete this->consistent_explore_instances.back();
						}
						this->consistent_explore_instances.back() = this->best_explore_instances[s_index];

						int index = this->consistent_explore_instances.size()-1;
						while (true) {
							if (this->consistent_explore_instances[index-1] == NULL
									|| this->consistent_explore_instances[index-1]->consistency < this->consistent_explore_instances[index]->consistency) {
								ExploreInstance* temp = this->consistent_explore_instances[index];
								this->consistent_explore_instances[index] = this->consistent_explore_instances[index-1];
								this->consistent_explore_instances[index-1] = temp;
							} else {
								break;
							}

							index--;
							if (index == 0) {
								break;
							}
						}
					} else {
						delete this->best_explore_instances[s_index];
					}

					this->best_explore_instances[s_index] = NULL;
				}

				// for (int s_index = 0; s_index < (int)this->best_explore_instances.size(); s_index++) {
				// 	if (s_index < NUM_EXPERIMENTS) {
				// 		this->consistent_explore_instances[s_index] = this->best_explore_instances[s_index];
				// 	} else {
				// 		delete this->best_explore_instances[s_index];
				// 	}

				// 	this->best_explore_instances[s_index] = NULL;
				// }

				// temp
				for (int e_index = 0; e_index < (int)this->consistent_explore_instances.size(); e_index++) {
					cout << e_index << endl;
					this->consistent_explore_instances[e_index]->print();
					cout << "consistency: " << this->consistent_explore_instances[e_index]->consistency << endl;
				}

				this->state = STATE_EXPERIMENT;
				this->state_iter = 0;

				this->curr_experiment = new Experiment(this->consistent_explore_instances[this->state_iter]);
				delete this->consistent_explore_instances[this->state_iter];
				this->consistent_explore_instances[this->state_iter] = NULL;
			}
		}
		break;
	case STATE_EXPERIMENT:
		delete this->scope_histories[0];

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
		}

		if (this->curr_experiment == NULL) {
			this->state_iter++;
			if (this->state_iter >= NUM_EXPERIMENTS) {
				for (int e_index = 0; e_index < (int)this->explore_experiments.size(); e_index++) {
					delete this->explore_experiments[e_index];
				}
				this->explore_experiments.clear();

				if (this->best_experiment != NULL) {
					Scope* last_updated_scope = this->best_experiment->scope_context;

					this->best_experiment->add(this);

					this->solution->curr_score = this->best_experiment->calc_new_score();

					delete this->best_experiment;
					this->best_experiment = NULL;

					clean_scope(last_updated_scope);

					this->solution->clean_scopes();

					this->solution->timestamp++;
					if (this->solution->timestamp >= RUN_TIMESTEPS) {
						this->solution->timestamp = -1;
					}
				}

				this->state = STATE_EXPLORE;
				this->state_iter = 0;
			} else {
				this->curr_experiment = new Experiment(this->consistent_explore_instances[this->state_iter]);
				delete this->consistent_explore_instances[this->state_iter];
				this->consistent_explore_instances[this->state_iter] = NULL;
			}
		}

		break;
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
