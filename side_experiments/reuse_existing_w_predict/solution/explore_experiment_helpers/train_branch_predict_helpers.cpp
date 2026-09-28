#include "explore_experiment.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "score_network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_BRANCH_PREDICT_ITERS = 30;
#else
const int TRAIN_BRANCH_PREDICT_ITERS = 100000;
#endif /* MDEBUG */

ScoreNetwork* ExploreExperiment::train_branch_predict_helper() {
	vector<Eigen::VectorXf> state_histories;
	vector<double> target_val_histories;
	for (int h_index = 0; h_index < (int)this->existing_obs_histories.size(); h_index++) {
		for (int i_index = 0; i_index < (int)this->existing_obs_histories[h_index].size(); i_index++) {
			state_histories.push_back(this->existing_state_histories[h_index][i_index]);

			this->existing_network->activate(this->existing_obs_histories[h_index][i_index]);
			double existing_predicted = this->existing_network->output->acti_vals[0];
			this->new_network->activate(this->existing_obs_histories[h_index][i_index]);
			double new_predicted = this->new_network->output->acti_vals[0];

			if (new_predicted >= existing_predicted) {
				target_val_histories.push_back(1.0);
			} else {
				target_val_histories.push_back(-1.0);
			}
		}
	}

	ScoreNetwork* new_branch_predict_network = new ScoreNetwork(NUM_STATES);
	double hidden_1_average_max_update = 0.0;
	double hidden_2_average_max_update = 0.0;
	double output_average_max_update = 0.0;

	uniform_int_distribution<int> distribution(0, state_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_BRANCH_PREDICT_ITERS; iter_index++) {
		int index = distribution(generator);

		new_branch_predict_network->activate(state_histories[index]);

		new_branch_predict_network->init_backprop(target_val_histories[index],
												  hidden_1_average_max_update,
												  hidden_2_average_max_update,
												  output_average_max_update);
	}
	new_branch_predict_network->state_input->errors.setConstant(0.0);

	return new_branch_predict_network;
}
