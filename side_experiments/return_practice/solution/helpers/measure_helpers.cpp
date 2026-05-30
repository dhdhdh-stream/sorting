#include "solution_helpers.h"

#include <iostream>

#include "network.h"
#include "run.h"
#include "test_indirect.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 40;
#else
const int MEASURE_NUM_ITERS = 4000;
#endif /* MDEBUG */

void measure_helper(Wrapper* wrapper,
					double& score_average,
					double& misguess_average) {
	ProblemType* problem_type = new TypeTestIndirect();

	double sum_scores = 0.0;
	double sum_misguess = 0.0;
	for (int iter_index = 0; iter_index < MEASURE_NUM_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		Run* run = new Run();

		wrapper->init(run);

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = wrapper->step(obs,
												run);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		sum_scores += target_val;

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(run->state[wrapper->world_model->final_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->final_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->final_networks[n_index]->output->acti_vals[0];
		}
		sum_misguess += (target_val - sum_score) * (target_val - sum_score);

		delete run;

		delete problem;
	}

	score_average = sum_scores / MEASURE_NUM_ITERS;
	misguess_average = sum_misguess / MEASURE_NUM_ITERS;;

	delete problem_type;
}

vector<vector<double>> TEST_OBS_R1{
	vector<double>{0.0},
	vector<double>{0.0}
};
vector<int> TEST_ACTIONS_R1{1};

vector<vector<double>> TEST_OBS_R2{
	vector<double>{1.0},
	vector<double>{0.0}
};
vector<int> TEST_ACTIONS_R2{1};

vector<vector<double>> TEST_OBS_R3{
	vector<double>{1.0},
	vector<double>{0.0}
};
vector<int> TEST_ACTIONS_R3{0};

vector<vector<double>> TEST_OBS_R4{
	vector<double>{0.0},
};
vector<int> TEST_ACTIONS_R4{};

vector<vector<double>> TEST_OBS_R5{
	vector<double>{0.0},
	vector<double>{0.0},
	vector<double>{0.0}
};
vector<int> TEST_ACTIONS_R5{1, 1};

vector<vector<double>> TEST_OBS_R6{
	vector<double>{0.0},
	vector<double>{0.0},
	vector<double>{0.0},
	vector<double>{0.0}
};
vector<int> TEST_ACTIONS_R6{1, 1, 0};

vector<vector<double>> TEST_OBS_R7{
	vector<double>{1.0},
	vector<double>{0.0},
	vector<double>{0.0},
	vector<double>{0.0}
};
vector<int> TEST_ACTIONS_R7{0, 1, 0};

void measure_test(Wrapper* wrapper) {
	{
		vector<double> state(wrapper->world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R1.size(); a_index++) {
			obs_helper(TEST_OBS_R1[a_index],
					   state,
					   wrapper);

			action_helper(TEST_ACTIONS_R1[a_index],
						  state,
						  wrapper);
		}

		obs_helper(TEST_OBS_R1.back(),
				   state,
				   wrapper);

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->final_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->final_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->final_networks[n_index]->output->acti_vals[0];
		}

		cout << "R1 predicted: " << sum_score << endl;
	}

	{
		vector<double> state(wrapper->world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R2.size(); a_index++) {
			obs_helper(TEST_OBS_R2[a_index],
					   state,
					   wrapper);

			action_helper(TEST_ACTIONS_R2[a_index],
						  state,
						  wrapper);
		}

		obs_helper(TEST_OBS_R2.back(),
				   state,
				   wrapper);

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->final_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->final_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->final_networks[n_index]->output->acti_vals[0];
		}

		cout << "R2 predicted: " << sum_score << endl;
	}

	{
		vector<double> state(wrapper->world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R3.size(); a_index++) {
			obs_helper(TEST_OBS_R3[a_index],
					   state,
					   wrapper);

			action_helper(TEST_ACTIONS_R3[a_index],
						  state,
						  wrapper);
		}

		obs_helper(TEST_OBS_R3.back(),
				   state,
				   wrapper);

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->final_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->final_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->final_networks[n_index]->output->acti_vals[0];
		}

		cout << "R3 predicted: " << sum_score << endl;
	}

	{
		vector<double> state(wrapper->world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R4.size(); a_index++) {
			obs_helper(TEST_OBS_R4[a_index],
					   state,
					   wrapper);

			action_helper(TEST_ACTIONS_R4[a_index],
						  state,
						  wrapper);
		}

		obs_helper(TEST_OBS_R4.back(),
				   state,
				   wrapper);

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->final_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->final_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->final_networks[n_index]->output->acti_vals[0];
		}

		cout << "R4 predicted: " << sum_score << endl;
	}

	{
		vector<double> state(wrapper->world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R5.size(); a_index++) {
			obs_helper(TEST_OBS_R5[a_index],
					   state,
					   wrapper);

			action_helper(TEST_ACTIONS_R5[a_index],
						  state,
						  wrapper);
		}

		obs_helper(TEST_OBS_R5.back(),
				   state,
				   wrapper);

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->final_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->final_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->final_networks[n_index]->output->acti_vals[0];
		}

		cout << "R5 predicted: " << sum_score << endl;
	}

	{
		vector<double> state(wrapper->world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R6.size(); a_index++) {
			obs_helper(TEST_OBS_R6[a_index],
					   state,
					   wrapper);

			action_helper(TEST_ACTIONS_R6[a_index],
						  state,
						  wrapper);
		}

		obs_helper(TEST_OBS_R6.back(),
				   state,
				   wrapper);

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->final_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->final_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->final_networks[n_index]->output->acti_vals[0];
		}

		cout << "R6 predicted: " << sum_score << endl;
	}

	{
		vector<double> state(wrapper->world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R7.size(); a_index++) {
			obs_helper(TEST_OBS_R7[a_index],
					   state,
					   wrapper);

			action_helper(TEST_ACTIONS_R7[a_index],
						  state,
						  wrapper);
		}

		obs_helper(TEST_OBS_R7.back(),
				   state,
				   wrapper);

		double sum_score = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->final_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->final_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->final_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->final_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->final_networks[n_index]->output->acti_vals[0];
		}

		cout << "R7 predicted: " << sum_score << endl;
	}
}
