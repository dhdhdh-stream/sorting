#include "solution_wrapper.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "new_scope_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

const int REGATHER_COUNTER_LIMIT = 20;
const int NEW_SCOPE_REGATHER_COUNTER_LIMIT = 800;

const int NEW_SCOPE_FOCUS_ITERS = 4;

void SolutionWrapper::experiment_init() {
	if ((int)this->solution->existing_scope_histories.size() < MEASURE_ITERS) {
		this->num_actions = 1;

		#if defined(MDEBUG) && MDEBUG
		this->run_index++;
		this->starting_run_seed = this->run_index;
		this->curr_run_seed = xorshift(this->starting_run_seed);
		#endif /* MDEBUG */

		ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
		this->scope_histories.push_back(scope_history);
		this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
	} else {
		if (is_new_scope_iter(this)) {
			while (this->curr_new_scope_experiment == NULL) {
				create_new_scope_overall_experiment(this);
			}
			while (this->curr_new_scope_experiment->curr_experiment == NULL) {
				create_new_scope_experiment(this);
			}
		} else {
			while (this->curr_branch_experiment == NULL) {
				create_branch_experiment(this);
			}
		}

		this->num_actions = 1;

		#if defined(MDEBUG) && MDEBUG
		this->run_index++;
		this->starting_run_seed = this->run_index;
		this->curr_run_seed = xorshift(this->starting_run_seed);
		#endif /* MDEBUG */

		if (is_new_scope_iter(this)) {
			this->experiment_history = new NewScopeExperimentHistory(
				this->curr_new_scope_experiment->curr_experiment);
		} else {
			this->experiment_history = new BranchExperimentHistory(this->curr_branch_experiment);
		}

		ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
		this->scope_histories.push_back(scope_history);
		this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
		this->experiment_context.push_back(NULL);
	}
}

tuple<bool,bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	bool fetch_action = false;
	
	if ((int)this->solution->existing_scope_histories.size() < MEASURE_ITERS) {
		while (!is_next) {
			if (this->node_context.back() == NULL) {
				if (this->scope_histories.size() == 1) {
					is_next = true;
					is_done = true;
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->exit_step(this);
				}
			} else {
				this->node_context.back()->step(obs,
												action,
												is_next,
												this);
			}
		}
	} else {
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
	}

	return tuple<bool,bool,int>{is_done, fetch_action, action};
}

void SolutionWrapper::set_action(int action) {
	AbstractExperiment* experiment = this->experiment_context.back()->experiment;
	experiment->set_action(action,
						   this);
}

void SolutionWrapper::experiment_end(double result) {
	if ((int)this->solution->existing_scope_histories.size() < MEASURE_ITERS) {
		while (true) {
			if (this->node_context.back() == NULL) {
				if (this->scope_histories.size() == 1) {
					break;
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->exit_step(this);
				}
			} else {
				this->node_context.back() = NULL;
			}
		}

		this->solution->existing_scope_histories.push_back(this->scope_histories[0]);
		this->solution->existing_target_val_histories.push_back(result);

		this->scope_histories.clear();
		this->node_context.clear();

		if ((int)this->solution->existing_scope_histories.size() == MEASURE_ITERS) {
			this->solution->measure_update();
		}
	} else {
		while (true) {
			if (this->node_context.back() == NULL
					&& this->experiment_context.back() == NULL) {
				if (this->scope_histories.size() == 1) {
					break;
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
				delete this->experiment_context.back();
				this->experiment_context.back() = NULL;
			} else {
				this->node_context.back() = NULL;
			}
		}

		this->experiment_history->experiment->backprop(
			result,
			this);

		delete this->experiment_history;
		this->experiment_history = NULL;

		this->scope_histories.clear();
		this->node_context.clear();
		this->experiment_context.clear();

		if (is_new_scope_iter(this)) {
			bool check_overall = false;
			bool early_exit = false;
			if (this->curr_new_scope_experiment->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
				this->curr_new_scope_experiment->generalize_iter++;

				this->curr_new_scope_experiment->curr_experiment->clean();
				delete this->curr_new_scope_experiment->curr_experiment;
				this->curr_new_scope_experiment->curr_experiment = NULL;

				check_overall = true;
			} else if (this->curr_new_scope_experiment->curr_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				this->curr_new_scope_experiment->generalize_iter++;

				for (int h_index = 0; h_index < (int)this->curr_new_scope_experiment->new_scope_histories.size(); h_index++) {
					delete this->curr_new_scope_experiment->new_scope_histories[h_index];
				}
				this->curr_new_scope_experiment->new_scope_histories = this->curr_new_scope_experiment->curr_experiment->new_scope_histories;
				this->curr_new_scope_experiment->curr_experiment->new_scope_histories.clear();
				this->curr_new_scope_experiment->new_target_val_histories = this->curr_new_scope_experiment->curr_experiment->new_target_val_histories;
				this->curr_new_scope_experiment->curr_experiment->new_target_val_histories.clear();

				this->curr_new_scope_experiment->successful_experiments.push_back(
					this->curr_new_scope_experiment->curr_experiment);
				this->curr_new_scope_experiment->curr_experiment = NULL;

				this->curr_new_scope_experiment->scope_context->new_scope_clean();
				for (int h_index = 0; h_index < (int)this->curr_new_scope_experiment->new_scope_histories.size(); h_index++) {
					new_scope_update_scores(this->curr_new_scope_experiment->new_scope_histories[h_index],
											this->curr_new_scope_experiment->new_target_val_histories[h_index],
											h_index,
											this->curr_new_scope_experiment->scope_context);
				}
				this->curr_new_scope_experiment->scope_context->new_scope_measure_update((int)this->curr_new_scope_experiment->new_scope_histories.size());

				check_overall = true;
				early_exit = !still_instances_possible(this->curr_new_scope_experiment);
			}

			if (check_overall) {
				if (this->curr_new_scope_experiment->generalize_iter > 0
						&& this->curr_new_scope_experiment->successful_experiments.size() == 0) {
					this->curr_new_scope_experiment->clean();

					delete this->curr_new_scope_experiment;
					this->curr_new_scope_experiment = NULL;

					this->regather_counter++;
					if (this->regather_counter >= NEW_SCOPE_REGATHER_COUNTER_LIMIT) {
						for (int h_index = 0; h_index < (int)this->solution->existing_scope_histories.size(); h_index++) {
							delete this->solution->existing_scope_histories[h_index];
						}
						this->solution->existing_scope_histories.clear();
						this->solution->existing_target_val_histories.clear();

						this->solution->clean();

						this->regather_counter = 0;
					}
				} else if (this->curr_new_scope_experiment->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES
						|| early_exit) {
					if (this->curr_new_scope_experiment->successful_experiments.size() >= NEW_SCOPE_MIN_NUM_LOCATIONS) {
						cout << "NewScopeOverallExperiment success" << endl;

						this->curr_new_scope_experiment->clean();

						if (this->best_new_scope_experiment == NULL) {
							this->best_new_scope_experiment = this->curr_new_scope_experiment;
						} else {
							double curr_impact = get_experiment_impact(this->curr_new_scope_experiment);
							double best_impact = get_experiment_impact(this->best_new_scope_experiment);
							if (curr_impact > best_impact) {
								delete this->best_new_scope_experiment;
								this->best_new_scope_experiment = this->curr_new_scope_experiment;
							} else {
								delete this->curr_new_scope_experiment;
							}
						}

						this->curr_new_scope_experiment = NULL;

						this->improvement_iter++;
						if (this->improvement_iter >= NEW_SCOPE_IMPROVEMENTS_PER_ITER) {
							Scope* last_updated_scope = this->best_new_scope_experiment->scope_context;

							this->best_new_scope_experiment->add(this);

							for (int h_index = 0; h_index < (int)this->solution->existing_scope_histories.size(); h_index++) {
								delete this->solution->existing_scope_histories[h_index];
							}
							this->solution->existing_scope_histories.clear();
							this->solution->existing_target_val_histories.clear();

							/**
							 * - don't use this->best_new_scope_experiment->new_scope_histories
							 *   - doesn't contain new scope histories
							 */

							delete this->best_new_scope_experiment;
							this->best_new_scope_experiment = NULL;

							clean_scope(last_updated_scope,
										this);

							this->solution->clean();

							this->solution->timestamp++;

							this->improvement_iter = 0;

							this->regather_counter = 0;
						}
					} else {
						this->curr_new_scope_experiment->clean();

						delete this->curr_new_scope_experiment;
						this->curr_new_scope_experiment = NULL;

						this->regather_counter++;
						if (this->regather_counter >= NEW_SCOPE_REGATHER_COUNTER_LIMIT) {
							for (int h_index = 0; h_index < (int)this->solution->existing_scope_histories.size(); h_index++) {
								delete this->solution->existing_scope_histories[h_index];
							}
							this->solution->existing_scope_histories.clear();
							this->solution->existing_target_val_histories.clear();

							this->solution->clean();

							this->regather_counter = 0;
						}
					}
				}
			}
		} else {
			if (this->curr_branch_experiment->result == EXPERIMENT_RESULT_FAIL) {
				this->regather_counter++;

				this->curr_branch_experiment->clean();
				delete this->curr_branch_experiment;

				this->curr_branch_experiment = NULL;

				if (this->regather_counter >= REGATHER_COUNTER_LIMIT) {
					for (int h_index = 0; h_index < (int)this->solution->existing_scope_histories.size(); h_index++) {
						delete this->solution->existing_scope_histories[h_index];
					}
					this->solution->existing_scope_histories.clear();
					this->solution->existing_target_val_histories.clear();

					this->solution->clean();

					this->regather_counter = 0;
				}
			} else if (this->curr_branch_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				this->curr_branch_experiment->clean();

				if (this->best_branch_experiment == NULL) {
					this->best_branch_experiment = this->curr_branch_experiment;
				} else {
					double curr_impact = get_experiment_impact(this->curr_branch_experiment);
					double best_impact = get_experiment_impact(this->best_branch_experiment);
					if (curr_impact > best_impact) {
						delete this->best_branch_experiment;
						this->best_branch_experiment = this->curr_branch_experiment;
					} else {
						delete this->curr_branch_experiment;
					}
				}

				this->curr_branch_experiment = NULL;

				this->improvement_iter++;
				if (this->improvement_iter >= IMPROVEMENTS_PER_ITER) {
					if (this->solution->last_new_scope != NULL) {
						this->solution->new_scope_iters++;
						if (this->solution->new_scope_iters >= NEW_SCOPE_FOCUS_ITERS) {
							this->solution->last_new_scope = NULL;
						}
					}

					Scope* last_updated_scope = this->best_branch_experiment->scope_context;

					this->best_branch_experiment->add(this);

					for (int h_index = 0; h_index < (int)this->solution->existing_scope_histories.size(); h_index++) {
						delete this->solution->existing_scope_histories[h_index];
					}
					this->solution->existing_scope_histories.clear();
					this->solution->existing_target_val_histories.clear();

					this->solution->existing_scope_histories = this->best_branch_experiment->new_scope_histories;
					this->best_branch_experiment->new_scope_histories.clear();
					this->solution->existing_target_val_histories = this->best_branch_experiment->new_target_val_histories;

					delete this->best_branch_experiment;
					this->best_branch_experiment = NULL;

					clean_scope(last_updated_scope,
								this);

					this->solution->clean();

					this->solution->timestamp++;

					this->solution->measure_update();

					this->improvement_iter = 0;

					this->regather_counter = 0;
				}
			}
		}
	}
}
