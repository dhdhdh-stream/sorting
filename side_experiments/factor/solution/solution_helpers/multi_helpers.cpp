#include "solution_helpers.h"

#include <chrono>
#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "multi_pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void multi_iter() {
	auto start_time = chrono::high_resolution_clock::now();

	int multi_id = 0;
	int improvement_iter = 0;

	while (true) {
		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "improvement_iter: " << improvement_iter << endl;
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->experiment_activate(
				problem,
				run_helper,
				scope_history);

		double target_val = problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		target_val -= run_helper.num_analyze * solution->curr_time_penalty;

		if (run_helper.multi_experiment_histories.size() < 3) {
			create_multi_experiment(scope_history,
									multi_id);
		}

		delete scope_history;
		delete problem;

		set<Scope*> updated_scopes;
		for (map<MultiPassThroughExperiment*, MultiPassThroughExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
				it != run_helper.multi_experiment_histories.end(); it++) {
			it->first->backprop(target_val,
								run_helper);
			if (it->first->result == EXPERIMENT_RESULT_FAIL) {
				it->first->finalize(NULL);
				delete it->first;
			} else if (it->first->result == EXPERIMENT_RESULT_SUCCESS) {
				updated_scopes.insert(it->first->scope_context);

				it->first->finalize(solution);
				delete it->first;

				improvement_iter++;
			}
		}

		for (set<Scope*>::iterator it = updated_scopes.begin();
				it != updated_scopes.end(); it++) {
			clean_scope(*it,
						solution);

			if ((*it)->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
				(*it)->exceeded = true;
			}
			if ((*it)->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
				(*it)->exceeded = false;
			}
		}

		if (improvement_iter >= 10) {
			break;
		}
	}

	solution->clear_experiments();

	double sum_score = 0.0;
	double sum_true_score = 0.0;
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "improvement_iter: " << improvement_iter << endl;
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->measure_activate(
			problem,
			run_helper,
			scope_history);
		delete scope_history;

		double target_val = problem->score_result();
		sum_score += target_val - 0.05 * run_helper.num_actions * solution->curr_time_penalty
			- run_helper.num_analyze * solution->curr_time_penalty;
		sum_true_score += target_val;

		delete problem;
	}

	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		for (map<int, AbstractNode*>::iterator it = solution->scopes[s_index]->nodes.begin();
				it != solution->scopes[s_index]->nodes.end(); it++) {
			it->second->average_instances_per_run /= MEASURE_ITERS;
			if (it->second->average_instances_per_run < 1.0) {
				it->second->average_instances_per_run = 1.0;
			}
		}
	}

	solution->curr_score = sum_score / MEASURE_ITERS;
	solution->curr_true_score = sum_true_score / MEASURE_ITERS;

	cout << "solution->curr_score: " << solution->curr_score << endl;
}
