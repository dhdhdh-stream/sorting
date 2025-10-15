#include "solution_wrapper.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "eval_experiment.h"
#include "explore_experiment.h"
#include "globals.h"
#include "helpers.h"
#include "refine_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "signal_experiment.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

const int MIN_EXPLORE_PER_RUN = 10;
const double MAX_EXPLORE_RATIO_PER_RUN = 0.2;

void SolutionWrapper::experiment_init() {
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
														  fetch_action,
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

	int num_explore = (int)this->explore_histories.size();
	for (int e_index = 0; e_index < (int)this->signal_experiment_histories.size(); e_index++) {
		switch (this->signal_experiment_histories[e_index].first->state) {
		case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
		case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
		case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
		case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
			num_explore++;
			break;
		}
	}
	if (this->solution->existing_scope_histories.size() >= HISTORIES_NUM_SAVE
			&& num_explore < MIN_EXPLORE_PER_RUN
			&& num_explore < MAX_EXPLORE_RATIO_PER_RUN * (double)this->num_actions) {
		create_experiment(this);
	}

	if (this->should_explore) {
		delete this->scope_histories[0];
	} else {
		if (this->solution->existing_scope_histories.size() >= HISTORIES_NUM_SAVE) {
			delete this->solution->existing_scope_histories.front().first;
			this->solution->sum_scores -= this->solution->existing_scope_histories.front().second;
			this->solution->existing_scope_histories.pop_front();
		}
		this->solution->existing_scope_histories.push_back({this->scope_histories[0], result});
		this->solution->sum_scores += result;
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	for (int e_index = 0; e_index < (int)this->explore_order_seen.size(); e_index++) {
		switch (this->explore_order_seen[e_index]->type) {
		case EXPERIMENT_TYPE_EXPLORE:
			{
				ExploreExperiment* experiment = (ExploreExperiment*)this->explore_order_seen[e_index];
				int num_following = (int)this->explore_order_seen.size() - 1 - e_index;
				if (experiment->last_num_following_explores.size() >= LAST_NUM_TRACK) {
					experiment->sum_num_following_explores -= experiment->last_num_following_explores.front();
					experiment->last_num_following_explores.pop_front();
				}
				experiment->last_num_following_explores.push_back(num_following);
				experiment->sum_num_following_explores += num_following;
			}
			break;
		case EXPERIMENT_TYPE_SIGNAL:
			{
				SignalExperiment* experiment = (SignalExperiment*)this->explore_order_seen[e_index];
				int num_following = (int)this->explore_order_seen.size() - 1 - e_index;
				if (experiment->last_num_following_explores.size() >= LAST_NUM_TRACK) {
					experiment->sum_num_following_explores -= experiment->last_num_following_explores.front();
					experiment->last_num_following_explores.pop_front();
				}
				experiment->last_num_following_explores.push_back(num_following);
				experiment->sum_num_following_explores += num_following;
			}
			break;
		}
	}
	this->explore_order_seen.clear();
	for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_histories.begin();
			it != this->explore_histories.end(); it++) {
		it->first->backprop(result,
							it->second,
							this);
		delete it->second;
	}
	this->explore_histories.clear();

	for (map<RefineExperiment*, RefineExperimentHistory*>::iterator it = this->refine_histories.begin();
			it != this->refine_histories.end(); it++) {
		it->first->backprop(result,
							it->second,
							this);
		delete it->second;
	}
	this->refine_histories.clear();

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

	for (int e_index = 0; e_index < (int)this->signal_experiment_histories.size(); e_index++) {
		this->signal_experiment_histories[e_index].first->scope_context->signal_experiment_history = NULL;

		this->signal_experiment_histories[e_index].first->backprop(
			result,
			this->signal_experiment_histories[e_index].second,
			this);

		delete this->signal_experiment_histories[e_index].second;
	}
	this->signal_experiment_histories.clear();

	for (set<Scope*>::iterator it = updated_scopes.begin();
			it != updated_scopes.end(); it++) {
		clean_scope(*it,
					this);
	}
}
