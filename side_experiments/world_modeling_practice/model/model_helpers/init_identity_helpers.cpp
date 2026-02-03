/**
 * - depend only on init for predict_networks
 */

#include "model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "problem.h"

using namespace std;

// TODO: misguess over actual variance
// - doesn't matter if distinctive?
//   - not looking for distinct points, but points that can loop on
// - maybe predict without any obs
//   - and that's the variance

// - want recognition to not work elsewhere
// - but also want action sequence to not just be stand in place?
//   - can never truly check for this though?

// - maybe just return sequence has to hit location which is not in place
//   - so full return sequence low, but partial high

void init_recognition_helper(ProblemType* problem_type,
							 vector<int>& init_actions,
							 vector<int>& identity_actions,
							 vector<Network*>& predict_networks,
							 double& identity_misguess,
							 double& explore_misguess) {
	uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);

	init_actions = vector<int>(5);
	for (int i_index = 0; i_index < 5; i_index++) {
		init_actions[i_index] = action_distribution(generator);
	}
	identity_actions = vector<int>(5);
	for (int i_index = 0; i_index < 5; i_index++) {
		identity_actions[i_index] = action_distribution(generator);
	}
	predict_networks = vector<Network*>(5);
	for (int i_index = 0; i_index < 5; i_index++) {
		Network* network = new Network(6);
		predict_networks[i_index] = network;
	}

	vector<vector<double>> init_obs;
	vector<vector<double>> identity_obs;
	for (int t_index = 0; t_index < 4000; t_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> curr_init_obs;
		curr_init_obs.push_back(problem->get_observations()[0]);
		for (int i_index = 0; i_index < 5; i_index++) {
			problem->perform_action(init_actions[i_index]);
			curr_init_obs.push_back(problem->get_observations()[0]);
		}
		init_obs.push_back(curr_init_obs);

		vector<double> curr_identity_obs;
		for (int i_index = 0; i_index < 5; i_index++) {
			problem->perform_action(identity_actions[i_index]);
			curr_identity_obs.push_back(problem->get_observations()[0]);
		}
		identity_obs.push_back(curr_identity_obs);

		delete problem;
	}

	uniform_int_distribution<int> input_distribution(0, 3999);
	for (int i_index = 0; i_index < 5; i_index++) {
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			int index = input_distribution(generator);

			predict_networks[i_index]->activate(init_obs[index]);

			double error = identity_obs[index][i_index] - predict_networks[i_index]->output->acti_vals[0];

			predict_networks[i_index]->backprop(error);
		}
	}

	cout << "init_actions:";
	for (int a_index = 0; a_index < (int)init_actions.size(); a_index++) {
		cout << " " << init_actions[a_index];
	}
	cout << endl;
	cout << "identity_actions:";
	for (int a_index = 0; a_index < (int)identity_actions.size(); a_index++) {
		cout << " " << identity_actions[a_index];
	}
	cout << endl;

	cout << "identity:" << endl;
	vector<double> sum_identity_misguess(5, 0.0);
	for (int h_index = 0; h_index < 1000; h_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> curr_init_obs;
		curr_init_obs.push_back(problem->get_observations()[0]);
		for (int i_index = 0; i_index < 5; i_index++) {
			problem->perform_action(init_actions[i_index]);
			curr_init_obs.push_back(problem->get_observations()[0]);
		}

		// temp
		if (h_index < 10) {
			cout << "curr_init_obs:";
			for (int i_index = 0; i_index < (int)curr_init_obs.size(); i_index++) {
				cout << " " << curr_init_obs[i_index];
			}
			cout << endl;
		}

		for (int i_index = 0; i_index < 5; i_index++) {
			predict_networks[i_index]->activate(curr_init_obs);

			problem->perform_action(identity_actions[i_index]);

			double obs = problem->get_observations()[0];

			sum_identity_misguess[i_index] += (obs - predict_networks[i_index]->output->acti_vals[0])
				* (obs - predict_networks[i_index]->output->acti_vals[0]);

			// temp
			if (h_index < 10) {
				cout << "predict_networks[i_index]->output->acti_vals[0]: " << predict_networks[i_index]->output->acti_vals[0] << endl;
				cout << obs << endl;
			}
		}

		delete problem;
	}

	double total_sum_identity_misguess = 0.0;
	for (int i_index = 0; i_index < 5; i_index++) {
		total_sum_identity_misguess += sum_identity_misguess[i_index];
	}
	identity_misguess = total_sum_identity_misguess / 1000.0;

	cout << "explore:" << endl;
	vector<double> sum_explore_misguess(5, 0.0);
	for (int h_index = 0; h_index < 1000; h_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> curr_init_obs;
		curr_init_obs.push_back(problem->get_observations()[0]);
		for (int i_index = 0; i_index < 5; i_index++) {
			problem->perform_action(init_actions[i_index]);
			curr_init_obs.push_back(problem->get_observations()[0]);
		}

		// temp
		if (h_index < 10) {
			cout << "curr_init_obs:";
			for (int i_index = 0; i_index < (int)curr_init_obs.size(); i_index++) {
				cout << " " << curr_init_obs[i_index];
			}
			cout << endl;
		}

		for (int random_index = 0; random_index < 20; random_index++) {
			problem->perform_action(action_distribution(generator));
		}

		for (int i_index = 0; i_index < 5; i_index++) {
			predict_networks[i_index]->activate(curr_init_obs);

			problem->perform_action(identity_actions[i_index]);

			double obs = problem->get_observations()[0];

			sum_explore_misguess[i_index] += (obs - predict_networks[i_index]->output->acti_vals[0])
				* (obs - predict_networks[i_index]->output->acti_vals[0]);

			// temp
			if (h_index < 10) {
				cout << "predict_networks[i_index]->output->acti_vals[0]: " << predict_networks[i_index]->output->acti_vals[0] << endl;
				cout << obs << endl;
			}
		}

		delete problem;
	}

	double total_sum_explore_misguess = 0.0;
	for (int i_index = 0; i_index < 5; i_index++) {
		total_sum_explore_misguess += sum_explore_misguess[i_index];
	}
	explore_misguess = total_sum_explore_misguess / 1000.0;

	cout << "sum_identity_misguess:" << endl;
	for (int i_index = 0; i_index < 5; i_index++) {
		cout << i_index << ": " << sum_identity_misguess[i_index] << endl;
	}

	cout << "sum_explore_misguess:" << endl;
	for (int i_index = 0; i_index < 5; i_index++) {
		cout << i_index << ": " << sum_explore_misguess[i_index] << endl;
	}
}

void init_return_helper(ProblemType* problem_type,
						vector<int>& init_actions,
						vector<int>& identity_actions,
						vector<Network*>& predict_networks,
						vector<int>& return_actions) {
	double best_sum_misguess = numeric_limits<double>::max();
	uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
	for (int try_index = 0; try_index < 200; try_index++) {
		vector<int> curr_return_actions;
		for (int i_index = 0; i_index < 4; i_index++) {
			curr_return_actions.push_back(action_distribution(generator));
		}

		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < 1000; iter_index++) {
			Problem* problem = problem_type->get_problem();

			vector<double> curr_init_obs;
			curr_init_obs.push_back(problem->get_observations()[0]);
			for (int i_index = 0; i_index < 5; i_index++) {
				problem->perform_action(init_actions[i_index]);
				curr_init_obs.push_back(problem->get_observations()[0]);
			}

			for (int loop_index = 0; loop_index < 3; loop_index++) {
				double loop_weight = 3.0 - loop_index;

				for (int a_index = 0; a_index < (int)curr_return_actions.size(); a_index++) {
					problem->perform_action(curr_return_actions[a_index]);
				}

				for (int i_index = 0; i_index < 5; i_index++) {
					predict_networks[i_index]->activate(curr_init_obs);

					problem->perform_action(identity_actions[i_index]);

					double obs = problem->get_observations()[0];

					sum_misguess += loop_weight * (obs - predict_networks[i_index]->output->acti_vals[0])
						* (obs - predict_networks[i_index]->output->acti_vals[0]);
				}
			}

			delete problem;
		}

		if (sum_misguess < best_sum_misguess) {
			return_actions = curr_return_actions;

			best_sum_misguess = sum_misguess;
		}
	}
}

void init_return_success_helper(ProblemType* problem_type,
								std::vector<int>& init_actions,
								std::vector<int>& identity_actions,
								std::vector<Network*>& predict_networks,
								std::vector<int>& return_actions,
								Network*& success_network) {
	Network* default_misguess_network = new Network(init_actions.size() + 1);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> curr_init_obs;
		curr_init_obs.push_back(problem->get_observations()[0]);
		for (int i_index = 0; i_index < 5; i_index++) {
			problem->perform_action(init_actions[i_index]);
			curr_init_obs.push_back(problem->get_observations()[0]);
		}

		double sum_misguess = 0.0;
		for (int i_index = 0; i_index < 5; i_index++) {
			predict_networks[i_index]->activate(curr_init_obs);

			problem->perform_action(identity_actions[i_index]);

			double obs = problem->get_observations()[0];

			sum_misguess += (obs - predict_networks[i_index]->output->acti_vals[0])
				* (obs - predict_networks[i_index]->output->acti_vals[0]);
		}

		default_misguess_network->activate(curr_init_obs);

		double error = sum_misguess - default_misguess_network->output->acti_vals[0];

		default_misguess_network->backprop(error);
	}

	success_network = new Network(init_actions.size() + 1 + return_actions.size());

	Network* w_o_return_network = new Network(init_actions.size() + 1);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> curr_init_obs;
		curr_init_obs.push_back(problem->get_observations()[0]);
		for (int i_index = 0; i_index < 5; i_index++) {
			problem->perform_action(init_actions[i_index]);
			curr_init_obs.push_back(problem->get_observations()[0]);
		}

		vector<double> return_obs;
		for (int a_index = 0; a_index < (int)return_actions.size(); a_index++) {
			problem->perform_action(return_actions[a_index]);
			return_obs.push_back(problem->get_observations()[0]);
		}

		double sum_misguess = 0.0;
		for (int i_index = 0; i_index < 5; i_index++) {
			predict_networks[i_index]->activate(curr_init_obs);

			problem->perform_action(identity_actions[i_index]);

			double obs = problem->get_observations()[0];

			sum_misguess += (obs - predict_networks[i_index]->output->acti_vals[0])
				* (obs - predict_networks[i_index]->output->acti_vals[0]);
		}

		vector<double> input;
		input.insert(input.end(), curr_init_obs.begin(), curr_init_obs.end());
		input.insert(input.end(), return_obs.begin(), return_obs.end());

		{
			success_network->activate(input);

			double error = sum_misguess - success_network->output->acti_vals[0];

			success_network->backprop(error);
		}

		{
			w_o_return_network->activate(curr_init_obs);

			double error = sum_misguess - w_o_return_network->output->acti_vals[0];

			w_o_return_network->backprop(error);
		}
	}

	for (int iter_index = 0; iter_index < 20; iter_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> curr_init_obs;
		curr_init_obs.push_back(problem->get_observations()[0]);
		for (int i_index = 0; i_index < 5; i_index++) {
			problem->perform_action(init_actions[i_index]);
			curr_init_obs.push_back(problem->get_observations()[0]);
		}

		vector<double> return_obs;
		for (int a_index = 0; a_index < (int)return_actions.size(); a_index++) {
			problem->perform_action(return_actions[a_index]);
			return_obs.push_back(problem->get_observations()[0]);
		}

		default_misguess_network->activate(curr_init_obs);

		vector<double> input;
		input.insert(input.end(), curr_init_obs.begin(), curr_init_obs.end());
		input.insert(input.end(), return_obs.begin(), return_obs.end());

		success_network->activate(input);

		w_o_return_network->activate(curr_init_obs);

		cout << "input:";
		for (int i_index = 0; i_index < (int)input.size(); i_index++) {
			cout << " " << input[i_index];
		}
		cout << endl;

		cout << "default_misguess_network->output->acti_vals[0]: " << default_misguess_network->output->acti_vals[0] << endl;

		cout << "success_network->output->acti_vals[0]: " << success_network->output->acti_vals[0] << endl;

		cout << "w_o_return_network->output->acti_vals[0]: " << w_o_return_network->output->acti_vals[0] << endl;
	}
}
