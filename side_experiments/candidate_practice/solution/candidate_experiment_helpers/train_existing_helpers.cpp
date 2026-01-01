#include "candidate_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "tunnel.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_EXISTING_NUM_DATAPOINTS = 20;
#else
const int TRAIN_EXISTING_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void CandidateExperiment::train_existing_check_activate(SolutionWrapper* wrapper) {
	CandidateExperimentHistory* history = (CandidateExperimentHistory*)wrapper->experiment_history;
	history->stack_traces.push_back(wrapper->scope_histories);

	CandidateExperimentState* new_experiment_state = new CandidateExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void CandidateExperiment::train_existing_step(vector<double>& obs,
											  SolutionWrapper* wrapper) {
	this->obs_histories.push_back(obs);

	this->sum_num_instances++;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void CandidateExperiment::train_existing_backprop(double target_val,
												  SolutionWrapper* wrapper) {
	CandidateExperimentHistory* history = (CandidateExperimentHistory*)wrapper->experiment_history;

	if (history->is_hit) {
		this->sum_true += target_val;
		this->hit_count++;

		double sum_vals = 0.0;
		gather_tunnel_data_helper(wrapper->scope_histories[0],
								  sum_vals);
		this->signal_vals.push_back(sum_vals);

		for (int i_index = 0; i_index < (int)history->stack_traces.size(); i_index++) {
			this->true_histories.push_back(target_val);

			double signal = this->candidate->get_signal(history->stack_traces[i_index].back()->obs_history);
			this->signal_histories.push_back(signal);
		}
	}

	if (this->hit_count >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		this->existing_true = this->sum_true / this->hit_count;

		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->signal_vals.size(); h_index++) {
			sum_vals += this->signal_vals[h_index];
		}
		this->candidate_starting_val_average = sum_vals / (double)this->signal_vals.size();

		double sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->signal_vals.size(); h_index++) {
			sum_variance += (this->signal_vals[h_index] - this->candidate_starting_val_average) * (this->signal_vals[h_index] - this->candidate_starting_val_average);
		}
		double candidate_starting_val_standard_deviation = sqrt(sum_variance / (double)this->signal_vals.size());
		if (candidate_starting_val_standard_deviation < MIN_STANDARD_DEVIATION) {
			candidate_starting_val_standard_deviation = MIN_STANDARD_DEVIATION;
		}
		this->candidate_starting_val_standard_error = candidate_starting_val_standard_deviation / sqrt((double)this->signal_vals.size());

		this->signal_vals.clear();

		uniform_int_distribution<int> val_input_distribution(0, this->obs_histories.size()-1);

		this->existing_true_network = new Network(this->obs_histories[0].size(),
												  NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = val_input_distribution(generator);

			this->existing_true_network->activate(this->obs_histories[rand_index]);

			double error = this->true_histories[rand_index] - this->existing_true_network->output->acti_vals[0];

			this->existing_true_network->backprop(error);
		}

		this->existing_signal_network = new Network(this->obs_histories[0].size(),
													NETWORK_SIZE_SMALL);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = val_input_distribution(generator);

			this->existing_signal_network->activate(this->obs_histories[rand_index]);

			double error = this->signal_histories[rand_index] - this->existing_signal_network->output->acti_vals[0];

			this->existing_signal_network->backprop(error);
		}

		this->obs_histories.clear();
		this->true_histories.clear();
		this->signal_histories.clear();

		this->average_instances_per_run = (double)this->sum_num_instances / (double)this->hit_count;

		this->best_surprise = numeric_limits<double>::lowest();

		uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		this->state = CANDIDATE_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
