#include "markov_experiment.h"

#include <algorithm>

#include "globals.h"
#include "markov_node.h"
#include "minesweeper.h"
#include "network.h"
#include "problem.h"
#include "scope.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_EPOCHS = 2;
const int NUM_SAMPLES = 10;
const int TRAIN_ITERS = 200;
#else
const int NUM_EPOCHS = 15;
const int NUM_SAMPLES = 1000;
const int TRAIN_ITERS = 200000;
#endif /* MDEBUG */

void MarkovExperiment::train_activate(AbstractNode*& curr_node,
									  Problem* problem,
									  vector<ContextLayer>& context,
									  RunHelper& run_helper,
									  MarkovExperimentHistory* history) {
	Minesweeper* minesweeper = (Minesweeper*)problem;

	/**
	 * - occasional random is better than temperature
	 *   - explore should be evaluated given other actions are correct
	 *     - vs. the random to random from temperature
	 */
	uniform_int_distribution<int> random_distribution(0, 4);
	uniform_int_distribution<int> random_index_distribution(-1, this->step_types.size()-1);

	int iter_index = 0;
	while (true) {
		run_helper.num_analyze += (1 + (int)this->step_types.size()) * (1 + 2*MARKOV_NODE_ANALYZE_SIZE) * (1 + 2*MARKOV_NODE_ANALYZE_SIZE);

		vector<vector<double>> input_vals(1 + 2*MARKOV_NODE_ANALYZE_SIZE);
		for (int x_index = 0; x_index < 1 + 2*MARKOV_NODE_ANALYZE_SIZE; x_index++) {
			input_vals[x_index] = vector<double>(1 + 2*MARKOV_NODE_ANALYZE_SIZE);
		}
		for (int x_index = -MARKOV_NODE_ANALYZE_SIZE; x_index <= MARKOV_NODE_ANALYZE_SIZE; x_index++) {
			for (int y_index = -MARKOV_NODE_ANALYZE_SIZE; y_index <= MARKOV_NODE_ANALYZE_SIZE; y_index++) {
				input_vals[x_index + MARKOV_NODE_ANALYZE_SIZE][y_index + MARKOV_NODE_ANALYZE_SIZE]
					= minesweeper->get_observation_helper(
						minesweeper->current_x + x_index,
						minesweeper->current_y + y_index);
			}
		}

		int best_index;
		if (this->state_iter == 0
				|| random_distribution(generator) == 0) {
			best_index = random_index_distribution(generator);
		} else {
			this->halt_network->activate(input_vals);

			best_index = -1;
			double best_val = this->halt_network->output->acti_vals[0];

			for (int o_index = 0; o_index < (int)this->step_types.size(); o_index++) {
				this->networks[o_index]->activate(input_vals);
				if (this->networks[o_index]->output->acti_vals[0] > best_val) {
					best_index = o_index;
					best_val = this->networks[o_index]->output->acti_vals[0];
				}
			}
		}

		this->option_histories.push_back(best_index);
		this->obs_histories.push_back(input_vals);

		if (best_index == -1) {
			break;
		}

		for (int s_index = 0; s_index < (int)this->step_types[best_index].size(); s_index++) {
			switch (this->step_types[best_index][s_index]) {
			case MARKOV_STEP_TYPE_ACTION:
				problem->perform_action(this->actions[best_index][s_index]);
				break;
			case MARKOV_STEP_TYPE_SCOPE:
				this->scopes[best_index][s_index]->activate(
					problem,
					context,
					run_helper);
				break;
			}

			run_helper.num_actions++;
		}

		iter_index++;
		if (iter_index >= MARKOV_NODE_MAX_ITERS) {
			break;
		}
	}

	curr_node = this->exit_next_node;
}

void MarkovExperiment::train_backprop(
		double target_val,
		RunHelper& run_helper) {
	while (this->target_val_histories.size() < this->option_histories.size()) {
		this->target_val_histories.push_back(target_val);
	}

	this->sub_state_iter++;
	if (this->sub_state_iter >= NUM_SAMPLES) {
		{
			default_random_engine generator_copy = generator;
			shuffle(this->option_histories.begin(), this->option_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->obs_histories.begin(), this->obs_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->target_val_histories.begin(), this->target_val_histories.end(), generator_copy);
		}

		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
			sum_vals += this->target_val_histories[h_index];
		}
		double average_val = sum_vals / (int)this->target_val_histories.size();
		for (int h_index = 0; h_index < (int)this->target_val_histories.size(); h_index++) {
			this->target_val_histories[h_index] -= average_val;
		}

		uniform_int_distribution<int> sample_distribution(0, this->option_histories.size()-1);

		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = sample_distribution(generator);
			if (this->option_histories[rand_index] == -1) {
				this->halt_network->activate(this->obs_histories[rand_index]);
				double error = this->target_val_histories[rand_index] - this->halt_network->output->acti_vals[0];
				this->halt_network->backprop(error);
			} else {
				this->networks[this->option_histories[rand_index]]->activate(this->obs_histories[rand_index]);
				double error = this->target_val_histories[rand_index]
					- this->networks[this->option_histories[rand_index]]->output->acti_vals[0];
				this->networks[this->option_histories[rand_index]]->backprop(error);
			}
		}

		this->option_histories.clear();
		this->obs_histories.clear();
		this->target_val_histories.clear();

		this->state_iter++;
		if (this->state_iter >= NUM_EPOCHS) {
			this->combined_score = 0.0;

			this->state = MARKOV_EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
			this->sub_state_iter = 0;
		} else {
			this->sub_state_iter = 0;
		}
	}
}
