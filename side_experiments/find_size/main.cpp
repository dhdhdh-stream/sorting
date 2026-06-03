#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"
#include "test_indirect.h"
#include "world_model.h"
#include "world_model_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

#if defined(MDEBUG) && MDEBUG
const int SAMPLES_PER_EPOCH = 40;
#else
const int SAMPLES_PER_EPOCH = 100000;
#endif /* MDEBUG */

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeTestIndirect();

	WorldModel* curr = new WorldModel(problem_type->num_obs(),
									  problem_type->num_possible_actions());

	WorldModel* small = new WorldModel(curr);
	small->remove_states();

	WorldModel* large = new WorldModel(curr);
	large->add_states();

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
	while (true) {
		double sum_small_error = 0.0;
		double sum_curr_error = 0.0;
		double sum_large_error = 0.0;
		for (int sample_index = 0; sample_index < SAMPLES_PER_EPOCH; sample_index++) {
			Problem* problem = problem_type->get_problem();

			vector<vector<double>> curr_obs;
			vector<int> curr_actions;

			curr_obs.push_back(problem->get_observations());

			int num_actions = num_actions_distribution(generator);
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);
				curr_actions.push_back(action);

				curr_obs.push_back(problem->get_observations());
			}

			double target_val = problem->score_result();

			double small_error;
			update_world_model_helper(curr_obs,
									  curr_actions,
									  target_val,
									  small_error,
									  small);
			sum_small_error += small_error;

			double curr_error;
			update_world_model_helper(curr_obs,
									  curr_actions,
									  target_val,
									  curr_error,
									  curr);
			sum_curr_error += curr_error;

			double large_error;
			update_world_model_helper(curr_obs,
									  curr_actions,
									  target_val,
									  large_error,
									  large);
			sum_large_error += large_error;

			delete problem;
		}

		cout << "sum_small_error: " << sum_small_error << endl;
		measure_test(small);
		cout << "sum_curr_error: " << sum_curr_error << endl;
		measure_test(curr);
		cout << "sum_large_error: " << sum_large_error << endl;
		measure_test(large);

		double small_denom = sqrt((small->misguess_variance_average + curr->misguess_variance_average) / 10000.0);
		double small_t_score = (curr->misguess_average - small->misguess_average) / small_denom;

		double large_denom = sqrt((large->misguess_variance_average + curr->misguess_variance_average) / 10000.0);
		double large_t_score = (curr->misguess_average - large->misguess_average) / large_denom;

		if (small_t_score >= 1.645) {
			delete curr;
			curr = small;

			delete large;

			small = new WorldModel(curr);
			small->remove_states();

			large = new WorldModel(curr);
			large->add_states();
		} else if (large_t_score >= 1.645) {
			delete curr;
			curr = large;

			delete small;

			small = new WorldModel(curr);
			small->remove_states();

			large = new WorldModel(curr);
			large->add_states();
		}
	}

	delete problem_type;

	cout << "Done" << endl;
}
