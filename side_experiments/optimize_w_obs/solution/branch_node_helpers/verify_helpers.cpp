#if defined(MDEBUG) && MDEBUG

#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchNode::verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	run_helper.num_analyze += (int)this->inputs.size();

	vector<double> input_vals(this->inputs.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		map<pair<pair<vector<int>,vector<int>>,int>, double>::iterator it =
			context.back().obs_history.find(this->inputs[i_index]);
		if (it != context.back().obs_history.end()) {
			input_vals[i_index] = it->second;
		}
	}
	this->network->activate(input_vals);
	double score = this->network->output->acti_vals[0];

	if (this->verify_key != NULL) {
		cout << "this->id: " << this->id << endl;

		cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
		cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
		problem->print();

		cout << "input_vals:" << endl;
		for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
			cout << i_index << ": " << input_vals[i_index] << endl;
		}

		cout << "context scope" << endl;
		for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
			cout << c_index << ": " << context[c_index].scope_id << endl;
		}
		cout << "context node" << endl;
		for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
			cout << c_index << ": " << context[c_index].node_id << endl;
		}

		if (this->verify_scores[0] != score) {
			cout << "this->verify_scores[0]: " << this->verify_scores[0] << endl;
			cout << "score: " << score << endl;

			cout << "seed: " << seed << endl;

			throw invalid_argument("branch node verify fail");
		}

		this->verify_scores.erase(this->verify_scores.begin());
	}

	bool is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	if (this->input_scope_context_ids.size() > 0) {
		for (int i_index = 0; i_index < (int)this->input_scope_context_ids.size(); i_index++) {
			bool match_context = false;
			if (context.size() >= this->input_scope_context_ids[i_index].size()) {
				match_context = true;
				for (int l_index = 0; l_index < (int)this->input_scope_context_ids[i_index].size()-1; l_index++) {
					int context_index = context.size() - this->input_scope_context_ids[i_index].size() + l_index;
					if (context[context_index].scope_id != this->input_scope_context_ids[i_index][l_index]
							|| context[context_index].node_id != this->input_node_context_ids[i_index][l_index]) {
						match_context = false;
						break;
					}
				}
			}

			if (match_context) {
				if (is_branch) {
					context[context.size() - this->input_scope_context_ids[i_index].size()]
						.obs_history[{{this->input_scope_context_ids[i_index],
							this->input_node_context_ids[i_index]}, -1}] = 1.0;
				} else {
					context[context.size() - this->input_scope_context_ids[i_index].size()]
						.obs_history[{{this->input_scope_context_ids[i_index],
							this->input_node_context_ids[i_index]}, -1}] = -1.0;
				}
			}
		}
	}

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
}

#endif /* MDEBUG */