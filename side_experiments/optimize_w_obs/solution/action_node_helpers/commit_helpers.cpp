#include "action_node.h"

#include "globals.h"
#include "potential_commit.h"
#include "problem.h"
#include "solution.h"

using namespace std;

void ActionNode::commit_gather_activate(AbstractNode*& curr_node,
										Problem* problem,
										vector<ContextLayer>& context,
										RunHelper& run_helper,
										int& node_count,
										AbstractNode*& potential_node_context,
										bool& potential_is_branch) {
	problem->perform_action(this->action);

	if (this->input_scope_context_ids.size() > 0) {
		vector<double> obs = problem->get_observations();

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
				context[context.size() - this->input_scope_context_ids[i_index].size()]
					.obs_history[{{this->input_scope_context_ids[i_index],
						this->input_node_context_ids[i_index]}, this->input_obs_indexes[i_index]}] = obs[this->input_obs_indexes[i_index]];
			}
		}
	}

	curr_node = this->next_node;

	uniform_int_distribution<int> select_distribution(0, node_count);
	node_count++;
	if (select_distribution(generator) == 0) {
		potential_node_context = this;
		potential_is_branch = false;
	}
}

void ActionNode::commit_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 PotentialCommit* potential_commit) {
	problem->perform_action(this->action);

	if (this->input_scope_context_ids.size() > 0) {
		vector<double> obs = problem->get_observations();

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
				context[context.size() - this->input_scope_context_ids[i_index].size()]
					.obs_history[{{this->input_scope_context_ids[i_index],
						this->input_node_context_ids[i_index]}, this->input_obs_indexes[i_index]}] = obs[this->input_obs_indexes[i_index]];
			}
		}
	}

	curr_node = this->next_node;

	if (potential_commit->node_context == this) {
		potential_commit->activate(curr_node,
								   problem,
								   context,
								   run_helper);
	}
}
