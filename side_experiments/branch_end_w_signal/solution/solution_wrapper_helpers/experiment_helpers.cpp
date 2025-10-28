#include "solution_wrapper.h"

#include <iostream>

#include "branch_end_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

// const int RUN_TIMESTEPS = 30;
const int RUN_TIMESTEPS = 100;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->curr_branch_experiment != NULL) {
		this->experiment_history = new BranchExperimentHistory(this->curr_branch_experiment);
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
	if (this->curr_branch_experiment == NULL) {
		create_branch_experiment(this->scope_histories[0],
								 this);
	} else {
		set<BranchEndNode*> updated_nodes;
		for (int h_index = 0; h_index < (int)this->branch_end_node_callback_histories.size(); h_index++) {
			BranchEndNode* branch_end_node = (BranchEndNode*)this->branch_end_node_callback_histories[h_index]->node;
			branch_end_node->update(result,
									this->branch_end_node_callback_histories[h_index]);
			updated_nodes.insert(branch_end_node);
		}
		this->branch_end_node_callbacks.clear();
		this->branch_end_node_callback_histories.clear();

		this->experiment_history->experiment->backprop(
			result,
			this);

		delete this->experiment_history;
		this->experiment_history = NULL;

		this->experiment_callbacks.clear();

		if (this->curr_branch_experiment->result == EXPERIMENT_RESULT_FAIL) {
			for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
				for (map<int, AbstractNode*>::iterator it = this->solution->scopes[s_index]->nodes.begin();
						it != this->solution->scopes[s_index]->nodes.end(); it++) {
					if (it->second->type == NODE_TYPE_BRANCH_END) {
						BranchEndNode* branch_end_node = (BranchEndNode*)it->second;
						branch_end_node->backprop();
					}
				}
			}

			this->curr_branch_experiment->clean();
			delete this->curr_branch_experiment;

			this->curr_branch_experiment = NULL;
		} else if (this->curr_branch_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
			for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
				for (map<int, AbstractNode*>::iterator it = this->solution->scopes[s_index]->nodes.begin();
						it != this->solution->scopes[s_index]->nodes.end(); it++) {
					if (it->second->type == NODE_TYPE_BRANCH_END) {
						BranchEndNode* branch_end_node = (BranchEndNode*)it->second;
						branch_end_node->backprop();
					}
				}
			}

			this->curr_branch_experiment->clean();

			if (this->best_branch_experiment == NULL) {
				this->best_branch_experiment = this->curr_branch_experiment;
			} else {
				double curr_impact = this->curr_branch_experiment->improvement;
				double best_impact = this->best_branch_experiment->improvement;
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
				Scope* last_updated_scope = this->best_branch_experiment->scope_context;

				this->best_branch_experiment->add(this);

				this->solution->curr_score = this->best_branch_experiment->calc_new_score();

				delete this->best_branch_experiment;
				this->best_branch_experiment = NULL;

				clean_scope(last_updated_scope,
							this);

				this->solution->timestamp++;
				if (this->solution->timestamp >= RUN_TIMESTEPS) {
					this->solution->timestamp = -1;
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
