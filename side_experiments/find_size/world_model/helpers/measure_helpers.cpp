#include "world_model_helpers.h"

#include <iostream>

#include "network.h"
#include "world_model.h"

using namespace std;

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

void measure_test(WorldModel* world_model) {
	{
		vector<double> state(world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R1.size(); a_index++) {
			obs_helper(TEST_OBS_R1[a_index],
					   state,
					   world_model);

			action_helper(TEST_ACTIONS_R1[a_index],
						  state,
						  world_model);
		}

		obs_helper(TEST_OBS_R1.back(),
				   state,
				   world_model);

		world_model->score_network->activate(state);

		cout << "R1 sum_score: " << world_model->score_network->output->acti_vals[0] << endl;
	}

	{
		vector<double> state(world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R2.size(); a_index++) {
			obs_helper(TEST_OBS_R2[a_index],
					   state,
					   world_model);

			action_helper(TEST_ACTIONS_R2[a_index],
						  state,
						  world_model);
		}

		obs_helper(TEST_OBS_R2.back(),
				   state,
				   world_model);

		world_model->score_network->activate(state);

		cout << "R2 sum_score: " << world_model->score_network->output->acti_vals[0] << endl;
	}

	{
		vector<double> state(world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R3.size(); a_index++) {
			obs_helper(TEST_OBS_R3[a_index],
					   state,
					   world_model);

			action_helper(TEST_ACTIONS_R3[a_index],
						  state,
						  world_model);
		}

		obs_helper(TEST_OBS_R3.back(),
				   state,
				   world_model);

		world_model->score_network->activate(state);

		cout << "R3 sum_score: " << world_model->score_network->output->acti_vals[0] << endl;
	}

	{
		vector<double> state(world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R4.size(); a_index++) {
			obs_helper(TEST_OBS_R4[a_index],
					   state,
					   world_model);

			action_helper(TEST_ACTIONS_R4[a_index],
						  state,
						  world_model);
		}

		obs_helper(TEST_OBS_R4.back(),
				   state,
				   world_model);

		world_model->score_network->activate(state);

		cout << "R4 sum_score: " << world_model->score_network->output->acti_vals[0] << endl;
	}

	{
		vector<double> state(world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R5.size(); a_index++) {
			obs_helper(TEST_OBS_R5[a_index],
					   state,
					   world_model);

			action_helper(TEST_ACTIONS_R5[a_index],
						  state,
						  world_model);
		}

		obs_helper(TEST_OBS_R5.back(),
				   state,
				   world_model);

		world_model->score_network->activate(state);

		cout << "R5 sum_score: " << world_model->score_network->output->acti_vals[0] << endl;
	}

	{
		vector<double> state(world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R6.size(); a_index++) {
			obs_helper(TEST_OBS_R6[a_index],
					   state,
					   world_model);

			action_helper(TEST_ACTIONS_R6[a_index],
						  state,
						  world_model);
		}

		obs_helper(TEST_OBS_R6.back(),
				   state,
				   world_model);

		world_model->score_network->activate(state);

		cout << "R6 sum_score: " << world_model->score_network->output->acti_vals[0] << endl;
	}

	{
		vector<double> state(world_model->num_states, 0.0);
		for (int a_index = 0; a_index < (int)TEST_ACTIONS_R7.size(); a_index++) {
			obs_helper(TEST_OBS_R7[a_index],
					   state,
					   world_model);

			action_helper(TEST_ACTIONS_R7[a_index],
						  state,
						  world_model);
		}

		obs_helper(TEST_OBS_R7.back(),
				   state,
				   world_model);

		world_model->score_network->activate(state);

		cout << "R7 sum_score: " << world_model->score_network->output->acti_vals[0] << endl;
	}
}
