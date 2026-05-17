#include "world_model_helpers.h"

#include "globals.h"
#include "world_model.h"
#include "world_model_wrapper.h"

using namespace std;

const int INIT_NUM_TRIES = 50;

void init_run(Run& run,
			  WorldModelWrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	run.state = vector<double>(world_model->num_states, 0.0);

	vector<int> best_commit;
	vector<int> best_return;
	double best_predicted = numeric_limits<double>::lowest();

	geometric_distribution<int> num_actions_distribution(0.3);
	uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);
	for (int try_index = 0; try_index < INIT_NUM_TRIES; try_index++) {
		int num_commit_actions = 1 + num_actions_distribution(generator);
		vector<int> potential_commit;
		for (int a_index = 0; a_index < num_commit_actions; a_index++) {
			potential_commit.push_back(action_distribution(generator));
		}
		int num_return_actions = num_actions_distribution(generator);
		vector<int> potential_return;
		for (int a_index = 0; a_index < num_return_actions; a_index++) {
			potential_return.push_back(action_distribution(generator));
		}

		vector<int> combined_potential;
		combined_potential.insert(combined_potential.end(), potential_commit.begin(), potential_commit.end());
		combined_potential.insert(combined_potential.end(), potential_return.begin(), potential_return.end());

		double potential_predicted = predict_helper(run.state,
													combined_potential,
													wrapper);
		if (potential_predicted > best_predicted) {
			best_commit = potential_commit;
			best_return = potential_return;
			best_predicted = potential_predicted;
		}
	}

	run.commit = best_commit;
	run.commit_index = 0;
	run.curr_return = best_return;
}
