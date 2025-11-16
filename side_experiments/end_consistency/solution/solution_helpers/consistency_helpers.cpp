#include "solution_helpers.h"

#include "abstract_experiment.h"
#include "network.h"
#include "scope.h"

using namespace std;

double calc_consistency(AbstractExperiment* experiment) {
	double existing_sum_consistency = 0.0;
	for (int i_index = 0; i_index < (int)experiment->existing_post_scope_histories.size(); i_index++) {
		double sum_consistency = 0.0;
		int count = 0;
		for (int h_index = 0; h_index < (int)experiment->existing_post_scope_histories[i_index].size(); h_index++) {
			Scope* scope = experiment->existing_post_scope_histories[i_index][h_index]->scope;
			if (scope->consistency_network != NULL) {
				vector<double> input = experiment->existing_post_scope_histories[i_index][h_index]->pre_obs;
				input.insert(input.end(), experiment->existing_post_scope_histories[i_index][h_index]->post_obs.begin(),
					experiment->existing_post_scope_histories[i_index][h_index]->post_obs.end());

				scope->consistency_network->activate(input);
				double consistency = scope->consistency_network->output->acti_vals[0];
				/**
				 * - allow to go below -1.0 to help distinguish between bad and very bad
				 *   - sigmoid not better (?)
				 */
				if (consistency >= 3.0) {
					sum_consistency += 3.0;
					count++;
				} else if (consistency <= -3.0) {
					sum_consistency += -3.0;
					count++;
				} else {
					sum_consistency += consistency;
					count++;
				}
			}
		}

		if (count != 0) {
			existing_sum_consistency += sum_consistency / (double)count;
		}
	}
	double existing_average_consistency = existing_sum_consistency / (double)experiment->existing_post_scope_histories.size();

	double new_sum_consistency = 0.0;
	for (int i_index = 0; i_index < (int)experiment->new_post_scope_histories.size(); i_index++) {
		double sum_consistency = 0.0;
		int count = 0;
		for (int h_index = 0; h_index < (int)experiment->new_post_scope_histories[i_index].size(); h_index++) {
			Scope* scope = experiment->new_post_scope_histories[i_index][h_index]->scope;
			if (scope->consistency_network != NULL) {
				vector<double> input = experiment->new_post_scope_histories[i_index][h_index]->pre_obs;
				input.insert(input.end(), experiment->new_post_scope_histories[i_index][h_index]->post_obs.begin(),
					experiment->new_post_scope_histories[i_index][h_index]->post_obs.end());

				scope->consistency_network->activate(input);
				double consistency = scope->consistency_network->output->acti_vals[0];
				/**
				 * - allow to go below -1.0 to help distinguish between bad and very bad
				 *   - sigmoid not better (?)
				 */
				if (consistency >= 3.0) {
					sum_consistency += 3.0;
					count++;
				} else if (consistency <= -3.0) {
					sum_consistency += -3.0;
					count++;
				} else {
					sum_consistency += consistency;
					count++;
				}
			}
		}

		if (count != 0) {
			new_sum_consistency += sum_consistency / (double)count;
		}
	}
	double new_average_consistency = new_sum_consistency / (double)experiment->new_post_scope_histories.size();

	return new_average_consistency - existing_average_consistency;
}
