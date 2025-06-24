#include "solution_helpers.h"

#include <algorithm>

#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"

using namespace std;

const double MIN_PCC = 0.05;

void select_reward_signal(Scope* scope,
						  Input& reward_signal) {
	double curr_pcc = 0.0;
	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_OBS) {
			ObsNode* obs_node = (ObsNode*)it->second;
			for (int f_index = 0; f_index < (int)obs_node->factors.size(); f_index++) {
				Factor* factor = obs_node->factors[f_index];
				if (factor->pcc > curr_pcc && factor->pcc >= MIN_PCC) {
					uniform_int_distribution<int> select_distribution(
						0, factor->num_success + 1 + factor->num_failure + 1 - 1);
					if (select_distribution(generator) < factor->num_success + 1) {
						curr_pcc = factor->pcc;

						Input input;
						input.scope_context = {scope};
						input.node_context = {it->first};
						input.factor_index = f_index;
						input.obs_index = -1;
						reward_signal = input;
					}
				}
			}
		}
	}
}
