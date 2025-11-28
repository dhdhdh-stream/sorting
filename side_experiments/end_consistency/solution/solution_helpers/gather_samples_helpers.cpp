#include "solution_helpers.h"

#include <fstream>
#include <iostream>

#include "globals.h"
#include "problem.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int SAMPLES_NUM_SAVE = 10;
#else
const int SAMPLES_NUM_SAVE = 4000;
#endif /* MDEBUG */

void gather_samples_helper(ProblemType* problem_type,
						   SolutionWrapper* wrapper,
						   string path,
						   string name) {
	while (wrapper->solution->explore_pre_obs.back().size() > SAMPLES_NUM_SAVE) {
		uniform_int_distribution<int> distribution(0, wrapper->solution->explore_pre_obs.back().size()-1);
		int index = distribution(generator);
		wrapper->solution->explore_pre_obs.back().erase(wrapper->solution->explore_pre_obs.back().begin() + index);
		wrapper->solution->explore_post_obs.back().erase(wrapper->solution->explore_post_obs.back().begin() + index);
		wrapper->solution->explore_scores.back().erase(wrapper->solution->explore_scores.back().begin() + index);
	}

	for (int iter_index = 0; iter_index < SAMPLES_NUM_SAVE; iter_index++) {
		Problem* problem = problem_type->get_problem();
		wrapper->problem = problem;

		wrapper->init();

		wrapper->solution->signal_pre_obs.back().push_back(problem->get_observations());

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = wrapper->step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		wrapper->solution->signal_post_obs.back().push_back(problem->get_observations());

		double target_val = problem->score_result();
		target_val -= 0.0001 * wrapper->num_actions;

		wrapper->solution->signal_scores.back().push_back(target_val);

		wrapper->end();

		delete problem;
	}

	ofstream output_file;
	output_file.open(path + "samples_" + name);

	output_file << wrapper->solution->signal_pre_obs.size() << endl;
	for (int t_index = 0; t_index < (int)wrapper->solution->signal_pre_obs.size(); t_index++) {
		for (int s_index = 0; s_index < SAMPLES_NUM_SAVE; s_index++) {
			for (int i_index = 0; i_index < (int)wrapper->solution->signal_pre_obs[t_index][s_index].size(); i_index++) {
				output_file << wrapper->solution->signal_pre_obs[t_index][s_index][i_index] << endl;
			}
			for (int i_index = 0; i_index < (int)wrapper->solution->signal_post_obs[t_index][s_index].size(); i_index++) {
				output_file << wrapper->solution->signal_post_obs[t_index][s_index][i_index] << endl;
			}
			output_file << wrapper->solution->signal_scores[t_index][s_index] << endl;
		}

		for (int s_index = 0; s_index < SAMPLES_NUM_SAVE; s_index++) {
			for (int i_index = 0; i_index < (int)wrapper->solution->explore_pre_obs[t_index][s_index].size(); i_index++) {
				output_file << wrapper->solution->explore_pre_obs[t_index][s_index][i_index] << endl;
			}
			for (int i_index = 0; i_index < (int)wrapper->solution->explore_post_obs[t_index][s_index].size(); i_index++) {
				output_file << wrapper->solution->explore_post_obs[t_index][s_index][i_index] << endl;
			}
			output_file << wrapper->solution->explore_scores[t_index][s_index] << endl;
		}
	}

	output_file.close();

	wrapper->solution->signal_pre_obs.push_back(vector<vector<double>>());
	wrapper->solution->signal_post_obs.push_back(vector<vector<double>>());
	wrapper->solution->signal_scores.push_back(vector<double>());
	wrapper->solution->explore_pre_obs.push_back(vector<vector<double>>());
	wrapper->solution->explore_post_obs.push_back(vector<vector<double>>());
	wrapper->solution->explore_scores.push_back(vector<double>());
}
