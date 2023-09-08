#include "exit_node.h"

using namespace std;



void ExitNode::activate(vector<ContextLayer>& context) {
	for (int m_index = 0; m_index < (int)this->weight_mods.size(); m_index++) {
		if (this->target_layers[m_index] == 0) {
			context[0].state_weights[this->target_indexes[m_index]] *= this->weight_mods[m_index];
		} else {
			int c_index = context.size() - this->scope_context.size() + this->target_layers[m_index];
			context[c_index].state_weights[this->target_indexes[m_index]] *= this->weight_mods[m_index];
		}
	}
}


