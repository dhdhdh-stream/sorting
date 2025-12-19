#include "solution_wrapper.h"

#include <iostream>

#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "signal_experiment.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);

	scope_history->pre_obs = this->problem->get_observations();
}

pair<bool,int> SolutionWrapper::step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
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

	return {is_done, action};
}

void SolutionWrapper::end() {
	this->scope_histories[0]->post_obs = this->problem->get_observations();

	/**
	 * - debug
	 */
	if (this->solution->scopes[0]->signal_experiment != NULL) {
		cout << "pre_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << this->scope_histories.back()->pre_obs[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}
		cout << "post_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << this->scope_histories.back()->post_obs[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}

		vector<double> input = this->scope_histories.back()->pre_obs;
		input.insert(input.end(), this->scope_histories.back()->post_obs.begin(),
			this->scope_histories.back()->post_obs.end());

		this->solution->scopes[0]->signal_experiment->consistency_network->activate(input);
		cout << "this->solution->scopes[0]->signal_experiment->consistency_network->output->acti_vals[0]: " << this->solution->scopes[0]->signal_experiment->consistency_network->output->acti_vals[0] << endl;

		this->solution->scopes[0]->signal_experiment->signal_network->activate(input);
		cout << "this->solution->scopes[0]->signal_experiment->signal_network->output->acti_vals[0]: " << this->solution->scopes[0]->signal_experiment->signal_network->output->acti_vals[0] << endl;

		cout << endl;
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
}
