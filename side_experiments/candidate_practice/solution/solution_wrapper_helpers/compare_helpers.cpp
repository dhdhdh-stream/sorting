#include "solution_wrapper.h"

#include "abstract_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void SolutionWrapper::compare_init() {
	this->compare_num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->compare_scope_histories.push_back(scope_history);
	this->compare_node_context.push_back(this->solution->scopes[0]->nodes[0]);
	this->compare_experiment_context.push_back(NULL);
}

pair<bool,int> SolutionWrapper::compare_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->compare_node_context.back() == NULL
				&& this->compare_experiment_context.back() == NULL) {
			if (this->compare_scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				if (this->compare_experiment_context[this->compare_experiment_context.size() - 2] != NULL) {
					AbstractExperiment* experiment = this->compare_experiment_context[this->compare_experiment_context.size() - 2]->experiment;
					experiment->compare_exit_step(this);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->compare_node_context[this->compare_node_context.size() - 2];
					scope_node->compare_exit_step(this);
				}
			}
		} else if (this->compare_experiment_context.back() != NULL) {
			AbstractExperiment* experiment = this->compare_experiment_context.back()->experiment;
			experiment->compare_step(obs,
									 action,
									 is_next,
									 this);
		} else {
			this->compare_node_context.back()->compare_step(obs,
															action,
															is_next,
															this);
		}
	}

	return {is_done, action};
}

void SolutionWrapper::compare_end() {
	delete this->compare_scope_histories[0];

	this->compare_scope_histories.clear();
	this->compare_node_context.clear();
	this->compare_experiment_context.clear();
}
