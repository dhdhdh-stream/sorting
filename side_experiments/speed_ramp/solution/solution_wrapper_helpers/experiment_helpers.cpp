#include "solution_wrapper.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "eval_experiment.h"
#include "explore_experiment.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "signal_eval_experiment.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int SIGNAL_IMPROVEMENTS_PER_ITER = 2;
#else
const int SIGNAL_IMPROVEMENTS_PER_ITER = 10;
#endif /* MDEBUG */

const int MIN_EXPLORE_PER_RUN = 10;

void SolutionWrapper::experiment_init() {
	// temp
	if (this->solution->scopes[0]->curr_signal_eval_experiment == NULL) {
		SignalEvalExperiment* new_signal_eval_experiment = new SignalEvalExperiment();

		new_signal_eval_experiment->scope_context = this->solution->scopes[0];

		vector<int> post_actions{0, 0, 0, 0, 1, 1, 1, 1};
		new_signal_eval_experiment->post_actions = post_actions;

		for (int s_index = 0; s_index < (int)this->solution->scopes[0]->signals.size(); s_index++) {
			new_signal_eval_experiment->previous_signals.push_back(
				new Signal(this->solution->scopes[0]->signals[s_index]));
		}

		this->solution->scopes[0]->curr_signal_eval_experiment = new_signal_eval_experiment;
	}

	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	uniform_int_distribution<int> explore_distribution(0, 4);
	if (explore_distribution(generator) == 0) {
		this->should_explore = true;
	} else {
		this->should_explore = false;
	}
	this->curr_explore = NULL;
	this->has_explore = false;

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
		bool is_signal = experiment_check_signal_activate(obs,
														  action,
														  is_next,
														  this);
		if (!is_signal) {
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

	if (this->explore_histories.size() < MIN_EXPLORE_PER_RUN) {
		create_experiment(this);
	}

	delete this->scope_histories[0];
	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	for (int e_index = 0; e_index < (int)this->explore_order_seen.size(); e_index++) {
		ExploreExperiment* experiment = this->explore_order_seen[e_index];
		int num_following = (int)this->explore_order_seen.size() - 1 - e_index;
		if (experiment->last_num_following_explores.size() >= LAST_NUM_TRACK) {
			experiment->sum_num_following_explores -= experiment->last_num_following_explores.front();
			experiment->last_num_following_explores.pop_front();
		}
		experiment->last_num_following_explores.push_back(num_following);
		experiment->sum_num_following_explores += num_following;
	}
	this->explore_order_seen.clear();
	for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_histories.begin();
			it != this->explore_histories.end(); it++) {
		it->first->backprop(result,
							it->second);
		delete it->second;
	}
	this->explore_histories.clear();

	set<Scope*> updated_scopes;
	for (map<EvalExperiment*, EvalExperimentHistory*>::iterator it = this->eval_histories.begin();
			it != this->eval_histories.end(); it++) {
		it->first->backprop(result,
							it->second,
							this,
							updated_scopes);
		delete it->second;
	}
	this->eval_histories.clear();

	for (map<SignalEvalExperiment*, SignalEvalExperimentHistory*>::iterator it = this->signal_eval_histories.begin();
			it != this->signal_eval_histories.end(); it++) {
		it->first->backprop(result,
							it->second,
							this);
		delete it->second;

		if (it->first->state == SIGNAL_EVAL_EXPERIMENT_STATE_DONE) {
			Scope* scope = it->first->scope_context;
			if (scope->best_signal_eval_experiment == NULL) {
				scope->best_signal_eval_experiment = it->first;
			} else {
				if (it->first->misguess_average < scope->best_signal_eval_experiment->misguess_average) {
					delete scope->best_signal_eval_experiment;
					scope->best_signal_eval_experiment = it->first;
				} else {
					delete it->first;
				}
			}
			scope->curr_signal_eval_experiment = NULL;

			scope->experiment_iter++;
			if (scope->experiment_iter >= SIGNAL_IMPROVEMENTS_PER_ITER) {
				scope->best_signal_eval_experiment->add(this);

				delete scope->best_signal_eval_experiment;
				scope->best_signal_eval_experiment = NULL;

				scope->experiment_iter = 0;
			}
		}
	}
	this->signal_eval_histories.clear();

	for (set<Scope*>::iterator it = updated_scopes.begin();
			it != updated_scopes.end(); it++) {
		clean_scope(*it,
					this);
	}
}
