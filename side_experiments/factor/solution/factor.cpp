

for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
				Scope* scope = this->existing_inputs[i_index].first.first.back();
				ObsNode* obs_node = (ObsNode*)scope->nodes[this->existing_inputs[i_index].first.second.back()];
				if (this->existing_inputs[i_index].second.first == -1) {
					bool is_existing = false;
					for (int ii_index = 0; ii_index < (int)input_action_node->input_scope_contexts.size(); ii_index++) {
						if (obs_node->input_scope_contexts[ii_index] == this->existing_inputs[i_index].first.first
								&& obs_node->input_node_context_ids[ii_index] == this->existing_inputs[i_index].first.second
								&& obs_node->input_obs_indexes[ii_index] == this->existing_inputs[i_index].second.second) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						obs_node->input_scope_contexts.push_back(this->existing_inputs[i_index].first.first);
						obs_node->input_node_context_ids.push_back(this->existing_inputs[i_index].first.second);
						obs_node->input_obs_indexes.push_back(this->existing_inputs[i_index].second.second);
					}
				} else {

				}
			}