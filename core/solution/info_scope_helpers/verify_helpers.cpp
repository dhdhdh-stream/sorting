#if defined(MDEBUG) && MDEBUG

#include "info_scope.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_pass_through_experiment.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "utilities.h"

using namespace std;

void InfoScope::verify_activate(Problem* problem,
								RunHelper& run_helper,
								ScopeHistory*& subscope_history,
								bool& result_is_positive) {
	if (this->state == INFO_SCOPE_STATE_DISABLED_NEGATIVE) {
		subscope_history = NULL;
		result_is_positive = false;
	} else if (this->state == INFO_SCOPE_STATE_DISABLED_POSITIVE) {
		subscope_history = NULL;
		result_is_positive = true;
	} else {
		vector<ContextLayer> inner_context;
		inner_context.push_back(ContextLayer());

		inner_context.back().scope = this->subscope;
		inner_context.back().node = NULL;

		subscope_history = new ScopeHistory(this->subscope);
		inner_context.back().scope_history = subscope_history;

		this->subscope->verify_activate(problem,
										inner_context,
										run_helper,
										subscope_history);

		run_helper.num_decisions++;

		vector<double> input_vals(this->input_node_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = subscope_history->node_histories.find(
				this->input_node_contexts[i_index]);
			if (it != subscope_history->node_histories.end()) {
				switch (this->input_node_contexts[i_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							input_vals[i_index] = 1.0;
						} else {
							input_vals[i_index] = -1.0;
						}
					}
					break;
				case NODE_TYPE_INFO_SCOPE:
					{
						InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
						if (info_scope_node_history->is_positive) {
							input_vals[i_index] = 1.0;
						} else {
							input_vals[i_index] = -1.0;
						}
					}
					break;
				case NODE_TYPE_INFO_BRANCH:
					{
						InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
						if (info_branch_node_history->is_branch) {
							input_vals[i_index] = 1.0;
						} else {
							input_vals[i_index] = -1.0;
						}
					}
					break;
				}
			}
		}

		double negative_score = this->negative_average_score;
		for (int i_index = 0; i_index < (int)this->linear_negative_input_indexes.size(); i_index++) {
			negative_score += input_vals[this->linear_negative_input_indexes[i_index]] * this->linear_negative_weights[i_index];
		}
		if (this->negative_network != NULL) {
			vector<vector<double>> negative_network_input_vals(this->negative_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->negative_network_input_indexes.size(); i_index++) {
				negative_network_input_vals[i_index] = vector<double>(this->negative_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->negative_network_input_indexes[i_index].size(); v_index++) {
					negative_network_input_vals[i_index][v_index] = input_vals[this->negative_network_input_indexes[i_index][v_index]];
				}
			}
			this->negative_network->activate(negative_network_input_vals);
			negative_score += this->negative_network->output->acti_vals[0];
		}

		double positive_score = this->positive_average_score;
		for (int i_index = 0; i_index < (int)this->linear_positive_input_indexes.size(); i_index++) {
			positive_score += input_vals[this->linear_positive_input_indexes[i_index]] * this->linear_positive_weights[i_index];
		}
		if (this->positive_network != NULL) {
			vector<vector<double>> positive_network_input_vals(this->positive_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->positive_network_input_indexes.size(); i_index++) {
				positive_network_input_vals[i_index] = vector<double>(this->positive_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->positive_network_input_indexes[i_index].size(); v_index++) {
					positive_network_input_vals[i_index][v_index] = input_vals[this->positive_network_input_indexes[i_index][v_index]];
				}
			}
			this->positive_network->activate(positive_network_input_vals);
			positive_score += this->positive_network->output->acti_vals[0];
		}

		if (this->verify_key == run_helper.verify_key) {
			cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
			problem->print();

			cout << "subscope_history->node_histories" << endl;
			for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = subscope_history->node_histories.begin();
					it != subscope_history->node_histories.end(); it++) {
				cout << it->first->id << endl;
			}

			cout << "input_vals:" << endl;
			for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
				cout << i_index << ": " << input_vals[i_index] << endl;
			}

			if (this->verify_negative_scores[0] != negative_score
					|| this->verify_positive_scores[0] != positive_score) {
				cout << "this->verify_negative_scores[0]: " << this->verify_negative_scores[0] << endl;
				cout << "negative_score: " << negative_score << endl;

				cout << "this->verify_positive_scores[0]: " << this->verify_positive_scores[0] << endl;
				cout << "positive_score: " << positive_score << endl;

				cout << "seed: " << seed << endl;

				throw invalid_argument("info scope verify fail");
			}

			this->verify_negative_scores.erase(this->verify_negative_scores.begin());
			this->verify_positive_scores.erase(this->verify_positive_scores.begin());
		}

		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			result_is_positive = true;
		} else {
			result_is_positive = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (positive_score >= negative_score) {
			result_is_positive = true;
		} else {
			result_is_positive = false;
		}
		#endif /* MDEBUG */
	}
}

#endif /* MDEBUG */