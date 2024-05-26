#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "distance.h"
#include "eval.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "Usage: ./worker [path]" << endl;
		exit(1);
	}
	string path = argv[1];

	/**
	 * - worker directories need to have already been created
	 */

	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new Distance();

	solution = new Solution();
	solution->load("workers/", "main");

	auto start_time = chrono::high_resolution_clock::now();
	while (true) {
		Problem* problem = new Distance();

		RunHelper run_helper;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = solution->scopes[0];
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		context.back().scope_history = root_history;

		run_helper.experiment_scope_history = root_history;

		EvalHistory* eval_history = new EvalHistory(solution->scopes[0]->eval);

		solution->scopes[0]->eval->activate_start(problem,
												  run_helper,
												  eval_history);

		problem->perform_action(Action(DISTANCE_ACTION_SEQUENCE));

		solution->scopes[0]->eval->activate_end(problem,
												run_helper,
												eval_history);

		if (run_helper.num_actions <= solution->num_actions_limit) {
			if (root_history->experiments_seen_order.size() == 0) {
				create_eval_experiment(eval_history);
			}

			if (root_history->experiment_histories.size() > 0) {
				for (int e_index = 0; e_index < (int)root_history->experiments_seen_order.size(); e_index++) {
					AbstractExperiment* experiment = root_history->experiments_seen_order[e_index];
					experiment->average_remaining_experiments_from_start =
						0.9 * experiment->average_remaining_experiments_from_start
						+ 0.1 * ((int)root_history->experiments_seen_order.size()-1 - e_index
							+ root_history->experiment_histories[0]->experiment->average_remaining_experiments_from_start);
				}

				root_history->experiment_histories.back()->experiment->backprop(
					NULL,
					eval_history,
					problem,
					context,
					run_helper);
				if (root_history->experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_FAIL) {
					root_history->experiment_histories.back()->experiment->finalize(NULL);
					delete root_history->experiment_histories.back()->experiment;
				} else if (root_history->experiment_histories.back()->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
					/**
					 * - root_history->experiment_histories.size() == 1
					 */
					Solution* duplicate = new Solution(solution);
					root_history->experiment_histories.back()->experiment->finalize(duplicate);
					delete root_history->experiment_histories.back()->experiment;

					while (true) {
						double sum_timestamp_score = 0.0;
						int timestamp_score_count = 0;
						double sum_instances_per_run = 0;
						double sum_local_num_actions = 0.0;
						int num_runs = 0;
						vector<double> impacts;
						vector<double> misguesses;
						while (true) {
							Problem* problem = new Distance();

							RunHelper run_helper;

							Metrics metrics(solution->explore_id,
											solution->explore_type,
											duplicate->explore_id,
											duplicate->explore_type);

							vector<ContextLayer> context;
							context.push_back(ContextLayer());

							context.back().scope = duplicate->scopes[0];
							context.back().node = NULL;

							ScopeHistory* root_history = new ScopeHistory(duplicate->scopes[0]);
							context.back().scope_history = root_history;

							duplicate->scopes[0]->measure_activate(
								metrics,
								problem,
								context,
								run_helper,
								root_history);

							delete root_history;

							sum_timestamp_score += metrics.curr_sum_timestamp_score;
							timestamp_score_count += metrics.curr_num_instances;
							if (run_helper.num_actions > duplicate->max_num_actions) {
								duplicate->max_num_actions = run_helper.num_actions;
							}
							sum_instances_per_run += metrics.next_num_instances;
							if (metrics.next_max_num_actions > duplicate->explore_scope_max_num_actions) {
								duplicate->explore_scope_max_num_actions = metrics.next_max_num_actions;
							}
							sum_local_num_actions += metrics.next_local_sum_num_actions;
							num_runs++;
							for (int i_index = 0; i_index < (int)metrics.next_impacts.size(); i_index++) {
								impacts.push_back(metrics.next_impacts[i_index]);
							}
							for (int m_index = 0; m_index < (int)metrics.next_misguesses.size(); m_index++) {
								misguesses.push_back(metrics.next_misguesses[m_index]);
							}

							delete problem;

							if (timestamp_score_count > MEASURE_ITERS) {
								break;
							}
						}
						if (sum_instances_per_run > 0) {
							duplicate->timestamp_score = sum_timestamp_score / timestamp_score_count;
							duplicate->explore_average_instances_per_run = (double)sum_instances_per_run / (double)num_runs;
							duplicate->explore_scope_local_average_num_actions = sum_local_num_actions / sum_instances_per_run;
							double sum_impacts = 0.0;
							for (int i_index = 0; i_index < (int)impacts.size(); i_index++) {
								sum_impacts += impacts[i_index];
							}
							duplicate->explore_scope_average_impact = sum_impacts / (int)impacts.size();
							double sum_impact_variance = 0.0;
							for (int i_index = 0; i_index < (int)impacts.size(); i_index++) {
								sum_impact_variance += (impacts[i_index] - duplicate->explore_scope_average_impact) * (impacts[i_index] - duplicate->explore_scope_average_impact);
							}
							duplicate->explore_scope_impact_standard_deviation = sqrt(sum_impact_variance / (int)impacts.size());
							double sum_misguesses = 0.0;
							for (int m_index = 0; m_index < (int)misguesses.size(); m_index++) {
								sum_misguesses += misguesses[m_index];
							}
							duplicate->explore_scope_average_misguess = sum_misguesses / (int)misguesses.size();
							double sum_misguess_variance = 0.0;
							for (int m_index = 0; m_index < (int)misguesses.size(); m_index++) {
								sum_misguess_variance += (misguesses[m_index] - duplicate->explore_scope_average_misguess) * (misguesses[m_index] - duplicate->explore_scope_average_misguess);
							}
							duplicate->explore_scope_misguess_standard_deviation = sqrt(sum_misguess_variance / (int)misguesses.size());

							cout << "duplicate->timestamp_score: " << duplicate->timestamp_score << endl;

							break;
						} else {
							duplicate->explore_id = 0;
							duplicate->explore_type = EXPLORE_TYPE_EVAL;
							duplicate->explore_scope_max_num_actions = 1;
						}
					}

					duplicate->timestamp++;

					duplicate->save(path, "possible_" + to_string((unsigned)time(NULL)));

					delete duplicate;
				}
			} else {
				for (int e_index = 0; e_index < (int)root_history->experiments_seen_order.size(); e_index++) {
					AbstractExperiment* experiment = root_history->experiments_seen_order[e_index];
					experiment->average_remaining_experiments_from_start =
						0.9 * experiment->average_remaining_experiments_from_start
						+ 0.1 * ((int)root_history->experiments_seen_order.size()-1 - e_index);
				}
			}
		}

		delete eval_history;

		delete root_history;

		delete problem;

		auto curr_time = chrono::high_resolution_clock::now();
		auto time_diff = chrono::duration_cast<chrono::seconds>(curr_time - start_time);
		if (time_diff.count() >= 20) {
			cout << "alive" << endl;

			ifstream solution_save_file;
			solution_save_file.open("workers/saves/main.txt");
			string timestamp_line;
			getline(solution_save_file, timestamp_line);
			int curr_timestamp = stoi(timestamp_line);
			solution_save_file.close();

			if (curr_timestamp > solution->timestamp) {
				delete solution;

				solution = new Solution();
				solution->load("workers/", "main");

				cout << "updated from main" << endl;
			}

			start_time = curr_time;
		}
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
