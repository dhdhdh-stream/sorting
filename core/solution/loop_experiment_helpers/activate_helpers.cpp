#include "loop_experiment.h"

using namespace std;

void LoopExperiment::activate(AbstractNode*& curr_node,
							  Problem& problem,
							  vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper,
							  AbstractExperimentHistory*& history) {
	bool is_selected = false;
	if (run_helper.selected_experiment == NULL) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope->id
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node->id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			bool select = false;
			set<AbstractExperiment*>::iterator it = run_helper.experiments_seen.find(this);
			if (it == run_helper.experiments_seen.end()) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					select = true;
				}

				run_helper.experiments_seen_order.push_back(this);
				run_helper.experiments_seen.insert(this);
			}
			if (select) {
				hook(context);

				run_helper.selected_experiment = this;
				run_helper.experiment_history = new LoopExperimentOverallHistory(this);

				is_selected = true;
			}
		}
	} else if (run_helper.selected_experiment == this) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope->id
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node->id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			is_selected = true;
		}
	}

	if (is_selected) {
		switch (this->state) {
		case LOOP_EXPERIMENT_STATE_TRAIN_EXISTING:

			break;
		case LOOP_EXPERIMENT_STATE_EXPLORE:

			break;
		case LOOP_EXPERIMENT_STATE_TRAIN_PRE:
		case LOOP_EXPERIMENT_STATE_TRAIN:
			switch (this->sub_state) {
			case LOOP_EXPERIMENT_SUB_STATE_TRAIN_HALT:

				break;
			case LOOP_EXPERIMENT_SUB_STATE_TRAIN_CONTINUE:

				break;
			}

			break;
		case LOOP_EXPERIMENT_STATE_MEASURE:

			break;
		case LOOP_EXPERIMENT_STATE_VERIFY_EXISTING:

			break;
		case LOOP_EXPERIMENT_STATE_VERIFY:

			break;
		case LOOP_EXPERIMENT_STATE_CAPTURE_VERIFY:

			break;
		}
	}
}

void LoopExperiment::hook_helper(vector<int>& scope_context,
								 vector<int>& node_context,
								 map<State*, StateStatus>& temp_state_vals,
								 ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->experiment_back_activate(scope_context,
													  node_context,
													  temp_state_vals,
													  action_node_history);
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				hook_helper(scope_context,
							node_context,
							temp_state_vals,
							scope_node_history->inner_scope_history);

				node_context.back() = -1;
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void LoopExperiment::hook(vector<ContextLayer>& context) {
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_state_nodes[s_index].size(); n_index++) {
			this->new_state_nodes[s_index][n_index]->experiment_state_scope_contexts.push_back(this->new_state_scope_contexts[s_index][n_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_node_contexts.push_back(this->new_state_node_contexts[s_index][n_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_obs_indexes.push_back(this->new_state_obs_indexes[s_index][n_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_defs.push_back(this->new_states[s_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_network_indexes.push_back(n_index);
		}
	}

	vector<int> scope_context;
	vector<int> node_context;
	hook_helper(scope_context,
				node_context,
				context[context.size() - this->scope_context.size()].temp_state_vals,
				context[context.size() - this->scope_context.size()].scope_history);
}

void LoopExperiment::unhook() {
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_state_nodes[s_index].size(); n_index++) {
			this->new_state_nodes[s_index][n_index]->experiment_state_scope_contexts.clear();
			this->new_state_nodes[s_index][n_index]->experiment_state_node_contexts.clear();
			this->new_state_nodes[s_index][n_index]->experiment_state_obs_indexes.clear();
			this->new_state_nodes[s_index][n_index]->experiment_state_defs.clear();
			this->new_state_nodes[s_index][n_index]->experiment_state_network_indexes.clear();
		}
	}
}

void LoopExperiment::backprop(double target_val,
							  RunHelper& run_helper,
							  LoopExperimentOverallHistory* history) {
	unhook();

	switch (this->state) {
	case LOOP_EXPERIMENT_STATE_TRAIN_EXISTING:

		break;
	case LOOP_EXPERIMENT_STATE_EXPLORE:

		break;
	case LOOP_EXPERIMENT_STATE_TRAIN_PRE:
	case LOOP_EXPERIMENT_STATE_TRAIN:
		switch (this->sub_state) {
		case LOOP_EXPERIMENT_SUB_STATE_TRAIN_HALT:

			break;
		case LOOP_EXPERIMENT_SUB_STATE_TRAIN_CONTINUE:

			break;
		}

		break;
	case LOOP_EXPERIMENT_STATE_MEASURE:

		break;
	case LOOP_EXPERIMENT_STATE_VERIFY_EXISTING:

		break;
	case LOOP_EXPERIMENT_STATE_VERIFY:

		break;
	case LOOP_EXPERIMENT_STATE_CAPTURE_VERIFY:

		break;
	}

	delete history;
}
