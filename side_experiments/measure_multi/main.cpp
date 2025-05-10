// can't capture sharp good changes
// - other experiments alter location, changing context, making a good change bad
// TODO: try fixing location?

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "obs_node.h"
#include "pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int multi_index = 0;

int run_index;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	string filename;
	if (argc > 1) {
		filename = argv[1];
		solution->load("saves/", filename);
	} else {
		filename = "main.txt";
		solution->init();
		solution->save("saves/", filename);
	}

	{
		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();
	}

	run_index = 0;

	while (solution->timestamp < EXPLORE_ITERS) {
		vector<AbstractNode*> experiment_starts;
		vector<AbstractNode*> experiment_exit_next_nodes;
		vector<AbstractExperiment*> experiments;
		set_experiment_nodes(experiment_starts,
							 experiment_exit_next_nodes,
							 experiments);

		int curr_experiment_index;
		int improvement_iter = 0;
		create_experiment(experiment_starts,
						  experiment_exit_next_nodes,
						  experiments,
						  curr_experiment_index,
						  improvement_iter);

		while (true) {
			run_index++;
			if (run_index%100000 == 0) {
				cout << "run_index: " << run_index << endl;
				cout << "solution->timestamp: " << solution->timestamp << endl;
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
			target_val -= run_helper.num_actions * solution->curr_time_penalty;

			bool is_return;
			Minesweeper* minesweeper = (Minesweeper*)problem;
			if (minesweeper->current_x == 4
					&& minesweeper->current_y == 4) {
				is_return = true;
			} else {
				is_return = false;
			}

			delete scope_history;
			delete problem;

			for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.experiment_histories.begin();
					it != run_helper.experiment_histories.end(); it++) {
				it->first->backprop(target_val,
									is_return,
									run_helper);
				if (it->first->result == EXPERIMENT_RESULT_FAIL) {
					it->first->unattach();
					delete it->first;

					create_experiment(experiment_starts,
									  experiment_exit_next_nodes,
									  experiments,
									  curr_experiment_index,
									  improvement_iter);
				} else if (it->first->result == EXPERIMENT_RESULT_SUCCESS) {
					cout << "EXPERIMENT_RESULT_SUCCESS " << curr_experiment_index << endl;

					it->first->unattach();
					experiments[curr_experiment_index] = it->first;
					improvement_iter++;

					if (improvement_iter < (int)experiments.size()) {
						create_experiment(experiment_starts,
										  experiment_exit_next_nodes,
										  experiments,
										  curr_experiment_index,
										  improvement_iter);
					}
				}
			}

			if (improvement_iter >= (int)experiments.size()) {
				break;
			}
		}

		for (int e_index = 0; e_index < (int)experiments.size(); e_index++) {
			experiments[e_index]->attach();
		}

		map<AbstractExperiment*, int> experiment_mapping;
		for (int e_index = 0; e_index < (int)experiments.size(); e_index++) {
			experiment_mapping[experiments[e_index]] = e_index;
		}
		vector<vector<double>> influences;
		vector<double> target_vals;
		vector<int> counts(experiments.size(), 0);
		for (int iter_index = 0; iter_index < MULTI_MEASURE_ITERS; iter_index++) {
			run_index++;
			if (run_index%100000 == 0) {
				cout << "run_index: " << run_index << endl;
				cout << "solution->timestamp: " << solution->timestamp << endl;
				cout << "improvement_iter: " << improvement_iter << endl;
			}

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			// for (int e_index = 0; e_index < (int)experiments.size(); e_index++) {
			// 	switch (experiments[e_index]->type) {
			// 	case EXPERIMENT_TYPE_BRANCH:
			// 		{
			// 			BranchExperiment* branch_experiment = (BranchExperiment*)experiments[e_index];
			// 			run_helper.experiment_histories[branch_experiment] = new BranchExperimentHistory(branch_experiment);
			// 		}
			// 		break;
			// 	case EXPERIMENT_TYPE_PASS_THROUGH:
			// 		{
			// 			PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)experiments[e_index];
			// 			run_helper.experiment_histories[pass_through_experiment] = new PassThroughExperimentHistory(pass_through_experiment);
			// 		}
			// 		break;
			// 	}
			// }

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
			target_val -= run_helper.num_actions * solution->curr_time_penalty;

			bool is_return;
			Minesweeper* minesweeper = (Minesweeper*)problem;
			if (minesweeper->current_x == 4
					&& minesweeper->current_y == 4) {
				is_return = true;
			} else {
				is_return = false;
			}

			vector<double> curr_influences(experiments.size(), 0.0);
			for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.experiment_histories.begin();
					it != run_helper.experiment_histories.end(); it++) {
				int index = experiment_mapping[it->first];
				if (it->second->is_active) {
					curr_influences[index] = 1.0;
				} else {
					// curr_influences[index] = -1.0;
				}
				counts[index]++;
			}
			influences.push_back(curr_influences);
			target_vals.push_back(target_val);

			delete scope_history;
			delete problem;

			for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.experiment_histories.begin();
					it != run_helper.experiment_histories.end(); it++) {
				it->first->backprop(target_val,
									is_return,
									run_helper);
			}
		}

		vector<double> multi_scores;
		multi_measure_calc(influences,
						   target_vals,
						   multi_scores);

		for (int e_index = 0; e_index < (int)experiments.size(); e_index++) {
			cout << e_index << endl;

			cout << "experiments[e_index]->type: " << experiments[e_index]->type << endl;

			experiments[e_index]->multi_measure_calc();

			cout << "experiments[e_index]->node_context->id: " << experiments[e_index]->node_context->id << endl;
			if (experiments[e_index]->exit_next_node == NULL) {
				cout << "experiments[e_index]->exit_next_node->id: -1" << endl;
			} else {
				cout << "experiments[e_index]->exit_next_node->id: " << experiments[e_index]->exit_next_node->id << endl;
			}

			cout << "experiments[e_index]->true_improvement: " << experiments[e_index]->true_improvement << endl;
			cout << "experiments[e_index]->improvement: " << experiments[e_index]->improvement << endl;

			cout << "multi_scores[e_index]: " << multi_scores[e_index] << endl;
			cout << "counts[e_index]: " << counts[e_index] << endl;
		}

		int best_index = 0;
		// double best_score = experiments[0]->true_improvement;
		// double best_score = multi_scores[0];
		double best_score = experiments[0]->improvement;
		for (int e_index = 1; e_index < (int)experiments.size(); e_index++) {
			// if (experiments[e_index]->true_improvement > best_score) {
			// if (multi_scores[e_index] > best_score) {
			if (experiments[e_index]->improvement > best_score) {
				best_index = e_index;
				// best_score = experiments[e_index]->true_improvement;
				// best_score = multi_scores[e_index];
				best_score = experiments[e_index]->improvement;
			}
		}

		Scope* last_updated_scope;
		for (int e_index = 0; e_index < (int)experiments.size(); e_index++) {
			experiments[e_index]->unattach();
			if (e_index == best_index) {
				last_updated_scope = experiments[e_index]->scope_context;

				experiments[e_index]->add();
			}
			delete experiments[e_index];
		}

		clean_scope(last_updated_scope);

		double sum_score = 0.0;
		double sum_true_score = 0.0;
		for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
			run_index++;

			Problem* problem = problem_type->get_problem();

			RunHelper run_helper;

			#if defined(MDEBUG) && MDEBUG
			run_helper.starting_run_seed = run_index;
			run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
			#endif /* MDEBUG */

			ScopeHistory* scope_history = new ScopeHistory(solution->scopes[0]);
			solution->scopes[0]->measure_activate(
				problem,
				run_helper,
				scope_history);
			delete scope_history;

			double target_val = problem->score_result();
			sum_score += target_val - run_helper.num_actions * solution->curr_time_penalty;
			sum_true_score += target_val;

			delete problem;
		}

		solution->measure_update();

		solution->curr_score = sum_score / MEASURE_ITERS;
		solution->curr_true_score = sum_true_score / MEASURE_ITERS;

		cout << "solution->curr_score: " << solution->curr_score << endl;

		solution->timestamp++;

		if (solution->timestamp % INCREASE_TIME_PENALTY_ITER == 0) {
			solution->curr_time_penalty *= 1.25;
		}
		if (solution->curr_true_score > solution->best_true_score) {
			solution->best_true_score = solution->curr_true_score;
			solution->best_true_score_timestamp = solution->timestamp;
		}
		if (solution->best_true_score_timestamp < solution->timestamp
				&& (solution->timestamp - solution->best_true_score_timestamp)
					% DECREASE_TIME_PENALTY_ITER == 0) {
			solution->curr_time_penalty *= 0.8;
		}

		solution->save("saves/", filename);

		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();

		#if defined(MDEBUG) && MDEBUG
		delete solution;
		solution = new Solution();
		solution->load("saves/", filename);
		#endif /* MDEBUG */
	}

	solution->clean_scopes();
	solution->save("saves/", filename);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
