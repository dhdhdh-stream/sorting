// #include "commit_experiment.h"

// #include "action_node.h"
// #include "constants.h"
// #include "globals.h"
// #include "obs_node.h"
// #include "problem.h"
// #include "scope.h"
// #include "scope_node.h"
// #include "solution_helpers.h"

// using namespace std;

// void CommitExperiment::commit_new_gather_activate(
// 		AbstractNode*& curr_node,
// 		Problem* problem,
// 		RunHelper& run_helper,
// 		ScopeHistory* scope_history) {
// 	for (int n_index = 0; n_index < this->step_iter; n_index++) {
// 		switch (this->new_nodes[n_index]->type) {
// 		case NODE_TYPE_ACTION:
// 			{
// 				ActionNode* node = (ActionNode*)this->new_nodes[n_index];
// 				node->commit_activate(problem,
// 									  run_helper,
// 									  scope_history);
// 			}
// 			break;
// 		case NODE_TYPE_SCOPE:
// 			{
// 				ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
// 				node->commit_activate(problem,
// 									  run_helper,
// 									  scope_history);
// 			}
// 			break;
// 		case NODE_TYPE_OBS:
// 			{
// 				ObsNode* node = (ObsNode*)this->new_nodes[n_index];
// 				node->commit_activate(problem,
// 									  run_helper,
// 									  scope_history);
// 			}
// 			break;
// 		}
// 	}

// 	vector<Scope*> scope_context;
// 	vector<int> node_context;
// 	int node_count = 0;
// 	Input new_input;
// 	gather_possible_helper(scope_history,
// 						   scope_context,
// 						   node_context,
// 						   node_count,
// 						   new_input);

// 	bool is_existing = false;
// 	for (int i_index = 0; i_index < (int)this->commit_new_inputs.size(); i_index++) {
// 		if (new_input == this->commit_new_inputs[i_index]) {
// 			is_existing = true;
// 			break;
// 		}
// 	}
// 	if (!is_existing) {
// 		this->commit_new_inputs.push_back(new_input);
// 	}

// 	for (int f_index = 0; f_index < GATHER_FACTORS_PER_ITER; f_index++) {
// 		pair<int,int> new_factor = {-1, -1};
// 		gather_possible_factor_helper(scope_history,
// 									  new_factor);

// 		if (new_factor.first != -1) {
// 			bool is_existing = false;
// 			for (int i_index = 0; i_index < (int)this->commit_new_factor_ids.size(); i_index++) {
// 				if (new_factor == this->commit_new_factor_ids[i_index]) {
// 					is_existing = true;
// 					break;
// 				}
// 			}
// 			if (!is_existing) {
// 				this->commit_new_factor_ids.push_back(new_factor);
// 			}
// 		}
// 	}

// 	for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
// 		if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
// 			problem->perform_action(this->save_actions[s_index]);

// 			run_helper.num_actions++;
// 		} else {
// 			ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[s_index]);
// 			this->save_scopes[s_index]->activate(problem,
// 				run_helper,
// 				inner_scope_history);
// 			delete inner_scope_history;
// 		}
// 	}

// 	curr_node = this->save_exit_next_node;
// }

// void CommitExperiment::commit_new_gather_backprop() {
// 	this->state_iter++;
// 	if (this->state_iter >= GATHER_ITERS) {
// 		while (this->commit_new_inputs.size() > GATHER_ITERS) {
// 			uniform_int_distribution<int> remove_distribution(0, this->commit_new_inputs.size()-1);
// 			int remove_index = remove_distribution(generator);
// 			this->commit_new_inputs.erase(this->commit_new_inputs.begin() + remove_index);
// 		}

// 		while (this->commit_new_factor_ids.size() > GATHER_ITERS * GATHER_FACTORS_PER_ITER) {
// 			uniform_int_distribution<int> remove_distribution(0, this->commit_new_factor_ids.size()-1);
// 			int remove_index = remove_distribution(generator);
// 			this->commit_new_factor_ids.erase(this->commit_new_factor_ids.begin() + remove_index);
// 		}

// 		this->state = COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW;
// 		this->state_iter = 0;
// 	}
// }
