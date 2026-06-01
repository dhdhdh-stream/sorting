// 1780288989

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "test_indirect.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

#if defined(MDEBUG) && MDEBUG
const int SAMPLES_PER_EPOCH = 40;
#else
const int SAMPLES_PER_EPOCH = 10000;
#endif /* MDEBUG */

const double SEQUENCE_RATIO = 0.3;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeTestIndirect();

	Wrapper* wrapper = new Wrapper(problem_type);

	int num_sequence = SEQUENCE_RATIO * SAMPLES_PER_EPOCH;

	geometric_distribution<int> num_actions_distribution(0.1);
	// uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);
	uniform_int_distribution<int> action_distribution(0, 1);
	{
		for (int sample_index = 0; sample_index < num_sequence; sample_index++) {
			Problem* problem = problem_type->get_problem();

			vector<vector<double>> curr_obs;
			vector<int> curr_actions;

			curr_obs.push_back(problem->get_observations());

			int num_actions = 1;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);
				curr_actions.push_back(action);

				curr_obs.push_back(problem->get_observations());
			}

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}
		for (int sample_index = num_sequence; sample_index < SAMPLES_PER_EPOCH; sample_index++) {
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

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}

		train_helper(wrapper);
	}
	{
		for (int sample_index = 0; sample_index < num_sequence; sample_index++) {
			Problem* problem = problem_type->get_problem();

			vector<vector<double>> curr_obs;
			vector<int> curr_actions;

			curr_obs.push_back(problem->get_observations());

			int num_actions = 1;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);
				curr_actions.push_back(action);

				curr_obs.push_back(problem->get_observations());
			}

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}
		for (int sample_index = num_sequence; sample_index < 2*num_sequence; sample_index++) {
			Problem* problem = problem_type->get_problem();

			vector<vector<double>> curr_obs;
			vector<int> curr_actions;

			curr_obs.push_back(problem->get_observations());

			int num_actions = 2;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);
				curr_actions.push_back(action);

				curr_obs.push_back(problem->get_observations());
			}

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}
		for (int sample_index = 2*num_sequence; sample_index < SAMPLES_PER_EPOCH; sample_index++) {
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

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}

		train_helper(wrapper);
	}
	{
		for (int sample_index = 0; sample_index < num_sequence; sample_index++) {
			Problem* problem = problem_type->get_problem();

			vector<vector<double>> curr_obs;
			vector<int> curr_actions;

			curr_obs.push_back(problem->get_observations());

			int num_actions = 2;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);
				curr_actions.push_back(action);

				curr_obs.push_back(problem->get_observations());
			}

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}
		for (int sample_index = num_sequence; sample_index < SAMPLES_PER_EPOCH; sample_index++) {
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

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}

		train_helper(wrapper);
	}
	{
		for (int sample_index = 0; sample_index < num_sequence; sample_index++) {
			Problem* problem = problem_type->get_problem();

			vector<vector<double>> curr_obs;
			vector<int> curr_actions;

			curr_obs.push_back(problem->get_observations());

			int num_actions = 2;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);
				curr_actions.push_back(action);

				curr_obs.push_back(problem->get_observations());
			}

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}
		for (int sample_index = num_sequence; sample_index < 2*num_sequence; sample_index++) {
			Problem* problem = problem_type->get_problem();

			vector<vector<double>> curr_obs;
			vector<int> curr_actions;

			curr_obs.push_back(problem->get_observations());

			int num_actions = 3;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);
				curr_actions.push_back(action);

				curr_obs.push_back(problem->get_observations());
			}

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}
		for (int sample_index = 2*num_sequence; sample_index < SAMPLES_PER_EPOCH; sample_index++) {
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

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}

		train_helper(wrapper);
	}
	{
		for (int sample_index = 0; sample_index < num_sequence; sample_index++) {
			Problem* problem = problem_type->get_problem();

			vector<vector<double>> curr_obs;
			vector<int> curr_actions;

			curr_obs.push_back(problem->get_observations());

			int num_actions = 3;
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);
				curr_actions.push_back(action);

				curr_obs.push_back(problem->get_observations());
			}

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}
		for (int sample_index = num_sequence; sample_index < SAMPLES_PER_EPOCH; sample_index++) {
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

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}

		train_helper(wrapper);
	}
	{
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

			double predicted_score;
			double predicted_misguess;
			eval_world_model_helper(curr_obs,
									curr_actions,
									predicted_score,
									predicted_misguess,
									wrapper);

			wrapper->sample_obs.push_back(curr_obs);
			wrapper->sample_actions.push_back(curr_actions);
			wrapper->sample_target_vals.push_back(problem->score_result());
			wrapper->sample_predicted_scores.push_back(predicted_score);
			wrapper->sample_predicted_misguesses.push_back(predicted_misguess);

			delete problem;
		}

		train_helper(wrapper);
	}

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
