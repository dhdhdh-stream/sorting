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
const int NEW_SCOPE_REGATHER_COUNTER_LIMIT = 400;

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
		while (this->curr_experiment == NULL) {
			create_experiment(this,
							  this->curr_experiment);
		}

		this->num_actions = 1;

		#if defined(MDEBUG) && MDEBUG
		this->run_index++;
		this->starting_run_seed = this->run_index;
		this->curr_run_seed = xorshift(this->starting_run_seed);
		#endif /* MDEBUG */

		switch (this->curr_experiment->type) {
		case EXPERIMENT_TYPE_BRANCH:
			{
				BranchExperiment* branch_experiment = (BranchExperiment*)this->curr_experiment;
				this->experiment_history = new BranchExperimentHistory(branch_experiment);
			}
			break;
		case EXPERIMENT_TYPE_NEW_SCOPE:
			{
				NewScopeExperiment* new_scope_experiment = (NewScopeExperiment*)this->curr_experiment;
				this->experiment_history = new NewScopeExperimentHistory(new_scope_experiment);
			}
			break;
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

		if (this->curr_experiment != NULL) {
			if (this->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
				this->regather_counter++;

				this->curr_experiment->clean();
				delete this->curr_experiment;

				this->curr_experiment = NULL;

				bool is_regather = false;
				if (this->solution->timestamp % 10 == 5) {
					if (this->regather_counter >= NEW_SCOPE_REGATHER_COUNTER_LIMIT) {
						is_regather = true;
					}
				} else {
					if (this->regather_counter >= REGATHER_COUNTER_LIMIT) {
						is_regather = true;
					}
				}
				if (is_regather) {
					for (int h_index = 0; h_index < (int)this->solution->existing_scope_histories.size(); h_index++) {
						delete this->solution->existing_scope_histories[h_index];
					}
					this->solution->existing_scope_histories.clear();
					this->solution->existing_target_val_histories.clear();

					this->solution->clean();

					this->regather_counter = 0;
				}
			} else if (this->curr_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				this->curr_experiment->clean();

				if (this->best_experiment == NULL) {
					this->best_experiment = this->curr_experiment;
				} else {
					double curr_impact = get_experiment_impact(this->curr_experiment);
					double best_impact = get_experiment_impact(this->best_experiment);
					if (curr_impact > best_impact) {
						delete this->best_experiment;
						this->best_experiment = this->curr_experiment;
					} else {
						delete this->curr_experiment;
					}
				}

				this->curr_experiment = NULL;

				improvement_iter++;
				bool is_next = false;
				if (this->solution->timestamp % 10 == 5) {
					if (improvement_iter >= NEW_SCOPE_IMPROVEMENTS_PER_ITER) {
						is_next = true;
					}
				} else {
					if (improvement_iter >= IMPROVEMENTS_PER_ITER) {
						is_next = true;
					}
				}
				if (is_next) {
					if (this->solution->last_new_scope != NULL) {
						this->solution->new_scope_iters++;
						if (this->solution->new_scope_iters >= NEW_SCOPE_FOCUS_ITERS) {
							this->solution->last_new_scope = NULL;
						}
					}

					Scope* last_updated_scope = this->best_experiment->scope_context;

					this->best_experiment->add(this);

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
