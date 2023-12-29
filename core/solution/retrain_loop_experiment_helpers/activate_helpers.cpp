#include "retrain_loop_experiment.h"

#include "action_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

bool RetrainLoopExperiment::activate(Problem* problem,
									 vector<ContextLayer>& context,
									 int& inner_exit_depth,
									 AbstractNode*& inner_exit_node,
									 RunHelper& run_helper,
									 ScopeNodeHistory* parent_scope_node_history) {
	bool is_selected = false;
	if (run_helper.selected_experiment == NULL) {
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
			hook();

			run_helper.selected_experiment = this;
			run_helper.experiment_history = new RetrainLoopExperimentOverallHistory(this);

			is_selected = true;
		}
	} else if (run_helper.selected_experiment == this) {
		is_selected = true;
	}

	if (is_selected) {
		back_activate(context);

		switch (this->state) {
		case RETRAIN_LOOP_EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_activate(problem,
									  context,
									  inner_exit_depth,
									  inner_exit_node,
									  run_helper,
									  parent_scope_node_history);
			break;
		case RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_HALT:
			train_halt_activate(problem,
								context,
								inner_exit_depth,
								inner_exit_node,
								run_helper,
								parent_scope_node_history);
			break;
		case RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE_PRE:
		case RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE:
			train_continue_activate(problem,
									context,
									inner_exit_depth,
									inner_exit_node,
									run_helper,
									parent_scope_node_history);
			break;
		case RETRAIN_LOOP_EXPERIMENT_STATE_MEASURE:
			measure_activate(problem,
							 context,
							 inner_exit_depth,
							 inner_exit_node,
							 run_helper,
							 parent_scope_node_history);
			break;
		case RETRAIN_LOOP_EXPERIMENT_STATE_VERIFY_EXISTING:
			verify_existing_activate(problem,
									 context,
									 inner_exit_depth,
									 inner_exit_node,
									 run_helper,
									 parent_scope_node_history);
			break;
		case RETRAIN_LOOP_EXPERIMENT_STATE_VERIFY:
			verify_activate(problem,
							context,
							inner_exit_depth,
							inner_exit_node,
							run_helper,
							parent_scope_node_history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case RETRAIN_LOOP_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(problem,
									context,
									inner_exit_depth,
									inner_exit_node,
									run_helper,
									parent_scope_node_history);
			break;
		#endif /* MDEBUG */
		}

		return true;
	} else {
		return false;
	}
}

void RetrainLoopExperiment::hook() {
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_state_nodes[s_index].size(); n_index++) {
			this->new_state_nodes[s_index][n_index]->experiment_state_scope_contexts.push_back(this->new_state_scope_contexts[s_index][n_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_node_contexts.push_back(this->new_state_node_contexts[s_index][n_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_obs_indexes.push_back(this->new_state_obs_indexes[s_index][n_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_defs.push_back(this->new_states[s_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_network_indexes.push_back(n_index);
		}
	}
}

void RetrainLoopExperiment::back_activate_helper(vector<int>& scope_context,
												 vector<int>& node_context,
												 map<State*, StateStatus>& temp_state_vals,
												 ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			AbstractNodeHistory* node_history = scope_history->node_histories[i_index][h_index];
			if (node_history->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->experiment_back_activate(scope_context,
													  node_context,
													  temp_state_vals,
													  action_node_history);
			} else if (node_history->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				back_activate_helper(scope_context,
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

void RetrainLoopExperiment::back_activate(vector<ContextLayer>& context) {
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		context[context.size() - 1 - this->scope_node->loop_scope_context.size()].temp_state_vals
			.erase(this->new_states[s_index]);
	}

	vector<int> scope_context;
	vector<int> node_context;
	back_activate_helper(scope_context,
						 node_context,
						 context[context.size() - 1 - this->scope_node->loop_scope_context.size()].temp_state_vals,
						 context[context.size() - 1 - this->scope_node->loop_scope_context.size()].scope_history);
}

void RetrainLoopExperiment::unhook() {
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

void RetrainLoopExperiment::backprop(double target_val,
									 RunHelper& run_helper,
									 RetrainLoopExperimentOverallHistory* history) {
	unhook();

	switch (this->state) {
	case RETRAIN_LOOP_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_HALT:
		train_halt_backprop(target_val,
							history);
		break;
	case RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE_PRE:
	case RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE:
		train_continue_backprop(target_val,
								history);
		break;
	case RETRAIN_LOOP_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val);
		break;
	case RETRAIN_LOOP_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case RETRAIN_LOOP_EXPERIMENT_STATE_VERIFY:
		verify_backprop(target_val);
		break;
	#if defined(MDEBUG) && MDEBUG
	case RETRAIN_LOOP_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}

	delete history;
}
