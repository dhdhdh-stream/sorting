#include "world_model_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "solution_helpers.h"
#include "test_indirect.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

vector<int> FORCE_ACTIONS_R1{1};
vector<int> FORCE_ACTIONS_R2{1, 1};
vector<int> FORCE_ACTIONS_R3{1, 1, 0};
vector<int> FORCE_ACTIONS_R4{0, 1, 0};

void force_sequence_helper(Wrapper* wrapper) {
	ProblemType* problem_type = new TypeTestIndirect();

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);

	init_helper(problem_type,
				wrapper);
	train_helper(wrapper);

	cout << "wrapper->world_model->average_max_update: " << wrapper->world_model->average_max_update << endl;

	measure_test(wrapper);

	{
		for (int iter_index = 0; iter_index < SAMPLES_PER_TRAIN; iter_index++) {
			{
				Problem* problem = problem_type->get_problem();

				vector<vector<double>> curr_obs;
				vector<int> curr_actions;

				curr_obs.push_back(problem->get_observations());

				for (int a_index = 0; a_index < (int)FORCE_ACTIONS_R1.size(); a_index++) {
					problem->perform_action(FORCE_ACTIONS_R1[a_index]);
					curr_actions.push_back(FORCE_ACTIONS_R1[a_index]);

					curr_obs.push_back(problem->get_observations());
				}

				wrapper->sample_obs.push_back(curr_obs);
				wrapper->sample_actions.push_back(curr_actions);
				wrapper->sample_target_vals.push_back(problem->score_result());

				delete problem;
			}

			{
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

				wrapper->sample_obs.push_back(curr_obs);
				wrapper->sample_actions.push_back(curr_actions);
				wrapper->sample_target_vals.push_back(problem->score_result());

				delete problem;
			}
		}

		train_helper(wrapper);
	}

	cout << "wrapper->world_model->average_max_update: " << wrapper->world_model->average_max_update << endl;

	measure_test(wrapper);

	// for (int iter_index = 0; iter_index < 10000; iter_index++) {
	// 	Problem* problem = problem_type->get_problem();

	// 	vector<vector<double>> curr_obs;
	// 	vector<int> curr_actions;

	// 	curr_obs.push_back(problem->get_observations());

	// 	int num_actions = num_actions_distribution(generator);
	// 	for (int a_index = 0; a_index < num_actions; a_index++) {
	// 		int action = action_distribution(generator);

	// 		problem->perform_action(action);
	// 		curr_actions.push_back(action);

	// 		curr_obs.push_back(problem->get_observations());
	// 	}

	// 	update_world_model_helper(curr_obs,
	// 							  curr_actions,
	// 							  problem->score_result(),
	// 							  wrapper);

	// 	delete problem;
	// }

	cout << "wrapper->world_model->average_max_update: " << wrapper->world_model->average_max_update << endl;

	measure_test(wrapper);

	{
		for (int iter_index = 0; iter_index < SAMPLES_PER_TRAIN; iter_index++) {
			{
				Problem* problem = problem_type->get_problem();

				vector<vector<double>> curr_obs;
				vector<int> curr_actions;

				curr_obs.push_back(problem->get_observations());

				for (int a_index = 0; a_index < (int)FORCE_ACTIONS_R2.size(); a_index++) {
					problem->perform_action(FORCE_ACTIONS_R2[a_index]);
					curr_actions.push_back(FORCE_ACTIONS_R2[a_index]);

					curr_obs.push_back(problem->get_observations());
				}

				wrapper->sample_obs.push_back(curr_obs);
				wrapper->sample_actions.push_back(curr_actions);
				wrapper->sample_target_vals.push_back(problem->score_result());

				delete problem;
			}

			{
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

				wrapper->sample_obs.push_back(curr_obs);
				wrapper->sample_actions.push_back(curr_actions);
				wrapper->sample_target_vals.push_back(problem->score_result());

				delete problem;
			}
		}

		train_helper(wrapper);
	}

	cout << "wrapper->world_model->average_max_update: " << wrapper->world_model->average_max_update << endl;

	measure_test(wrapper);

	// for (int iter_index = 0; iter_index < 10000; iter_index++) {
	// 	Problem* problem = problem_type->get_problem();

	// 	vector<vector<double>> curr_obs;
	// 	vector<int> curr_actions;

	// 	curr_obs.push_back(problem->get_observations());

	// 	int num_actions = num_actions_distribution(generator);
	// 	for (int a_index = 0; a_index < num_actions; a_index++) {
	// 		int action = action_distribution(generator);

	// 		problem->perform_action(action);
	// 		curr_actions.push_back(action);

	// 		curr_obs.push_back(problem->get_observations());
	// 	}

	// 	update_world_model_helper(curr_obs,
	// 							  curr_actions,
	// 							  problem->score_result(),
	// 							  wrapper);

	// 	delete problem;
	// }

	cout << "wrapper->world_model->average_max_update: " << wrapper->world_model->average_max_update << endl;

	measure_test(wrapper);

	{
		for (int iter_index = 0; iter_index < SAMPLES_PER_TRAIN; iter_index++) {
			{
				Problem* problem = problem_type->get_problem();

				vector<vector<double>> curr_obs;
				vector<int> curr_actions;

				curr_obs.push_back(problem->get_observations());

				for (int a_index = 0; a_index < (int)FORCE_ACTIONS_R3.size(); a_index++) {
					problem->perform_action(FORCE_ACTIONS_R3[a_index]);
					curr_actions.push_back(FORCE_ACTIONS_R3[a_index]);

					curr_obs.push_back(problem->get_observations());
				}

				wrapper->sample_obs.push_back(curr_obs);
				wrapper->sample_actions.push_back(curr_actions);
				wrapper->sample_target_vals.push_back(problem->score_result());

				delete problem;
			}

			{
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

				wrapper->sample_obs.push_back(curr_obs);
				wrapper->sample_actions.push_back(curr_actions);
				wrapper->sample_target_vals.push_back(problem->score_result());

				delete problem;
			}
		}

		train_helper(wrapper);
	}

	cout << "wrapper->world_model->average_max_update: " << wrapper->world_model->average_max_update << endl;

	measure_test(wrapper);

	// for (int iter_index = 0; iter_index < 10000; iter_index++) {
	// 	Problem* problem = problem_type->get_problem();

	// 	vector<vector<double>> curr_obs;
	// 	vector<int> curr_actions;

	// 	curr_obs.push_back(problem->get_observations());

	// 	int num_actions = num_actions_distribution(generator);
	// 	for (int a_index = 0; a_index < num_actions; a_index++) {
	// 		int action = action_distribution(generator);

	// 		problem->perform_action(action);
	// 		curr_actions.push_back(action);

	// 		curr_obs.push_back(problem->get_observations());
	// 	}

	// 	update_world_model_helper(curr_obs,
	// 							  curr_actions,
	// 							  problem->score_result(),
	// 							  wrapper);

	// 	delete problem;
	// }

	cout << "wrapper->world_model->average_max_update: " << wrapper->world_model->average_max_update << endl;

	measure_test(wrapper);

	{
		for (int iter_index = 0; iter_index < SAMPLES_PER_TRAIN; iter_index++) {
			{
				Problem* problem = problem_type->get_problem();

				vector<vector<double>> curr_obs;
				vector<int> curr_actions;

				curr_obs.push_back(problem->get_observations());

				for (int a_index = 0; a_index < (int)FORCE_ACTIONS_R4.size(); a_index++) {
					problem->perform_action(FORCE_ACTIONS_R4[a_index]);
					curr_actions.push_back(FORCE_ACTIONS_R4[a_index]);

					curr_obs.push_back(problem->get_observations());
				}

				wrapper->sample_obs.push_back(curr_obs);
				wrapper->sample_actions.push_back(curr_actions);
				wrapper->sample_target_vals.push_back(problem->score_result());

				delete problem;
			}

			{
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

				wrapper->sample_obs.push_back(curr_obs);
				wrapper->sample_actions.push_back(curr_actions);
				wrapper->sample_target_vals.push_back(problem->score_result());

				delete problem;
			}
		}

		train_helper(wrapper);
	}

	cout << "wrapper->world_model->average_max_update: " << wrapper->world_model->average_max_update << endl;

	measure_test(wrapper);

	for (int iter_index = 0; iter_index < 10000; iter_index++) {
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

		update_world_model_helper(curr_obs,
								  curr_actions,
								  problem->score_result(),
								  wrapper);

		delete problem;
	}

	cout << "wrapper->world_model->average_max_update: " << wrapper->world_model->average_max_update << endl;

	measure_test(wrapper);

	delete problem_type;
}
