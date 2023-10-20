/**
 * - don't worry about getting highest accuracy earliest possible
 *   - benefit to early branching might not even counter benefit to late abstraction
 *   - costs a lot of extra state/runtime
 */

#include "scope.h"

#include <iostream>
#include <queue>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "helpers.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void Scope::update_histories(double target_val,
							 ScopeHistory* history) {
	if (history->exceeded_depth) {
		return;
	}

	ScopeHistory* new_scope_history = new ScopeHistory(history);
	new_scope_history->input_state_snapshots = history->input_state_snapshots;
	new_scope_history->local_state_snapshots = history->local_state_snapshots;
	if (this->scope_histories.size() >= NUM_DATAPOINTS) {
		this->scope_histories.pop_front();
	}
	this->scope_histories.push_back(new_scope_history);

	if (this->target_val_histories.size() >= NUM_DATAPOINTS) {
		this->target_val_histories.pop_front();
	}
	this->target_val_histories.push_back(target_val);
}

void Scope::update() {
	if (this->scope_histories.size() >= NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (list<double>::iterator it = this->target_val_histories.begin();
				it != this->target_val_histories.end(); it++) {
			sum_scores += *it;
		}
		this->average_score = sum_scores/NUM_DATAPOINTS;
		/**
		 * - don't save averages/weights across updates
		 *   - only rely on states to preserve insights across updates
		 */

		double sum_score_variance = 0.0;
		for (list<double>::iterator it = this->target_val_histories.begin();
				it != this->target_val_histories.end(); it++) {
			sum_score_variance += (*it - this->average_score) * (*it - this->average_score);
		}
		this->score_variance = sum_score_variance/NUM_DATAPOINTS;

		MatrixXd state_vals(NUM_DATAPOINTS, this->num_input_states + this->num_local_states);
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			for (int s_index = 0; s_index < this->num_input_states + this->num_local_states; s_index++) {
				state_vals(d_index, s_index) = 0.0;
			}
		}
		{
			int d_index = 0;
			for (list<ScopeHistory*>::iterator it = this->scope_histories.begin();
					it != this->scope_histories.end(); it++) {
				for (map<int, StateStatus>::iterator input_it = (*it)->input_state_snapshots.begin();
						input_it != (*it)->input_state_snapshots.end(); input_it++) {
					StateNetwork* last_network = input_it->second.last_network;
					if (last_network != NULL) {
						double normalized = (input_it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						state_vals(d_index, input_it->first) = normalized;
					} else {
						state_vals(d_index, input_it->first) = input_it->second.val;
					}
				}

				for (map<int, StateStatus>::iterator local_it = (*it)->local_state_snapshots.begin();
						local_it != (*it)->local_state_snapshots.end(); local_it++) {
					StateNetwork* last_network = local_it->second.last_network;
					if (last_network != NULL) {
						double normalized = (local_it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						state_vals(d_index, this->num_input_states + local_it->first) = normalized;
					} else {
						state_vals(d_index, this->num_input_states + local_it->first) = local_it->second.val;
					}
				}

				d_index++;
			}
		}

		VectorXd target_vals(NUM_DATAPOINTS);
		{
			int d_index = 0;
			for (list<double>::iterator it = this->target_val_histories.begin();
					it != this->target_val_histories.end(); it++) {
				target_vals(d_index) = *it - this->average_score;

				d_index++;
			}
		}

		VectorXd weights = state_vals.fullPivHouseholderQr().solve(target_vals);
		for (int s_index = 0; s_index < this->num_input_states; s_index++) {
			this->input_state_weights[s_index] = weights(s_index);
		}
		for (int s_index = 0; s_index < this->num_local_states; s_index++) {
			this->local_state_weights[s_index] = weights(this->num_input_states + s_index);
		}

		VectorXd predicted_scores = state_vals * weights;
		VectorXd v_diffs = target_vals - predicted_scores;
		vector<double> diffs(NUM_DATAPOINTS);
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			diffs[d_index] = v_diffs(d_index);
		}

		while (true) {
			ObsExperiment* obs_experiment = create_obs_experiment(this->scope_histories.back());
			/**
			 * - simply always use most recent run
			 */

			obs_experiment->experiment(this->scope_histories,
									   diffs);
			bool is_success = obs_experiment->scope_eval(this);

			delete obs_experiment;

			if (!is_success) {
				break;
			}
		}
	}
}
