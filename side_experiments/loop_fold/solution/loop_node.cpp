#include "loop_node.h"

#include <iostream>

using namespace std;

const double TARGET_MAX_UPDATE = 0.001;

LoopNode::LoopNode(string id,
				   Network* init_network,
				   vector<AbstractNode*> nodes,
				   vector<int> inner_num_scopes,
				   vector<int> inner_sizes,
				   Network* loop_network) {

}

LoopNode::LoopNode(string id,
				   ifstream& input_file) {

}

LoopNode::~LoopNode() {

}

void LoopNode::activate(vector<vector<double>>& state_vals,
						vector<bool>& scopes_on,
						vector<double>& obs) {

}

void LoopNode::activate(vector<vector<double>>& state_vals,
						vector<bool>& scopes_on,
						vector<double>& obs,
						vector<AbstractNetworkHistory*>& network_historys) {

}

void LoopNode::backprop(double target_val,
						vector<vector<double>>& state_errors) {

}

void LoopNode::backprop(double target_val,
						vector<vector<double>>& state_errors,
						vector<AbstractNetworkHistory*>& network_historys) {

}

void LoopNode::backprop_zero_train(AbstractNode* original,
								   double& sum_error) {

}

void LoopNode::activate_state(vector<vector<double>>& state_vals,
							  vector<bool>& scopes_on,
							  vector<double>& obs) {

}

void LoopNode::backprop_zero_train_state(AbstractNode* original,
										 double& sum_error) {

}

void LoopNode::get_scope_sizes(vector<int>& scope_sizes) {

}

void LoopNode::save(ofstream& output_file) {

}

void LoopNode::activate(int num_iters,
						vector<vector<vector<double>>>& iter_loop_flat_vals,
						vector<vector<double>>& state_vals,
						vector<bool>& scopes_on,
						vector<double>& obs) {
	vector<double> init_input;
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			init_input.push_back(state_vals[sc_index][st_index]);
		}
	}
	this->init_network->activate(init_input);

	vector<double> loop_state(this->loop_state_size);
	for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
		loop_state[s_index] = this->init_network->output->acti_vals[s_index];
	}

	for (int iter_index = 0; iter_index < num_iters; iter_index++) {
		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			this->nodes[n_index]->activate(state_vals,
										   scopes_on,
										   iter_loop_flat_vals[iter_index][n_index]);
		}

		vector<double> loop_init;
		for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
			for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
				loop_init.push_back(state_vals[sc_index][st_index]);
			}
		}
		for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
			loop_init.push_back(loop_state[s_index]);
		}
		this->loop_network->activate(loop_init);
		for (int s_index = 0; s_index < this->loop_state_size; s_index++) {
			loop_state[s_index] = this->loop_network->output->acti_vals[s_index];
		}

		for (int sc_index = 0; sc_index < this->inner_num_scopes; sc_index++) {
			state_vals.pop_back();
			scopes_on.pop_back();
		}
	}

	if (just_score) {
		state_vals.erase(state_vals.begin()+1, state_vals.end());
		scopes_on.erase(scopes_on.begin()+1, scopes_on.end());
	} else {
		this->state_network->activate(state_vals,
									  scopes_on,
									  loop_state);

		if (!update_existing_scope) {
			state_vals.push_back(vector<double>(this->new_scope_size));
			scopes_on.push_back(true);
		}

		for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
			state_vals.back()[st_index] = this->state_network->output->acti_vals[st_index];
		}

		for (int c_index = 0; c_index < (int)this->compression_networks.size(); c_index++) {
			this->compression_networks[c_index]->activate(state_vals,
														  scopes_on);

			int sum_scope_sizes = 0;
			for (int s_index = 0; s_index < this->compress_num_scopes[c_index]; s_index++) {
				sum_scope_sizes += (int)state_vals.back().size();
				state_vals.pop_back();
				scopes_on.pop_back();
			}
			state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_sizes[c_index]));
			scopes_on.push_back(true);

			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
			}
		}
	}
}

void LoopNode::activate(int num_iters,
						vector<vector<vector<double>>>& iter_loop_flat_vals,
						vector<vector<double>>& state_vals,
						vector<bool>& scopes_on,
						vector<double>& obs,
						vector<AbstractNetworkHistory*>& network_historys) {

}
