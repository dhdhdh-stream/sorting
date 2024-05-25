#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "distance.h"
#include "eval.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new Distance();

	solution = new Solution();
	solution->init();
	// solution->load("", "main");

	solution->save("", "main");

	#if defined(MDEBUG) && MDEBUG
	int run_index = 0;
	#endif /* MDEBUG */

	Eval* eval = solution->scopes[0]->eval;

	while (true) {
		Problem* problem = new Distance();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.curr_run_seed = run_index;
		run_index++;
		#endif /* MDEBUG */

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope = solution->scopes[0];
		context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		context.back().scope_history = root_history;

		#if defined(MDEBUG) && MDEBUG
		run_helper.problem_snapshot = problem->copy_snapshot();
		run_helper.run_seed_snapshot = run_helper.curr_run_seed;
		#endif /* MDEBUG */

		run_helper.experiment_scope_history = root_history;

		EvalHistory* eval_history = new EvalHistory(eval);

		eval->activate_start(problem,
							 run_helper,
							 eval_history);

		problem->perform_action(Action(DISTANCE_ACTION_SEQUENCE));

		eval->activate_end(problem,
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

					#if defined(MDEBUG) && MDEBUG
					while (duplicate->verify_problems.size() > 0) {
						Problem* problem = duplicate->verify_problems[0];

						RunHelper run_helper;
						run_helper.verify_key = duplicate->verify_key;

						run_helper.curr_run_seed = duplicate->verify_seeds[0];
						cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
						/**
						 * - also set to enable easy catching
						 */
						run_helper.run_seed_snapshot = duplicate->verify_seeds[0];
						duplicate->verify_seeds.erase(duplicate->verify_seeds.begin());

						vector<ContextLayer> context;
						context.push_back(ContextLayer());

						context.back().scope = duplicate->scopes[solution->explore_id];
						context.back().node = NULL;

						ScopeHistory* root_history = new ScopeHistory(duplicate->scopes[solution->explore_id]);
						context.back().scope_history = root_history;

						duplicate->scopes[solution->explore_id]->verify_activate(
							problem,
							context,
							run_helper,
							root_history);

						delete root_history;

						delete duplicate->verify_problems[0];
						duplicate->verify_problems.erase(duplicate->verify_problems.begin());
					}
					duplicate->clear_verify();
					#endif /* MDEBUG */

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
							uniform_int_distribution<int> explore_id_distribution(0, (int)duplicate->scopes.size()-1);
							duplicate->explore_id = explore_id_distribution(generator);
							if (duplicate->scopes[duplicate->explore_id]->eval->input_node_contexts.size() == 0) {
								duplicate->explore_type = EXPLORE_TYPE_EVAL;
							} else {
								uniform_int_distribution<int> explore_type_distribution(0, 1);
								duplicate->explore_type = explore_type_distribution(generator);
							}
							duplicate->explore_scope_max_num_actions = 1;
						}
					}

					#if defined(MDEBUG) && MDEBUG
					delete solution;
					solution = duplicate;

					solution->timestamp++;
					solution->save("", "main");

					ofstream display_file;
					display_file.open("../display.txt");
					solution->save_for_display(display_file);
					display_file.close();
					#else
					delete duplicate;
					#endif /* MDEBUG */
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

		#if defined(MDEBUG) && MDEBUG
		if (run_helper.problem_snapshot != NULL) {
			delete run_helper.problem_snapshot;
			run_helper.problem_snapshot = NULL;
		}
		#endif /* MDEBUG */

		delete root_history;

		delete problem;

		#if defined(MDEBUG) && MDEBUG
		if (run_index%2000 == 0) {
			delete solution;
			solution = new Solution();
			solution->load("", "main");
		}
		#endif /* MDEBUG */
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
