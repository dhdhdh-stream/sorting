#include "factor.h"

using namespace std;



void Factor::link(Solution* parent_solution) {
	if (this->input_scope_contexts.size() == 0) {
		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			Scope* scope = this->inputs[i_index].first.first.back();
			AbstractNode* node = scope->nodes[this->inputs[i_index].first.second.back()];
			switch (node->type) {
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)node;

					bool is_existing = false;
					for (int ii_index = 0; ii_index < (int)branch_node->input_scope_contexts.size(); ii_index++) {
						if (branch_node->input_scope_contexts[ii_index] == this->inputs[i_index].first.first
								&& branch_node->input_node_context_ids[ii_index] == this->inputs[i_index].first.second) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						branch_node->input_scope_contexts.push_back(this->inputs[i_index].first.first);
						branch_node->input_node_context_ids.push_back(this->inputs[i_index].first.second);
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node;

					if (this->inputs[i_index].second.first == -1) {
						bool is_existing = false;
						for (int ii_index = 0; ii_index < (int)input_action_node->input_scope_contexts.size(); ii_index++) {
							if (obs_node->input_scope_contexts[ii_index] == this->inputs[i_index].first.first
									&& obs_node->input_node_context_ids[ii_index] == this->inputs[i_index].first.second
									&& obs_node->input_obs_indexes[ii_index] == this->inputs[i_index].second.second) {
								is_existing = true;
								break;
							}
						}
						if (!is_existing) {
							obs_node->input_scope_contexts.push_back(this->inputs[i_index].first.first);
							obs_node->input_node_context_ids.push_back(this->inputs[i_index].first.second);
							obs_node->input_obs_indexes.push_back(this->inputs[i_index].second.second);
						}
					} else {
						Factor* factor = obs_node->factors[this->inputs[i_index].second.first];

						factor->link(parent_solution);

						bool is_existing = false;
						for (int ii_index = 0; ii_index < (int)factor->input_scope_contexts.size(); ii_index++) {
							if (factor->input_scope_contexts[ii_index] == this->inputs[i_index].first.first
									&& factor->input_node_context_ids[ii_index] == this->inputs[i_index].first.second) {
								is_existing = true;
								break;
							}
						}
						if (!is_existing) {
							factor->input_scope_contexts.push_back(this->inputs[i_index].first.first);
							factor->input_node_context_ids.push_back(this->inputs[i_index].first.second);
						}
					}
				}
				break;
			}
		}
	}
}
