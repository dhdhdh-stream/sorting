#include "branch_experiment.h"

using namespace std;

void BranchExperiment::experiment_transform() {
	double score_standard_deviation = sqrt(*this->existing_score_variance);
	double misguess_standard_deviation = sqrt(*this->existing_misguess_variance);

	cout << "this->existing_average_score: " << *this->existing_average_score << endl;

	double branch_improvement = this->test_branch_average_score - this->test_branch_existing_average_score;
	cout << "this->test_branch_average_score: " << this->test_branch_average_score << endl;
	cout << "this->test_branch_existing_average_score: " << this->test_branch_existing_average_score << endl;

	double replace_improvement = this->test_replace_average_score - *this->existing_average_score;
	cout << "this->test_replace_average_score: " << this->test_replace_average_score << endl;

	double misguess_improvement = *this->existing_average_misguess - this->test_replace_average_misguess;
	cout << "this->test_replace_average_misguess: " << this->test_replace_average_misguess << endl;
	cout << "this->existing_average_misguess: " << *this->existing_average_misguess << endl;

	// 0.0001 rolling average variance approx. equal to 20000 average variance (?)

	double branch_improvement_t_value = branch_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "branch_improvement_t_value: " << branch_improvement_t_value << endl;

	double replace_improvement_t_value = replace_improvement
		/ (score_standard_deviation / sqrt(20000));
	cout << "replace_improvement_t_value: " << replace_improvement_t_value << endl;

	double misguess_improvement_t_value = misguess_improvement
		/ (misguess_standard_deviation / sqrt(20000));
	cout << "misguess_improvement_t_value: " << misguess_improvement_t_value << endl;

	if (branch_improvement_t_value > 2.326) {	// >99%
		if (replace_improvement_t_value > -0.842	// 80%<
				&& this->is_recursive == 0) {
			cout << "FOLD_RESULT_REPLACE" << endl;
			this->experiment_result = BRANCH_EXPERIMENT_RESULT_REPLACE;
		} else {
			cout << "FOLD_RESULT_BRANCH" << endl;
			this->experiment_result = BRANCH_EXPERIMENT_RESULT_BRANCH;
		}
	} else if (*this->existing_average_misguess > 0.01
			&& misguess_improvement_t_value > 2.326	// >99%
			&& replace_improvement_t_value > -0.842	// 80%<
			&& this->is_recursive == 0) {
		cout << "FOLD_RESULT_REPLACE" << endl;
		this->experiment_result = BRANCH_EXPERIMENT_RESULT_REPLACE;
	} else {
		cout << "FOLD_RESULT_FAIL" << endl;
		this->experiment_result = BRANCH_EXPERIMENT_RESULT_FAIL;
	}

	if (this->experiment_result != BRANCH_EXPERIMENT_RESULT_FAIL) {
		// determine if new types needed
		this->new_state_furthest_layer_seen_in = vector<int>(this->num_new_states, (int)this->scope_context.size()+1);

		for (map<int, vector<vector<StateNetwork*>>>::iterator it = this->state_networks.begin();
				it != this->state_networks.end(); it++) {
			for (int n_index = 0; n_index < it->second.size(); n_index++) {
				if (it->second[n_index].size() > 0) {
					for (int s_index = 0; s_index < this->num_new_states; s_index++) {
						double sum_obs_impact = 0.0;
						for (int in_index = 0; in_index < 20; in_index++) {
							sum_obs_impact += abs(it->second[n_index][s_index]->hidden->weights[in_index][0][0]);
						}

						if (sum_obs_impact > 0.1) {
							// network needed
							int furthest_layer_seen_in = this->scope_furthest_layer_seen_in.find(it->first)->second;
							if (furthest_layer_seen_in < this->new_state_furthest_layer_seen_in[s_index]) {
								this->new_state_furthest_layer_seen_in[s_index] = furthest_layer_seen_in;
							}
						}
					}
				}
			}
		}

		this->input_is_new_type = vector<int>(this->num_new_states-5);
		for (int s_index = 0; s_index < this->num_new_states-5; s_index++) {
			if (this->new_state_furthest_layer_seen_in[s_index] < this->local_init_scope_depths[s_index]) {
				this->input_is_new_type[s_index] = true;
			} else {
				this->input_is_new_type[s_index] = false;
			}
		}

		// TODO: check inner

		// remove unnecessary new state


		// remove networks if not new type


		this->state = BRANCH_EXPERIMENT_STATE_NEW_TYPES;
		this->state_iter = 0;
		this->sum_error = 0.0;
	} else {

	}
}
