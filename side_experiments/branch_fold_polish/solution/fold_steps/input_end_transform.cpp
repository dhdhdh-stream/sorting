#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::input_end() {
	if (this->sum_error/10000 < 0.001
			|| (this->input_networks.size() > 0
				&& this->input_sizes.back() == (this->curr_s_input_sizes[this->input_layer.back()]
					this->curr_scope_sizes[this->input_layer.back()]))) {
		delete this->curr_score_network;
		this->curr_score_network = this->test_score_network;
		this->test_score_network = NULL;

		if (this->curr_compress_new_size > 0) {
			// this->curr_compress_num_layers > 0
			delete this->curr_compress_network;
			this->curr_compress_network = this->test_compress_network;
			this->test_compress_network = NULL;
		}

		if (this->curr_score_network->subfold_index == (int)this->curr_scope_sizes.size()-2) {
			add_finished_step();
		} else {
			this->test_score_network = new FoldNetwork(this->curr_score_network);
			this->test_score_network->subfold_index++;

			if (this->curr_compress_new_size > 0) {
				this->test_compress_network = new FoldNetwork(this->curr_compress_network);
				this->test_compress_network->subfold_index++;
			}

			// start from size 1 if have previous
			if (this->input_networks.size() > 0) {
				this->input_layer.push_back(this->test_score_network->subfold_index);
				this->input_sizes.push_back(1);
				this->input_networks.push_back(new FoldNetwork(1,
															   this->curr_s_input_sizes[this->test_score_network->subfold_index],
															   this->curr_scope_sizes[this->test_score_network->subfold_index],
															   20));
				// add to curr_s_input_sizes permanently 1 at a time
				this->curr_s_input_sizes[this->test_score_network->subfold_index+1]++;
			}

			this->test_score_network->set_s_input_size(this->curr_s_input_sizes[
				this->test_score_network->subfold_index+1]);
			if (this->curr_compress_new_size > 0) {
				this->test_compress_network->set_s_input_size(this->curr_s_input_sizes[
					this->test_score_network->subfold_index+1]);
			}

			this->last_state = STATE_INPUT;
			this->state = STATE_INPUT;
			this->state_iter = 0;
			this->sum_error = 0.0;
			this->new_state_factor = 25;
		}
	} else {
		delete this->test_score_network;
		this->test_score_network = new FoldNetwork(this->curr_score_network);
		this->test_score_network->subfold_index++;

		if (this->curr_compress_new_size > 0) {
			delete this->test_compress_network;
			this->test_compress_network = new FoldNetwork(this->curr_compress_network);
			this->test_compress_network->subfold_index++;
		}

		if (this->input_layer.size() > 0
				&& this->input_layer.back() == this->test_score_network->subfold_index) {
			this->input_sizes.back()++;
			delete this->input_networks.back();
			this->input_networks.pop_back();
		} else {
			this->input_layer.push_back(this->test_score_network->subfold_index);
			this->input_sizes.push_back(1);
		}
		this->input_networks.push_back(new FoldNetwork(this->input_sizes.back(),
													   this->curr_s_input_sizes[this->test_score_network->subfold_index],
													   this->curr_scope_sizes[this->test_score_network->subfold_index],
													   20));
		// add to curr_s_input_sizes permanently 1 at a time
		this->curr_s_input_sizes[this->test_score_network->subfold_index+1]++;
		this->test_score_network->set_s_input_size(this->curr_s_input_sizes[
			this->test_score_network->subfold_index+1]);
		if (this->curr_compress_new_size > 0) {
			this->test_compress_network->set_s_input_size(this->curr_s_input_sizes[
				this->test_score_network->subfold_index+1]);
		}

		// no change to last_state
		this->state = STATE_INPUT;
		this->state_iter = 0;
		this->sum_error = 0.0;
		this->new_state_factor = 25;
	}
}
