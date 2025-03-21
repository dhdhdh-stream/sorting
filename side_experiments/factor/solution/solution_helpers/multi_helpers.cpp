#include "solution_helpers.h"

#include <chrono>
#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "multi_branch_experiment.h"
#include "multi_commit_experiment.h"
#include "multi_pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void multi_iter() {
	auto start_time = chrono::high_resolution_clock::now();

	int improvement_iter = 0;

	while (true) {
		run_index++;

		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "improvement_iter: " << improvement_iter << endl;
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->experiment_activate(
				problem,
				run_helper,
				scope_history);

		double target_val = problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		target_val -= run_helper.num_analyze * solution->curr_time_penalty;

		if (run_helper.multi_experiment_histories.size() < 3) {
			create_multi_experiment(scope_history);
		}

		delete scope_history;
		delete problem;

		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
				it != run_helper.multi_experiment_histories.end(); it++) {
			it->first->backprop(target_val,
								run_helper);
		}
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
				it != run_helper.multi_experiment_histories.end(); it++) {
			if (it->first->result == EXPERIMENT_RESULT_FAIL) {
				it->first->finalize(NULL);
				delete it->first;
			} else if (it->first->result == EXPERIMENT_RESULT_SUCCESS) {
				it->first->finalize(solution);
				delete it->first;

				improvement_iter++;
			}
		}

		// if (improvement_iter >= 10) {
		if (improvement_iter >= 1) {
			break;
		}
	}

	solution->clear_experiments();

	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		clean_scope(solution->scopes[s_index],
					solution);

		if (solution->scopes[s_index]->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
			solution->scopes[s_index]->exceeded = true;
		}
		if (solution->scopes[s_index]->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
			solution->scopes[s_index]->exceeded = false;
		}
	}

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

void multi_branch_iter() {
	auto start_time = chrono::high_resolution_clock::now();

	MultiBranchExperiment* best_experiment = NULL;
	int improvement_iter = 0;

	while (true) {
		run_index++;

		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "improvement_iter: " << improvement_iter << endl;
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->experiment_activate(
				problem,
				run_helper,
				scope_history);

		double target_val = problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		target_val -= run_helper.num_analyze * solution->curr_time_penalty;

		if (run_helper.multi_experiment_histories.size() < 3) {
			create_multi_branch_experiment(scope_history);
		}

		delete scope_history;
		delete problem;

		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
				it != run_helper.multi_experiment_histories.end(); it++) {
			it->first->backprop(target_val,
								run_helper);
		}
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
				it != run_helper.multi_experiment_histories.end(); it++) {
			MultiBranchExperiment* multi_branch_experiment = (MultiBranchExperiment*)it->first;
			if (multi_branch_experiment->result == EXPERIMENT_RESULT_FAIL) {
				multi_branch_experiment->clear();
				delete multi_branch_experiment;
			} else if (multi_branch_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				multi_branch_experiment->clear();
				if (best_experiment == NULL) {
					best_experiment = multi_branch_experiment;
				} else {
					if (multi_branch_experiment->improvement > best_experiment->improvement) {
						delete best_experiment;
						best_experiment = multi_branch_experiment;
					} else {
						delete multi_branch_experiment;
					}
				}

				improvement_iter++;
			}
		}

		// if (improvement_iter >= 10) {
		if (improvement_iter >= 1) {
			break;
		}
	}

	best_experiment->finalize(solution);
	delete best_experiment;

	solution->clear_experiments();

	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		clean_scope(solution->scopes[s_index],
					solution);

		if (solution->scopes[s_index]->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
			solution->scopes[s_index]->exceeded = true;
		}
		if (solution->scopes[s_index]->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
			solution->scopes[s_index]->exceeded = false;
		}
	}

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

void multi_commit_iter() {
	auto start_time = chrono::high_resolution_clock::now();

	int improvement_iter = 0;

	while (true) {
		run_index++;

		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			start_time = curr_time;

			cout << "improvement_iter: " << improvement_iter << endl;
		}

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->experiment_activate(
				problem,
				run_helper,
				scope_history);

		double target_val = problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		target_val -= run_helper.num_analyze * solution->curr_time_penalty;

		if (run_helper.num_multi_instances == 0) {
			create_multi_commit_experiment(scope_history);
		} else {
			int ratio = run_helper.num_original_actions / run_helper.num_multi_instances;
			if (ratio > 200) {
				create_multi_commit_experiment(scope_history);
			}
		}

		delete scope_history;
		delete problem;

		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
				it != run_helper.multi_experiment_histories.end(); it++) {
			it->first->backprop(target_val,
								run_helper);
		}
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
				it != run_helper.multi_experiment_histories.end(); it++) {
			if (it->first->result == EXPERIMENT_RESULT_FAIL) {
				it->first->finalize(NULL);
				delete it->first;
			} else if (it->first->result == EXPERIMENT_RESULT_SUCCESS) {
				it->first->finalize(solution);
				delete it->first;

				improvement_iter++;
			}
		}

		// if (improvement_iter >= 10) {
		if (improvement_iter >= 1) {
			break;
		}
	}

	solution->clear_experiments();

	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		clean_scope(solution->scopes[s_index],
					solution);

		if (solution->scopes[s_index]->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
			solution->scopes[s_index]->exceeded = true;
		}
		if (solution->scopes[s_index]->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
			solution->scopes[s_index]->exceeded = false;
		}
	}

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

	// temp
	ofstream display_file;
	display_file.open("../display.txt");
	solution->save_for_display(display_file);
	display_file.close();

	cout << "solution->curr_score: " << solution->curr_score << endl;
}
