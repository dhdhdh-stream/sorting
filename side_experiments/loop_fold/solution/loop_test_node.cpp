#include "loop_test_node.h"

#include <iostream>

using namespace std;

LoopTestNode::LoopTestNode(vector<int> initial_inner_scope_sizes,
						   Network* init_network,
						   FoldLoopNetwork* original_loop,
						   FoldCombineNetwork* original_combine,
						   int obs_size) {
	this->obs_size = obs_size;

	this->init_network = init_network;

	this->curr_inner_scope_sizes = initial_inner_scope_sizes;
	this->curr_loop = original_loop;
	this->curr_combine = original_combine;

	this->test_loop = NULL;
	this->test_combine = NULL;

	this->score_network = new ScoreNetwork(initial_inner_scope_sizes,
										   this->obs_size);

	this->state_network = NULL;

	this->state = STATE_LEARN_SCORE;
	this->state_iter = 0;
	this->sum_error = 0.0;
	this->best_sum_error = -1.0;

	this->new_scope_size = 0;
}

LoopTestNode::~LoopTestNode() {
	// do nothing

	// all networks will be taken or cleared by increment()
}

void LoopTestNode::loop_init(vector<vector<double>>& outer_state_vals,
							 vector<double>& loop_state) {

}

void LoopTestNode::activate(vector<vector<double>>& inner_state_vals,
							vector<bool>& scopes_on,
							vector<double>& obs,
							vector<AbstractNetworkHistory*>& network_historys) {

}

void LoopTestNode::loop_iter(vector<vector<double>>& loop_flat_vals,
							 vector<double>& loop_state,
							 vector<vector<double>>& outer_state_vals,
							 vector<vector<double>>& inner_state_vals,
							 vector<AbstractNetworkHistory*>& network_historys) {

}

void LoopTestNode::process(vector<double>& loop_state,
						   vector<vector<double>>& post_loop_flat_vals,
						   vector<vector<double>>& outer_state_vals,
						   double target_val,
						   vector<Node*>& nodes,
						   vector<AbstractNetworkHistory*>& network_historys) {

}

void LoopTestNode::increment() {

}
