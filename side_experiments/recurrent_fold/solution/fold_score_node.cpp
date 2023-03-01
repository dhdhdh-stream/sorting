#include "fold_score_node.h"

using namespace std;

FoldScoreNode::FoldScoreNode() {

}

FoldScoreNode::~FoldScoreNode() {

}

void FoldScoreNode::explore_on_path_activate(vector<double>& local_state_vals,
											 vector<double>& input_vals,
											 vector<vector<double>>& flat_vals,
											 double& predicted_score,
											 double& scale_factor,
											 vector<int>& scope_context,
											 vector<int>& node_context,
											 vector<int>& context_iter,
											 vector<ContextHistory*>& context_histories,
											 int& fold_exit_depth,
											 int& fold_exit_node_id,
											 RunHelper& run_helper,
											 FoldNodeHistory* history) {
	bool matches_context = true;
	if (this->fold_scope_context.size() > scope_context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->fold_scope_context.size(); c_index++) {
			if (this->fold_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
					|| this->fold_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
				matches_context = false;
				break;
			}
		}
	}
	if (matches_context) {
		// TODO: save history based on whether before or after explore
		this->existing_score_network->activate(local_state_vals,
											   input_vals);
		double existing_score = scale_factor*this->existing_score_network->output->acti_vals[0];

		double fold_score = this->fold->explore_off_path_score_activate(
			local_state_vals,
			input_vals,
			context_iter,
			context_histories,
			run_helper);
		fold_score *= scale_factor;

		if (existing_score > fold_score) {
			fold_exit_depth = 0;
			fold_exit_node_id = this->existing_next_node_id;
		} else {
			fold_exit_depth = this->fold_scope_context.size()-1;
			fold_exit_node_id = this->fold_next_node_id;
		}
	} else {
		fold_exit_depth = 0;
		fold_exit_node_id = this->existing_next_node_id;
	}
}
