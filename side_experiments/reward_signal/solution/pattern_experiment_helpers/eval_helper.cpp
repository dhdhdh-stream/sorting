#include "pattern_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "pattern.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

const double MIN_CONSIDER_HIT_PERCENT = 0.2;

const int NUM_TRIES = 100;

const int NUM_KEYPOINTS = 10;
const int NUM_INPUTS = 10;

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

void PatternExperiment::eval() {
	int num_instances = (int)this->explore_target_vals.size();
	int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
	int num_test_instances = num_instances - num_train_instances;

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < num_instances; h_index++) {
		sum_vals += this->explore_target_vals[h_index];
	}
	double average_val = sum_vals / (double)num_instances;

	for (int h_index = 0; h_index < num_instances; h_index++) {
		this->explore_target_vals[h_index] -= average_val;
	}

	/**
	 * - check existing pattern
	 */
	double default_sum_misguess = 0.0;
	for (int h_index = num_train_instances; h_index < num_instances; h_index++) {
		default_sum_misguess += this->explore_target_vals[h_index] * this->explore_target_vals[h_index];
	}
	double default_average_misguess = default_sum_misguess / (double)num_test_instances;

	double curr_average_misguess;
	if (this->scope_context->pattern == NULL) {
		curr_average_misguess = default_average_misguess;
	} else {
		vector<double> predicted_outputs;
		vector<double> actual_outputs;
		for (int h_index = num_train_instances; h_index < num_instances; h_index++) {
			bool has_match;
			double predicted;
			this->scope_context->pattern->activate(has_match,
												   predicted,
												   this->explore_scope_histories[h_index]);
			if (has_match) {
				predicted_outputs.push_back(predicted);
				actual_outputs.push_back(this->explore_target_vals[h_index]);
			}
		}

		double existing_match_percent = (double)predicted_outputs.size() / (double)num_test_instances;
		if (existing_match_percent < PATTERN_MIN_MATCH_PERCENT
				|| existing_match_percent > PATTERN_MAX_MATCH_PERCENT) {
			delete this->scope_context->pattern;
			this->scope_context->pattern = NULL;

			curr_average_misguess = default_average_misguess;
		} else {
			double existing_sum_misguess = 0.0;
			for (int p_index = 0; p_index < (int)predicted_outputs.size(); p_index++) {
				existing_sum_misguess += (predicted_outputs[p_index] - actual_outputs[p_index])
					* (predicted_outputs[p_index] - actual_outputs[p_index]);
			}
			double existing_average_misguess = existing_sum_misguess / (double)predicted_outputs.size();

			if (existing_average_misguess < default_average_misguess) {
				delete this->scope_context->pattern;
				this->scope_context->pattern = NULL;

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

		bool is_success = train_keypoints_helper(potential_pattern);
		if (!is_success) {
			delete potential_pattern;
			continue;
		}

		double potential_average_misguess = train_and_eval_predict_helper(potential_pattern);
		if (potential_average_misguess < curr_average_misguess) {
			if (this->scope_context->pattern != NULL) {
				delete this->scope_context->pattern;
			}
			this->scope_context->pattern = potential_pattern;

			curr_average_misguess = potential_average_misguess;
		} else {
			delete potential_pattern;
		}
	}
}
