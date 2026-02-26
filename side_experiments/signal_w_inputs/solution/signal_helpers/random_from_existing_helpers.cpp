#include "signal_helpers.h"

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void random_explore_index_helper(ScopeHistory* scope_history,
								 vector<int>& explore_index) {
	uniform_int_distribution<int> distribution(0, scope_history->node_histories.size()-1);
	int index = distribution(generator);

	explore_index.push_back(index);

	AbstractNodeHistory* node_history = next(scope_history->node_histories.begin(), index)->second;
	if (node_history->node->type == NODE_TYPE_SCOPE) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
		random_explore_index_helper(scope_node_history->scope_history,
									explore_index);
	}
}

void random_signal_samples_helper(ScopeHistory* scope_history,
								  map<Scope*, pair<int, ScopeHistory*>>& samples) {
	Scope* scope = scope_history->scope;

	map<Scope*, pair<int, ScopeHistory*>>::iterator it = samples.find(scope);
	if (it == samples.end()) {
		samples[scope] = {1, scope_history};
	} else {
		uniform_int_distribution<int> distribution(0, it->second.first);
		if (distribution(generator) == 0) {
			it->second.second = scope_history;
		}
		it->second.first++;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			random_signal_samples_helper(scope_node_history->scope_history,
										 samples);
		}
	}
}

void random_from_existing_iter(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();
	Problem* problem = problem_type->get_problem();
	wrapper->problem = problem;

	wrapper->num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(wrapper->solution->scopes[0]);
	wrapper->scope_histories.push_back(scope_history);
	wrapper->node_context.push_back(wrapper->solution->scopes[0]->nodes[0]);

	while (true) {
		vector<double> obs = problem->get_observations();

		int action;
		bool is_next = false;
		bool is_done = false;
		while (!is_next) {
			if (wrapper->node_context.back() == NULL) {
				if (wrapper->scope_histories.size() == 1) {
					is_next = true;
					is_done = true;
				} else {
					ScopeNode* scope_node = (ScopeNode*)wrapper->node_context[wrapper->node_context.size() - 2];
					scope_node->exit_step(wrapper);
				}
			} else {
				wrapper->node_context.back()->step(obs,
												   action,
												   is_next,
												   wrapper);
			}
		}

		if (is_done) {
			break;
		} else {
			problem->perform_action(action);
		}
	}

	double target_val = problem->score_result();
	target_val -= 0.0001 * wrapper->num_actions;

	map<Scope*, pair<int, ScopeHistory*>> samples;
	random_signal_samples_helper(scope_history,
								 samples);

	for (map<Scope*, pair<int, ScopeHistory*>>::iterator it = samples.begin();
			it != samples.end(); it++) {
		vector<int> explore_index;
		random_explore_index_helper(it->second.second,
									explore_index);

		pre_signal_add_sample(it->second.second,
							  explore_index,
							  target_val,
							  wrapper);
	}

	delete wrapper->scope_histories[0];

	wrapper->scope_histories.clear();
	wrapper->node_context.clear();

	delete problem;
}
