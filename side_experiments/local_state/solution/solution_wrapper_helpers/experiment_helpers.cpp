#include "solution_wrapper.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "init_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::experiment_init(vector<double> obs) {
	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->iters_since_update < UPDATE_NUM_ITERS) {
		// uniform_int_distribution<int> type_distribution(0, 1);
		// this->run_type = type_distribution(generator);

		this->run_type = RUN_TYPE_EXISTING;
	} else {
		this->run_type = RUN_TYPE_EXPLORE;
	}

	this->run_num_actions = 0;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);
	this->experiment_context.push_back(NULL);

	this->states.push_back(Eigen::VectorXf());
	this->states.back().resize(this->solution->starting_scope->num_states);
	this->states.back().setConstant(0.0);

	this->solution->starting_scope->experiment_start_activate(
		obs,
		this);
}

tuple<bool,bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
	if (this->experiment_context.back() != NULL) {
		AbstractExperiment* experiment = this->experiment_context.back()->experiment;
		experiment->experiment_step_callback(obs,
											 this);
	} else {
		if (this->node_context.back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->node_context.back();
			action_node->experiment_step_callback(obs,
												  this);
		}
	}

	int action;
	bool is_next = false;
	bool is_done = false;
	bool fetch_action = false;

	if (this->run_type == RUN_TYPE_DAMAGE) {
		uniform_int_distribution<int> damage_distribution(0, 19);
		if (damage_distribution(generator) == 0) {
			Scope* scope = this->scope_histories.back()->scope;

			uniform_int_distribution<int> distribution(0, scope->generic_action_nodes.size()-1);
			scope->generic_action_nodes[distribution(generator)]->experiment_step(
				obs,
				action,
				is_next,
				this);
		}
	}

	while (!is_next) {
		if (this->node_context.back() == NULL
				&& this->experiment_context.back() == NULL) {
			if (this->scope_histories.back()->experiment_callback_histories.size() > 0) {
				Scope* scope = this->scope_histories.back()->scope;
				scope->end_score_network->activate(this->states.back());
				double signal = scope->end_score_network->output->acti_vals(0);
				for (int c_index = 0; c_index < (int)this->scope_histories.back()->experiment_callback_histories.size(); c_index++) {
					ExploreExperimentHistory* explore_experiment_history = (ExploreExperimentHistory*)this->scope_histories.back()->experiment_callback_histories[c_index];
					int index = this->scope_histories.back()->experiment_callback_indexes[c_index];
					explore_experiment_history->signal_histories[index] = signal;
				}
			}

			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				if (this->experiment_context[this->experiment_context.size() - 2] != NULL) {
					AbstractExperiment* experiment = this->experiment_context[this->experiment_context.size() - 2]->experiment;
					experiment->experiment_exit_step(this);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->experiment_exit_step(obs,
													 this);
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
	if (this->run_type == RUN_TYPE_EXISTING) {
		update_helper(result,
					  this);
	}

	if (this->run_type == RUN_TYPE_EXPLORE) {
		if (this->explore_experiment_histories.size() == 0) {
			create_experiment(this->scope_histories[0],
							  this);
		} else if (this->explore_experiment_histories.size() >= 2) {
			ExploreExperiment* keep_experiment = NULL;
			for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_experiment_histories.begin();
					it != this->explore_experiment_histories.end(); it++) {
				if (keep_experiment == NULL) {
					keep_experiment = it->first;
				} else {
					if (it->first->further_than(keep_experiment)) {
						delete keep_experiment;

						keep_experiment = it->first;
					} else {
						delete it->first;
					}
				}
			}
		}
	}

	if (this->run_type != RUN_TYPE_EXPLORE) {
		if (this->train_scope_histories.size() < HISTORIES_NUM_SAVE) {
			this->train_scope_histories.push_back(this->scope_histories[0]);
			this->train_target_val_histories.push_back(result);
		} else {
			delete this->train_scope_histories[this->train_histories_index];
			this->train_scope_histories[this->train_histories_index] = this->scope_histories[0];
			this->train_target_val_histories[this->train_histories_index] = result;
		}
		this->train_histories_index++;
		if (this->train_histories_index >= HISTORIES_NUM_SAVE) {
			this->train_histories_index = 0;
		}

		train_helper(this);
	} else {
		delete this->scope_histories[0];
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	this->states.clear();

	if (this->run_type == RUN_TYPE_EXPLORE) {
		if (this->explore_experiment_histories.size() == 1) {
			for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_experiment_histories.begin();
					it != this->explore_experiment_histories.end(); it++) {
				it->first->backprop(result,
									it->second,
									this);
			}
		}

		for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_experiment_histories.begin();
				it != this->explore_experiment_histories.end(); it++) {
			delete it->second;
		}
		this->explore_experiment_histories.clear();
	}

	this->iters_since_update++;
	if (this->iters_since_update == UPDATE_NUM_ITERS) {
		for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
			Scope* scope = this->solution->scopes[s_index];
			for (int n_index = 0; n_index < (int)scope->start_init_networks.size(); n_index++) {
				scope->start_init_networks[n_index]->is_ramp = false;
			}
			for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
					it != scope->nodes.end(); it++) {
				switch (it->second->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)it->second;

						for (int n_index = 0; n_index < (int)action_node->init_networks.size(); n_index++) {
							action_node->init_networks[n_index]->is_ramp = false;
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)it->second;

						branch_node->original_network->is_ramp = false;
						branch_node->branch_network->is_ramp = false;

						branch_node->is_ramp = false;
					}
					break;
				}
			}
		}

		if (this->solution->timestamp != 0) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->prev_solution->curr_score > this->solution->curr_score) {
			#endif /* MDEBUG */
				// temp
				cout << "reset" << endl;
				cout << "this->prev_solution->curr_num_resets: " << this->prev_solution->curr_num_resets << endl;
				cout << "this->prev_solution->curr_score: " << this->prev_solution->curr_score << endl;
				cout << "this->solution->curr_score: " << this->solution->curr_score << endl;

				this->prev_solution->curr_num_resets++;
				if (this->prev_solution->curr_num_resets >= STUCK_NUM_ITERS) {
					this->prev_solution->timestamp = -1;
				}

				delete this->solution;
				this->solution = new Solution(this->prev_solution);
			}
		}
	}
}
