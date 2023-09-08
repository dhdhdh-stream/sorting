#include "explore.h"

using namespace std;



void Explore::activate(vector<double>& flat_vals,
					   vector<ContextLayer>& context,
					   int& explore_exit_depth,
					   int& explore_exit_node_id,
					   RunHelper& run_helper,
					   ExploreHistory* history) {
	if (this->state == EXPLORE_STATE_SETUP) {
		vector<vector<double>*> state_vals(this->context_indexes.size()+1);
		state_vals[0] = context[0].state_vals;
		for (int c_index = 0; c_index < (int)this->context_indexes.size(); c_index++) {
			state_vals[1+c_index] = context[context.size()-this->context_indexes.size()+c_index].state_vals;
		}

		history->running_average_score_snapshot = run_helper.running_average_score;
		history->running_average_misguess_snapshot = run_helper.running_average_misguess;
		history->scale_factor_snapshot = run_helper.scale_factor;

		ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->score_network);
		this->score_network->activate(state_vals,
									  score_network_history);
		history->score_network_history = score_network_history;
		history->score_network_output = this->score_network->output->acti_vals[0];

		ScoreNetworkHistory* misguess_network_history = new ScoreNetworkHistory(this->misguess_network);
		this->misguess_network->activate(state_vals,
										 misguess_network_history);
		history->misguess_network_history = misguess_network_history;
		history->misguess_network_output = this->misguess_network->output->acti_vals[0];

		if (this->explore_node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->explore_node;
			explore_exit_depth = 0;
			explore_exit_node_id = action_node->next_node_id;
		} else {
			ScopeNode* scope_node = (ScopeNode*)this->explore_node;
			explore_exit_depth = 0;
			explore_exit_node_id = scope_node->next_node_id;
		}
	} else if (this->state == EXPLORE_STATE_EXPLORE) {

	} else if (this->state == EXPLORE_STATE_EXPERIMENT) {

	}
}


