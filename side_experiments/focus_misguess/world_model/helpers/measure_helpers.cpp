#include "world_model_helpers.h"

#include <iostream>

#include "network.h"
#include "world_model.h"
#include "wrapper.h"

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
		for (int n_index = 0; n_index < (int)wrapper->world_model->score_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->score_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->score_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->score_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->score_networks[n_index]->output->acti_vals[0];
		}

		double sum_misguess = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->misguess_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->misguess_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->misguess_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->misguess_networks[n_index]->activate(inputs);
			sum_misguess += wrapper->world_model->misguess_networks[n_index]->output->acti_vals[0];
		}

		cout << "R1 sum_score: " << sum_score << endl;
		cout << "R1 sum_misguess: " << sum_misguess << endl;
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
		for (int n_index = 0; n_index < (int)wrapper->world_model->score_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->score_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->score_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->score_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->score_networks[n_index]->output->acti_vals[0];
		}

		double sum_misguess = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->misguess_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->misguess_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->misguess_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->misguess_networks[n_index]->activate(inputs);
			sum_misguess += wrapper->world_model->misguess_networks[n_index]->output->acti_vals[0];
		}

		cout << "R2 sum_score: " << sum_score << endl;
		cout << "R2 sum_misguess: " << sum_misguess << endl;
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
		for (int n_index = 0; n_index < (int)wrapper->world_model->score_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->score_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->score_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->score_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->score_networks[n_index]->output->acti_vals[0];
		}

		double sum_misguess = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->misguess_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->misguess_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->misguess_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->misguess_networks[n_index]->activate(inputs);
			sum_misguess += wrapper->world_model->misguess_networks[n_index]->output->acti_vals[0];
		}

		cout << "R3 sum_score: " << sum_score << endl;
		cout << "R3 sum_misguess: " << sum_misguess << endl;
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
		for (int n_index = 0; n_index < (int)wrapper->world_model->score_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->score_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->score_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->score_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->score_networks[n_index]->output->acti_vals[0];
		}

		double sum_misguess = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->misguess_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->misguess_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->misguess_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->misguess_networks[n_index]->activate(inputs);
			sum_misguess += wrapper->world_model->misguess_networks[n_index]->output->acti_vals[0];
		}

		cout << "R4 sum_score: " << sum_score << endl;
		cout << "R4 sum_misguess: " << sum_misguess << endl;
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
		for (int n_index = 0; n_index < (int)wrapper->world_model->score_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->score_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->score_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->score_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->score_networks[n_index]->output->acti_vals[0];
		}

		double sum_misguess = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->misguess_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->misguess_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->misguess_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->misguess_networks[n_index]->activate(inputs);
			sum_misguess += wrapper->world_model->misguess_networks[n_index]->output->acti_vals[0];
		}

		cout << "R5 sum_score: " << sum_score << endl;
		cout << "R5 sum_misguess: " << sum_misguess << endl;
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
		for (int n_index = 0; n_index < (int)wrapper->world_model->score_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->score_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->score_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->score_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->score_networks[n_index]->output->acti_vals[0];
		}

		double sum_misguess = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->misguess_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->misguess_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->misguess_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->misguess_networks[n_index]->activate(inputs);
			sum_misguess += wrapper->world_model->misguess_networks[n_index]->output->acti_vals[0];
		}

		cout << "R6 sum_score: " << sum_score << endl;
		cout << "R6 sum_misguess: " << sum_misguess << endl;
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
		for (int n_index = 0; n_index < (int)wrapper->world_model->score_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->score_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->score_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->score_networks[n_index]->activate(inputs);
			sum_score += wrapper->world_model->score_networks[n_index]->output->acti_vals[0];
		}

		double sum_misguess = 0.0;
		for (int n_index = 0; n_index < (int)wrapper->world_model->misguess_networks.size(); n_index++) {
			vector<double> inputs;
			for (int i_index = 0; i_index < (int)wrapper->world_model->misguess_network_inputs[n_index].size(); i_index++) {
				inputs.push_back(state[wrapper->world_model->misguess_network_inputs[n_index][i_index]]);
			}
			wrapper->world_model->misguess_networks[n_index]->activate(inputs);
			sum_misguess += wrapper->world_model->misguess_networks[n_index]->output->acti_vals[0];
		}

		cout << "R7 sum_score: " << sum_score << endl;
		cout << "R7 sum_misguess: " << sum_misguess << endl;
	}
}
