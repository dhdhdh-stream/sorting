#include "action_node.h"

#include "globals.h"
#include "problem.h"
#include "solution.h"

using namespace std;

void ActionNode::set_activate(AbstractNode*& curr_node,
							  Problem* problem,
							  vector<ContextLayer>& context,
							  RunHelper& run_helper,
							  bool& experiment_seen,
							  map<pair<AbstractNode*,bool>, int>& nodes_seen) {
	problem->perform_action(this->action);

	if (this->input_scope_contexts.size() > 0) {
		vector<double> obs = problem->get_observations();

		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			bool match_context = false;
			if (context.size() >= this->input_scope_contexts[i_index].size()) {
				match_context = true;
				for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size()-1; l_index++) {
					int context_index = context.size() - this->input_scope_contexts[i_index].size() + l_index;
					if (context[context_index].scope != this->input_scope_contexts[i_index][l_index]
							|| context[context_index].node_id != this->input_node_context_ids[i_index][l_index]) {
						match_context = false;
						break;
					}
				}
			}

			if (match_context) {
				context[context.size() - this->input_scope_contexts[i_index].size()]
					.obs_history[{{this->input_scope_contexts[i_index],
						this->input_node_context_ids[i_index]}, this->input_obs_indexes[i_index]}] = obs[this->input_obs_indexes[i_index]];
			}
		}
	}

	curr_node = this->next_node;

	if (this->is_experiment) {
		experiment_seen = true;
	}

	if (!experiment_seen) {
		map<pair<AbstractNode*,bool>, int>::iterator it = nodes_seen.find({this, false});
		if (it == nodes_seen.end()) {
			nodes_seen[{this, false}] = 1;
		} else {
			it->second++;
		}
	}
}
