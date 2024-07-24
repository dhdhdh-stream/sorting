#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "solution.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;
Solution* solution;

int run_index;

const int TRIM_EPOCHS = 2;
const int TRIM_TRIES = 50;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeMinesweeper();

	solution = new Solution();
	solution->load("", "main");

	solution->save("", "checkpoint");

	int starting_num_nodes = 0;
	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		starting_num_nodes += (solution->scopes[s_index]->nodes.size() - 2);
	}
	cout << "starting_num_nodes: " << starting_num_nodes << endl;

	Solution* curr_solution = solution;

	for (int e_index = 0; e_index < TRIM_EPOCHS; e_index++) {
		Solution* best_trim = NULL;
		double best_score;
		for (int t_index = 0; t_index < TRIM_TRIES; t_index++) {
			Solution* duplicate = new Solution(curr_solution);
			duplicate->random_trim();

			double sum_score = 0.0;
			bool exceeded_limit = false;
			for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
				Problem* problem = problem_type->get_problem();

				RunHelper run_helper;

				#if defined(MDEBUG) && MDEBUG
				run_helper.starting_run_seed = run_index;
				run_helper.curr_run_seed = run_index;
				#endif /* MDEBUG */
				run_index++;

				vector<ContextLayer> context;
				duplicate->scopes[0]->measure_activate(
					problem,
					context,
					run_helper);

				double target_val;
				if (!run_helper.exceeded_limit) {
					target_val = problem->score_result(run_helper.num_analyze,
													   run_helper.num_actions);

					delete problem;

					sum_score += target_val;
				} else {
					delete problem;

					exceeded_limit = true;

					break;
				}
			}

			if (exceeded_limit) {
				cout << "exceeded_limit" << endl;
			} else {
				double average_score = sum_score / MEASURE_ITERS;

				cout << "average_score: " << average_score << endl;

				if (best_trim == NULL
						|| average_score > best_score) {
					if (best_trim != NULL) {
						delete best_trim;
					}
					best_trim = duplicate;
					best_score = average_score;
				} else {
					delete duplicate;
				}
			}
		}

		int ending_num_nodes = 0;
		for (int s_index = 0; s_index < (int)best_trim->scopes.size(); s_index++) {
			ending_num_nodes += (best_trim->scopes[s_index]->nodes.size() - 2);
		}
		cout << "ending_num_nodes: " << ending_num_nodes << endl;

		curr_solution = best_trim;
	}

	vector<double> target_vals;
	int max_num_actions = 0;
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		vector<ContextLayer> context;
		curr_solution->scopes[0]->measure_activate(
			problem,
			context,
			run_helper);

		if (run_helper.num_actions > max_num_actions) {
			max_num_actions = run_helper.num_actions;
		}

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = problem->score_result(run_helper.num_analyze,
											   run_helper.num_actions);
		} else {
			target_val = -1.0;
		}

		target_vals.push_back(target_val);

		delete problem;
	}

	for (int s_index = 0; s_index < (int)curr_solution->scopes.size(); s_index++) {
		for (map<int, AbstractNode*>::iterator it = curr_solution->scopes[s_index]->nodes.begin();
				it != curr_solution->scopes[s_index]->nodes.end(); it++) {
			it->second->average_instances_per_run /= MEASURE_ITERS;
			if (it->second->average_instances_per_run < 1.0) {
				it->second->average_instances_per_run = 1.0;
			}
		}
	}

	double sum_score = 0.0;
	for (int d_index = 0; d_index < MEASURE_ITERS; d_index++) {
		sum_score += target_vals[d_index];
	}
	curr_solution->average_score = sum_score / MEASURE_ITERS;

	curr_solution->max_num_actions = max_num_actions;

	curr_solution->timestamp++;

	curr_solution->save("", "main");

	ofstream display_file;
	display_file.open("../display.txt");
	curr_solution->save_for_display(display_file);
	display_file.close();

	delete curr_solution;

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
