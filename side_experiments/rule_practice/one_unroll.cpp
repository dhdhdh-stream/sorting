// - movement is necessary to get to the right spot, but it's not the difference?

// - what is good and what is bad depends on the random solutions being performed
//  - so always solution dependent?

// - need to improve random, but keep variance and randomness
//   - look for simple possibilities to trim out

// - perhaps start with all n-grams of a bad sequence
//   - then gradually filter out n-grams that appear in good/neutrl sequences

// - filter doesn't have to be absolute?
//   - just has to improve neutral?

// - filter patterns instead of scoring everything?
//   - so still choose randomly among rest

#include <iostream>

#include "globals.h"
#include "network.h"
#include "simple_problem.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeSimpleProblem();

	Network* network = new Network(1, 1, 3);

	uniform_int_distribution<int> length_distribution(1, 49);
	uniform_int_distribution<int> action_distribution(0, 2);
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		Problem* problem = problem_type->get_problem();

		int length = length_distribution(generator);

		vector<vector<double>> obs_history;
		vector<int> move_history;

		for (int l_index = 0; l_index < length; l_index++) {
			vector<double> obs = problem->get_observations();
			obs_history.push_back(obs);

			int move = action_distribution(generator);
			problem->perform_action(Action(move));
			move_history.push_back(move);
		}

		double sum_predicted_scores = 0.0;
		vector<NetworkHistory*> network_histories;
		for (int l_index = 0; l_index < length; l_index++) {
			vector<vector<double>> curr_obs_vals;
			vector<int> curr_moves;
			curr_obs_vals.push_back(obs_history[l_index]);
			curr_moves.push_back(move_history[l_index]);

			NetworkHistory* network_history = new NetworkHistory();

			network->activate(curr_obs_vals,
							  curr_moves,
							  network_history);
			sum_predicted_scores += network->output->acti_vals[0];

			network_histories.push_back(network_history);
		}

		double target_val = problem->score_result();
		double error = target_val - sum_predicted_scores / length;
		for (int l_index = 0; l_index < length; l_index++) {
			network->backprop(error,
							  network_histories[l_index]);
			delete network_histories[l_index];
		}

		delete problem;
	}

	for (int measure_index = 0; measure_index < 10; measure_index++) {
		cout << "measure_index: " << measure_index << endl;

		Problem* problem = problem_type->get_problem();

		int length = length_distribution(generator);

		vector<vector<double>> obs_history;
		vector<int> move_history;

		double sum_predicted_scores = 0.0;
		for (int l_index = 0; l_index < length; l_index++) {
			cout << l_index << endl;

			vector<double> obs = problem->get_observations();
			obs_history.push_back(obs);

			int move = action_distribution(generator);
			problem->perform_action(Action(move));
			move_history.push_back(move);

			vector<vector<double>> curr_obs_vals;
			vector<int> curr_moves;
			curr_obs_vals.push_back(obs_history[l_index]);
			curr_moves.push_back(move_history[l_index]);

			cout << "obs[0]: " << obs[0] << endl;
			cout << "move: " << move << endl;
			problem->print();

			network->activate(curr_obs_vals,
							  curr_moves);
			cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
			sum_predicted_scores += network->output->acti_vals[0];
		}

		double target_val = problem->score_result();
		cout << "target_val: " << target_val << endl;
		cout << "length: " << length << endl;
		cout << "sum_predicted_scores: " << sum_predicted_scores << endl;

		cout << endl;

		delete problem;
	}

	delete problem_type;

	cout << "Done" << endl;
}
