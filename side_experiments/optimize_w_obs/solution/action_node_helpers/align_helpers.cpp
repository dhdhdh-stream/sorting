#include "action_node.h"

#include "globals.h"
#include "sample.h"
#include "scope.h"

using namespace std;

void ActionNode::align_activate(AbstractNode*& curr_node,
								Alignment& alignment,
								vector<ContextLayer>& context) {
	int closest_match = -1;
	for (int a_index = alignment.step_nodes.size()-1; a_index < (int)alignment.sample->actions.size(); a_index++) {
		if (alignment.sample->actions[a_index] == this->action) {
			closest_match = a_index;
			break;
		}
	}

	pair<vector<int>,vector<int>> local_context;
	for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
		local_context.first.push_back(context[l_index].scope_id);
		local_context.second.push_back(context[l_index].node_id);
	}
	local_context.first.push_back(this->parent->id);
	local_context.second.push_back(this->id);

	if (closest_match == -1) {
		alignment.step_nodes.back().push_back(local_context);
	} else {
		int match_distance = closest_match - (alignment.step_nodes.size()-1);
		uniform_int_distribution<int> select_distribution(0, (match_distance+1)/2);
		if (select_distribution(generator) == 0) {
			for (int d_index = 0; d_index < match_distance; d_index++) {
				alignment.step_nodes.push_back(vector<pair<vector<int>,vector<int>>>());
			}
			alignment.step_nodes.back().push_back(local_context);

			if (this->input_scope_context_ids.size() > 0) {
				vector<double> obs = alignment.sample->obs[alignment.step_nodes.size()-1];

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
						if (this->input_obs_indexes[i_index] == -1) {
							context[context.size() - this->input_scope_context_ids[i_index].size()]
								.obs_history[{{this->input_scope_context_ids[i_index],
									this->input_node_context_ids[i_index]}, -1}] = 1.0;
						} else {
							context[context.size() - this->input_scope_context_ids[i_index].size()]
								.obs_history[{{this->input_scope_context_ids[i_index],
									this->input_node_context_ids[i_index]}, this->input_obs_indexes[i_index]}] = obs[this->input_obs_indexes[i_index]];
						}
					}
				}
			}

			alignment.step_nodes.push_back(vector<pair<vector<int>,vector<int>>>());
		} else {
			alignment.step_nodes.back().push_back(local_context);
		}
	}

	curr_node = this->next_node;
}
