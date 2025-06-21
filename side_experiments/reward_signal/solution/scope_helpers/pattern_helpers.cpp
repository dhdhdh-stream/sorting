#include "scope.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "obs_node.h"
#include "pattern.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

const double MIN_CONSIDER_HIT_PERCENT = 0.2;

const int NUM_TRIES = 100;

const int NUM_KEYPOINTS = 10;
const int NUM_INPUTS = 10;

const int KEYPOINT_TRAIN_ITERS = 1000;
const int MAX_TRAIN_ITERS = 50000;
const double EXISTING_MIN_MATCH_PERCENT = 0.95;
const double EXPLORE_MIN_MATCH_PERCENT = 0.15;
const double EXPLORE_MAX_MATCH_PERCENT = 0.20;

double analyze_input(Input& input,
					 vector<ScopeHistory*>& existing_scope_histories) {
	vector<double> vals;
	int num_is_on = 0;
	for (int h_index = 0; h_index < (int)existing_scope_histories.size(); h_index++) {
		double val;
		bool is_on;
		fetch_input_helper(existing_scope_histories[h_index],
						   input,
						   0,
						   val,
						   is_on);
		if (is_on) {
			vals.push_back(val);
			num_is_on++;
		}
	}

	return (double)num_is_on / (double)existing_scope_histories.size();
}

void gather_input_helper(ScopeHistory* scope_history,
						 vector<Scope*>& scope_context,
						 vector<int>& node_context,
						 map<Input, double>& existing_input_tracker,
						 vector<ScopeHistory*>& existing_scope_histories) {
	Scope* scope = scope_history->scope;

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				scope_context.push_back(scope);
				node_context.push_back(it->first);

				gather_input_helper(scope_node_history->scope_history,
									scope_context,
									node_context,
									existing_input_tracker,
									existing_scope_histories);

				scope_context.pop_back();
				node_context.pop_back();
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				scope_context.push_back(scope);
				node_context.push_back(it->first);

				Input input;
				input.scope_context = scope_context;
				input.node_context = node_context;
				input.factor_index = -1;
				input.obs_index = -1;

				map<Input, double>::iterator it = existing_input_tracker.find(input);
				if (it == existing_input_tracker.end()) {
					double hit_percent = analyze_input(input,
													   existing_scope_histories);
					existing_input_tracker[input] = hit_percent;
				}

				scope_context.pop_back();
				node_context.pop_back();
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;

				for (int o_index = 0; o_index < (int)obs_node_history->obs_history.size(); o_index++) {
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					Input input;
					input.scope_context = scope_context;
					input.node_context = node_context;
					input.factor_index = -1;
					input.obs_index = o_index;

					map<Input, double>::iterator it = existing_input_tracker.find(input);
					if (it == existing_input_tracker.end()) {
						double hit_percent = analyze_input(input,
														   existing_scope_histories);
						existing_input_tracker[input] = hit_percent;
					}

					scope_context.pop_back();
					node_context.pop_back();
				}

				for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
					scope_context.push_back(scope);
					node_context.push_back(it->first);

					Input input;
					input.scope_context = scope_context;
					input.node_context = node_context;
					input.factor_index = f_index;
					input.obs_index = -1;

					map<Input, double>::iterator it = existing_input_tracker.find(input);
					if (it == existing_input_tracker.end()) {
						double hit_percent = analyze_input(input,
														   existing_scope_histories);
						existing_input_tracker[input] = hit_percent;
					}

					scope_context.pop_back();
					node_context.pop_back();
				}
			}
			break;
		}
	}
}

bool train_keypoints_helper(Pattern* potential_pattern,
							vector<ScopeHistory*>& existing_scope_histories,
							vector<ScopeHistory*>& explore_scope_histories) {
	vector<vector<double>> existing_vals(existing_scope_histories.size());
	vector<vector<bool>> existing_is_on(existing_scope_histories.size());
	for (int h_index = 0; h_index < (int)existing_scope_histories.size(); h_index++) {
		vector<double> curr_vals(potential_pattern->keypoints.size());
		vector<bool> curr_is_on(potential_pattern->keypoints.size());
		for (int k_index = 0; k_index < (int)potential_pattern->keypoints.size(); k_index++) {
			double val;
			bool is_on;
			fetch_input_helper(existing_scope_histories[h_index],
							   potential_pattern->keypoints[k_index],
							   0,
							   val,
							   is_on);
			curr_vals[k_index] = val;
			curr_is_on[k_index] = is_on;
		}
		existing_vals[h_index] = curr_vals;
		existing_is_on[h_index] = curr_is_on;
	}

	vector<vector<double>> explore_vals(explore_scope_histories.size());
	vector<vector<bool>> explore_is_on(explore_scope_histories.size());
	for (int h_index = 0; h_index < (int)explore_scope_histories.size(); h_index++) {
		vector<double> curr_vals(potential_pattern->keypoints.size());
		vector<bool> curr_is_on(potential_pattern->keypoints.size());
		for (int k_index = 0; k_index < (int)potential_pattern->keypoints.size(); k_index++) {
			double val;
			bool is_on;
			fetch_input_helper(explore_scope_histories[h_index],
							   potential_pattern->keypoints[k_index],
							   0,
							   val,
							   is_on);
			curr_vals[k_index] = val;
			curr_is_on[k_index] = is_on;
		}
		explore_vals[h_index] = curr_vals;
		explore_is_on[h_index] = curr_is_on;
	}

	potential_pattern->keypoint_network = new Network((int)potential_pattern->keypoints.size(),
													  explore_vals,
													  explore_is_on);

	uniform_int_distribution<int> is_existing_distribution(0, 5);
	uniform_int_distribution<int> existing_distribution(0, (int)existing_scope_histories.size()-1);
	uniform_int_distribution<int> explore_distribution(0, (int)explore_scope_histories.size()-1);
	double curr_explore_target = -0.2;
	int train_iter = 0;
	while (true) {
		train_iter++;
		if (train_iter >= MAX_TRAIN_ITERS) {
			return false;
		}

		int existing_num_match = 0;
		int existing_count = 0;
		int explore_num_match = 0;
		int explore_count = 0;
		for (int t_index = 0; t_index < KEYPOINT_TRAIN_ITERS; t_index++) {
			if (is_existing_distribution(generator) == 0) {
				existing_count++;

				int random_index = existing_distribution(generator);
				potential_pattern->keypoint_network->activate(existing_vals[random_index],
															  existing_is_on[random_index]);

				if (potential_pattern->keypoint_network->output->acti_vals[0] > 0.0) {
					existing_num_match++;
				}

				double error = 1.0 - potential_pattern->keypoint_network->output->acti_vals[0];
				potential_pattern->keypoint_network->backprop(error);
			} else {
				explore_count++;

				int random_index = explore_distribution(generator);
				potential_pattern->keypoint_network->activate(explore_vals[random_index],
															  explore_is_on[random_index]);

				if (potential_pattern->keypoint_network->output->acti_vals[0] > 0.0) {
					explore_num_match++;
				}

				double error = curr_explore_target - potential_pattern->keypoint_network->output->acti_vals[0];
				potential_pattern->keypoint_network->backprop(error);
			}
		}

		double existing_match_percent = (double)existing_num_match / (double)existing_count;
		double explore_match_percent = (double)explore_num_match / (double)explore_count;
		if (existing_match_percent >= EXISTING_MIN_MATCH_PERCENT
				&& explore_match_percent >= EXPLORE_MIN_MATCH_PERCENT
				&& explore_match_percent <= EXPLORE_MAX_MATCH_PERCENT) {
			break;
		} else {
			if (explore_match_percent > EXPLORE_MAX_MATCH_PERCENT) {
				curr_explore_target *= 1.25;
			} else if (explore_match_percent < EXPLORE_MIN_MATCH_PERCENT) {
				curr_explore_target *= 0.8;
			}
		}
	}

	return true;
}

double train_and_eval_predict_helper(Pattern* potential_pattern,
									 vector<ScopeHistory*>& explore_scope_histories,
									 vector<double>& explore_target_vals) {
	vector<vector<double>> filtered_input_vals;
	vector<vector<bool>> filtered_input_is_on;
	vector<double> filtered_target_vals;
	for (int h_index = 0; h_index < (int)explore_scope_histories.size(); h_index++) {
		vector<double> keypoint_vals(potential_pattern->keypoints.size());
		vector<bool> keypoint_is_on(potential_pattern->keypoints.size());
		for (int k_index = 0; k_index < (int)potential_pattern->keypoints.size(); k_index++) {
			double val;
			bool is_on;
			fetch_input_helper(explore_scope_histories[h_index],
							   potential_pattern->keypoints[k_index],
							   0,
							   val,
							   is_on);
			keypoint_vals[k_index] = val;
			keypoint_is_on[k_index] = is_on;
		}
		potential_pattern->keypoint_network->activate(keypoint_vals,
													  keypoint_is_on);
		if (potential_pattern->keypoint_network->output->acti_vals[0] > 0.0) {
			vector<double> curr_vals(potential_pattern->inputs.size());
			vector<bool> curr_is_on(potential_pattern->inputs.size());
			for (int i_index = 0; i_index < (int)potential_pattern->inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(explore_scope_histories[h_index],
								   potential_pattern->inputs[i_index],
								   0,
								   val,
								   is_on);
				curr_vals[i_index] = val;
				curr_is_on[i_index] = is_on;
			}

			filtered_input_vals.push_back(curr_vals);
			filtered_input_is_on.push_back(curr_is_on);
			filtered_target_vals.push_back(explore_target_vals[h_index]);
		}
	}

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)filtered_target_vals.size(); h_index++) {
		sum_vals += filtered_target_vals[h_index];
	}
	double average_val = sum_vals / (double)filtered_target_vals.size();

	vector<double> n_filtered_target_vals((int)filtered_target_vals.size());
	for (int h_index = 0; h_index < (int)filtered_target_vals.size(); h_index++) {
		n_filtered_target_vals[h_index] = filtered_target_vals[h_index] - average_val;
	}

	potential_pattern->predict_network = new Network((int)potential_pattern->inputs.size(),
													 filtered_input_vals,
													 filtered_input_is_on);

	train_network(filtered_input_vals,
				  filtered_input_is_on,
				  n_filtered_target_vals,
				  potential_pattern->predict_network);

	double average_misguess;
	double val_standard_deviation;
	measure_predict_network(filtered_input_vals,
							filtered_input_is_on,
							n_filtered_target_vals,
							potential_pattern->predict_network,
							average_misguess,
							val_standard_deviation);

	potential_pattern->predict_standard_deviation = val_standard_deviation;

	return average_misguess;
}

void Scope::update_pattern() {
	int num_instances = (int)this->explore_target_vals.size();
	int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
	int num_test_instances = num_instances - num_train_instances;

	double default_sum_vals = 0.0;
	for (int h_index = 0; h_index < num_instances; h_index++) {
		default_sum_vals += this->explore_target_vals[h_index];
	}
	double default_average_val = default_sum_vals / (double)num_instances;

	vector<double> n_default_target_vals(num_instances);
	for (int h_index = 0; h_index < num_instances; h_index++) {
		n_default_target_vals[h_index] = this->explore_target_vals[h_index] - default_average_val;
	}

	/**
	 * - check existing pattern
	 */
	double default_sum_misguess = 0.0;
	for (int h_index = num_train_instances; h_index < num_instances; h_index++) {
		default_sum_misguess += n_default_target_vals[h_index] * n_default_target_vals[h_index];
	}
	double default_average_misguess = default_sum_misguess / (double)num_test_instances;

	double curr_average_misguess;
	if (this->pattern == NULL) {
		curr_average_misguess = default_average_misguess;
	} else {
		vector<double> predicted_outputs;
		vector<double> existing_outputs;
		for (int h_index = num_train_instances; h_index < num_instances; h_index++) {
			bool has_match;
			double predicted;
			this->pattern->activate(has_match,
									predicted,
									this->explore_scope_histories[h_index]);
			if (has_match) {
				predicted_outputs.push_back(predicted);
				existing_outputs.push_back(this->explore_target_vals[h_index]);
			}
		}

		double existing_match_percent = (double)predicted_outputs.size() / (double)num_test_instances;
		if (existing_match_percent < EXPLORE_MIN_MATCH_PERCENT
				|| existing_match_percent > EXPLORE_MAX_MATCH_PERCENT) {
			delete this->pattern;
			this->pattern = NULL;

			curr_average_misguess = default_average_misguess;
		} else {
			double existing_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)existing_outputs.size(); h_index++) {
				existing_sum_vals += existing_outputs[h_index];
			}
			double existing_average_val = existing_sum_vals / (double)existing_outputs.size();

			double existing_sum_misguess = 0.0;
			for (int p_index = 0; p_index < (int)predicted_outputs.size(); p_index++) {
				double n_output = existing_outputs[p_index] - existing_average_val;
				existing_sum_misguess += (predicted_outputs[p_index] - n_output)
					* (predicted_outputs[p_index] - n_output);
			}
			double existing_average_misguess = existing_sum_misguess / (double)predicted_outputs.size();

			if (existing_average_misguess < default_average_misguess) {
				delete this->pattern;
				this->pattern = NULL;

				curr_average_misguess = default_average_misguess;
			} else {
				curr_average_misguess = existing_average_misguess;
			}
		}
	}

	map<Input, double> existing_input_tracker;
	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		vector<Scope*> scope_context;
		vector<int> node_context;
		gather_input_helper(this->existing_scope_histories[h_index],
							scope_context,
							node_context,
							existing_input_tracker,
							this->existing_scope_histories);
	}

	vector<Input> possible_inputs;
	for (map<Input, double>::iterator it = existing_input_tracker.begin();
			it != existing_input_tracker.end(); it++) {
		if (it->second >= MIN_CONSIDER_HIT_PERCENT) {
			possible_inputs.push_back(it->first);
		}
	}

	for (int t_index = 0; t_index < NUM_TRIES; t_index++) {
		vector<int> remaining_indexes;
		for (int i_index = 0; i_index < (int)possible_inputs.size(); i_index++) {
			remaining_indexes.push_back(i_index);
		}

		vector<Input> keypoints;
		for (int k_index = 0; k_index < NUM_KEYPOINTS; k_index++) {
			uniform_int_distribution<int> remaining_distribution(0, remaining_indexes.size()-1);
			int random_index = remaining_distribution(generator);
			keypoints.push_back(possible_inputs[remaining_indexes[random_index]]);
			remaining_indexes.erase(remaining_indexes.begin() + random_index);
		}

		vector<Input> inputs;
		for (int i_index = 0; i_index < NUM_INPUTS; i_index++) {
			uniform_int_distribution<int> remaining_distribution(0, remaining_indexes.size()-1);
			int random_index = remaining_distribution(generator);
			inputs.push_back(possible_inputs[remaining_indexes[random_index]]);
			remaining_indexes.erase(remaining_indexes.begin() + random_index);
		}

		Pattern* potential_pattern = new Pattern();
		potential_pattern->keypoints = keypoints;
		potential_pattern->inputs = inputs;

		bool is_success = train_keypoints_helper(potential_pattern,
												 this->existing_scope_histories,
												 this->explore_scope_histories);
		if (!is_success) {
			delete potential_pattern;
			continue;
		}

		double potential_average_misguess = train_and_eval_predict_helper(
			potential_pattern,
			this->explore_scope_histories,
			this->explore_target_vals);
		if (potential_average_misguess < curr_average_misguess) {
			if (this->pattern != NULL) {
				delete this->pattern;
			}
			this->pattern = potential_pattern;

			curr_average_misguess = potential_average_misguess;
		} else {
			delete potential_pattern;
		}
	}

	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		delete this->existing_scope_histories[h_index];
	}
	this->existing_scope_histories.clear();
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		delete this->explore_scope_histories[h_index];
	}
	this->explore_scope_histories.clear();
	this->explore_target_vals.clear();
}
