#include "loop_init_test_node.h"

#include <iostream>

using namespace std;

LoopInitTestNode::LoopInitTestNode(vector<int> initial_scope_sizes,
								   FoldLoopInitNetwork* original_init,
								   FoldLoopNetwork* original_loop,
								   FoldNetwork* original_combine,
								   int obs_size) {
	this->obs_size = obs_size;

	this->curr_scope_sizes = this->initial_scope_sizes;
	this->curr_init = this->original_init;
	this->curr_loop = this->original_loop;
	this->curr_combine = this->original_combine;

	this->test_init = NULL;
	this->test_loop = NULL;
	this->test_combine = NULL;

	this->score_network = new ScoreNetwork(initial_scope_sizes,
										   this->obs_size);

	this->state_network = NULL;

	this->state = STATE_LEARN_SCORE;
	this->state_iter = 0;
	this->sum_error = 0.0;
	this->best_sum_error = -1.0;

	this->new_scope_size = 0;
}

LoopInitTestNode::~LoopInitTestNode() {
	// do nothing

	// all networks will be taken or cleared by increment()
}

void LoopInitTestNode::activate(vector<vector<double>>& state_vals,
								vector<bool>& scopes_on,
								vector<double>& obs) {
	this->score_network->activate(state_vals,
								  obs);
	state_vals[0][0] = this->score_network->output->acti_vals[0];

	if (this->state == STATE_LEARN_SCORE) {
		// do nothing
	} else if (this->state == STATE_JUST_SCORE_LEARN
			|| this->state == STATE_JUST_SCORE_MEASURE
			|| this->state == STATE_JUST_SCORE_TUNE) {
		state_vals.erase(state_vals.begin()+1, state_vals.end());
		// no need for scopes_on anymore
	} else {
		if (this->new_scope_size > 0) {
			state_vals.push_back(vector<double>(this->new_scope_size));
			scopes_on.push_back(true);
		}

		this->state_network->activate(state_vals,
									  scopes_on,
									  obs);
		for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
			state_vals.back()[st_index] = this->state_network->output->acti_vals[st_index];
		}

		if (this->state == STATE_COMPRESS_TUNE) {
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
		} else if ((this->state == STATE_COMPRESS_LEARN
				|| this->state == STATE_COMPRESS_MEASURE)
				&& this->test_compress_sizes > 1) {
			// activate all but last
			for (int c_index = 0; c_index < (int)this->compression_networks.size()-1; c_index++) {
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

			this->test_compression_network->activate(state_vals,
													 scopes_on);

			int sum_scope_sizes = 0;
			for (int s_index = 0; s_index < this->test_compress_num_scopes; s_index++) {
				sum_scope_sizes += (int)state_vals.back().size();
				state_vals.pop_back();
				scopes_on.pop_back();
			}
			state_vals.push_back(vector<double>(sum_scope_sizes - this->test_compress_sizes));
			scopes_on.push_back(true);

			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->test_compression_network->output->acti_vals[st_index];
			}
		} else if (this->state == STATE_CAN_COMPRESS_LEARN
				|| this->state == STATE_CAN_COMPRESS_MEASURE
				|| this->state == STATE_COMPRESS_LEARN
				|| this->state == STATE_COMPRESS_MEASURE) {
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

			this->test_compression_network->activate(state_vals,
													 scopes_on);

			int sum_scope_sizes = 0;
			for (int s_index = 0; s_index < this->test_compress_num_scopes; s_index++) {
				sum_scope_sizes += (int)state_vals.back().size();
				state_vals.pop_back();
				scopes_on.pop_back();
			}
			state_vals.push_back(vector<double>(sum_scope_sizes - this->test_compress_sizes));
			scopes_on.push_back(true);

			for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
				state_vals.back()[st_index] = this->test_compression_network->output->acti_vals[st_index];
			}
		}
	}
}

void LoopInitTestNode::loop_init(vector<vector<double>>& pre_loop_flat_vals,
								 vector<double>& loop_state,
								 vector<vector<double>>& outer_state_vals) {
	if (this->state == STATE_LEARN_SCORE
			|| this->state == STATE_JUST_SCORE_TUNE) {
		// do nothing
	} else if (this->state == STATE_LOCAL_SCOPE_TUNE
			|| this->state == STATE_COMPRESS_TUNE
			|| this->state == STATE_ADD_SCOPE_TUNE) {
		this->curr_init->activate(pre_loop_flat_vals,
								  outer_state_vals);
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state[s_index] = this->curr_init->output->acti_vals[s_index];
		}
	} else {
		this->test_init->activate(pre_loop_flat_vals,
								  outer_state_vals);
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state[s_index] = this->test_init->output->acti_vals[s_index];
		}
	}
}

void LoopInitTestNode::loop_iter(vector<vecotr<double>>& pre_loop_flat_vals,
								 vector<vector<double>>& loop_flat_vals,
								 vector<double>& loop_state,
								 vector<vector<double>>& outer_state_vals,
								 vector<AbstractNetworkHistory*>& network_historys) {
	if (this->state == STATE_LEARN_SCORE
			|| this->state == STATE_JUST_SCORE_TUNE) {
		// do nothing
	} else if (this->state == STATE_LOCAL_SCOPE_TUNE
			|| this->state == STATE_COMPRESS_TUNE
			|| this->state == STATE_ADD_SCOPE_TUNE) {
		this->curr_loop->activate(pre_loop_flat_vals,
								  loop_flat_vals,
								  loop_state,
								  outer_state_vals,
								  network_historys);
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state[s_index] = this->curr_loop->output->acti_vals[s_index];
		}
	} else {
		this->test_loop->activate(pre_loop_flat_vals,
								  loop_flat_vals,
								  loop_state,
								  outer_state_vals,
								  network_historys);
		for (int s_index = 0; s_index < (int)loop_state.size(); s_index++) {
			loop_state[s_index] = this->test_loop->output->acti_vals[s_index];
		}
	}
}

void LoopInitTestNode::process(vector<vector<double>>& combine_inputs,
							   vector<vector<double>>& outer_state_vals,
							   double target_val,
							   vector<Node*>& nodes) {

}

void LoopInitTestNode::increment() {

}
