#include "pattern.h"

#include "network.h"
#include "solution_helpers.h"

using namespace std;

Pattern::Pattern() {
	this->keypoint_network = NULL;
	this->predict_network = NULL;
}

Pattern::~Pattern() {
	if (this->keypoint_network != NULL) {
		delete this->keypoint_network;
	}

	if (this->predict_network != NULL) {
		delete this->predict_network;
	}
}

void Pattern::activate(bool& has_match,
					   double& predicted,
					   ScopeHistory* scope_history) {
	vector<double> keypoint_vals(this->keypoints.size());
	vector<bool> keypoint_is_on(this->keypoints.size());
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_history,
						   this->keypoints[k_index],
						   0,
						   val,
						   is_on);
		keypoint_vals[k_index] = val;
		keypoint_is_on[k_index] = is_on;
	}
	this->keypoint_network->activate(keypoint_vals,
									 keypoint_is_on);
	#if defined(MDEBUG) && MDEBUG
	if (rand()%2 == 0) {
	#else
	if (this->keypoint_network->output->acti_vals[0] < 0.0) {
	#endif /* MDEBUG */
		has_match = false;
	} else {
		has_match = true;

		vector<double> input_vals(this->inputs.size());
		vector<bool> input_is_on(this->inputs.size());
		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   this->inputs[i_index],
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		this->predict_network->activate(input_vals,
										input_is_on);
		predicted = this->predict_network->output->acti_vals[0];
	}
}

void Pattern::clean_inputs(Scope* scope,
						   int node_id) {
	for (int k_index = (int)this->keypoints.size()-1; k_index >= 0; k_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->keypoints[k_index].scope_context.size(); l_index++) {
			if (this->keypoints[k_index].scope_context[l_index] == scope
					&& this->keypoints[k_index].node_context[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->keypoints.erase(this->keypoints.begin() + k_index);
			this->keypoint_network->remove_input(k_index);
		}
	}

	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope
					&& this->inputs[i_index].node_context[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->predict_network->remove_input(i_index);
		}
	}
}

void Pattern::clean_inputs(Scope* scope) {
	for (int k_index = (int)this->keypoints.size()-1; k_index >= 0; k_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->keypoints[k_index].scope_context.size(); l_index++) {
			if (this->keypoints[k_index].scope_context[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->keypoints.erase(this->keypoints.begin() + k_index);
			this->keypoint_network->remove_input(k_index);
		}
	}

	for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->inputs.erase(this->inputs.begin() + i_index);
			this->predict_network->remove_input(i_index);
		}
	}
}

void Pattern::replace_factor(Scope* scope,
							 int original_node_id,
							 int original_factor_index,
							 int new_node_id,
							 int new_factor_index) {
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		if (this->keypoints[k_index].scope_context.back() == scope
				&& this->keypoints[k_index].node_context.back() == original_node_id
				&& this->keypoints[k_index].factor_index == original_factor_index) {
			this->keypoints[k_index].node_context.back() = new_node_id;
			this->keypoints[k_index].factor_index = new_factor_index;
		}
	}

	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.back() == scope
				&& this->inputs[i_index].node_context.back() == original_node_id
				&& this->inputs[i_index].factor_index == original_factor_index) {
			this->inputs[i_index].node_context.back() = new_node_id;
			this->inputs[i_index].factor_index = new_factor_index;
		}
	}
}

void Pattern::replace_obs_node(Scope* scope,
							   int original_node_id,
							   int new_node_id) {
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		if (this->keypoints[k_index].scope_context.back() == scope
				&& this->keypoints[k_index].node_context.back() == original_node_id) {
			this->keypoints[k_index].node_context.back() = new_node_id;
		}
	}

	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		if (this->inputs[i_index].scope_context.back() == scope
				&& this->inputs[i_index].node_context.back() == original_node_id) {
			this->inputs[i_index].node_context.back() = new_node_id;
		}
	}
}

void Pattern::replace_scope(Scope* original_scope,
							Scope* new_scope,
							int new_scope_node_id) {
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		for (int l_index = 1; l_index < (int)this->keypoints[k_index].scope_context.size(); l_index++) {
			if (this->keypoints[k_index].scope_context[l_index] == original_scope) {
				this->keypoints[k_index].scope_context.insert(
					this->keypoints[k_index].scope_context.begin() + l_index, new_scope);
				this->keypoints[k_index].node_context.insert(
					this->keypoints[k_index].node_context.begin() + l_index, new_scope_node_id);
				break;
			}
		}
	}

	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		for (int l_index = 1; l_index < (int)this->inputs[i_index].scope_context.size(); l_index++) {
			if (this->inputs[i_index].scope_context[l_index] == original_scope) {
				this->inputs[i_index].scope_context.insert(
					this->inputs[i_index].scope_context.begin() + l_index, new_scope);
				this->inputs[i_index].node_context.insert(
					this->inputs[i_index].node_context.begin() + l_index, new_scope_node_id);
				break;
			}
		}
	}
}
