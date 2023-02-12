#include "fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

void Fold::inner_scope_input_end() {
	if (this->sum_error/10000 < 0.01
			|| (this->inner_input_input_networks.size() > 0
				&& this->inner_input_input_sizes.back() == (this->curr_s_input_sizes[this->inner_input_input_layer.back()]
					+ this->curr_scope_sizes[this->inner_input_input_layer.back()]))) {
		delete this->curr_inner_input_network;
		this->curr_inner_input_network = this->test_inner_input_network;
		this->test_inner_input_network = NULL;

		if (this->curr_inner_input_network->subfold_index == (int)this->curr_scope_sizes.size()-2) {
			if (!this->is_inner_scope[this->finished_steps.size()]) {
				this->curr_s_input_sizes.push_back(0);
				// obs_size always 1 for sorting
				this->curr_scope_sizes.push_back(1);
			} else {
				this->curr_s_input_sizes.push_back(solution->scope_dictionary[this->existing_scope_ids[this->finished_steps.size()]]->num_inputs);
				this->curr_scope_sizes.push_back(solution->scope_dictionary[this->existing_scope_ids[this->finished_steps.size()]]->num_outputs);
			}

			this->curr_fold->add_scope(this->curr_scope_sizes.back());
			this->curr_fold->fold_index++;
			this->curr_fold->migrate_weights();
			for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
				if (this->is_inner_scope[f_index]) {
					this->curr_input_folds[f_index]->add_scope(this->curr_scope_sizes.back());
					this->curr_input_folds[f_index]->fold_index++;
					this->curr_input_folds[f_index]->migrate_weights();
				}
			}
			this->curr_end_fold->add_scope(this->curr_scope_sizes.back());
			this->curr_end_fold->fold_index++;
			this->curr_end_fold->migrate_weights();

			this->curr_score_network = new FoldNetwork(1,
													   this->curr_s_input_sizes[0],
													   this->curr_scope_sizes,
													   50);

			cout << "INNER_SCOPE_INPUT success" << endl;
			cout << "ending INNER_SCOPE_INPUT" << endl;
			cout << "beginning STATE_SCORE" << endl;

			this->state = STATE_SCORE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			this->test_inner_input_network = new FoldNetwork(this->curr_inner_input_network);
			this->test_inner_input_network->subfold_index++;

			// start from size 1 if have previous
			if (this->inner_input_input_networks.size() > 0) {
				this->inner_input_input_layer.push_back(this->test_inner_input_network->subfold_index);
				this->inner_input_input_sizes.push_back(1);
				this->inner_input_input_networks.push_back(new FoldNetwork(1,
																		   this->curr_s_input_sizes[this->test_inner_input_network->subfold_index],
																		   vector<int>{this->curr_scope_sizes[this->test_inner_input_network->subfold_index]},
																		   50));
				// add to curr_s_input_sizes permanently 1 at a time
				this->curr_s_input_sizes[this->test_inner_input_network->subfold_index+1]++;
			}

			this->test_inner_input_network->set_s_input_size(this->curr_s_input_sizes[
				this->test_inner_input_network->subfold_index+1]);

			cout << "INNER_SCOPE_INPUT success" << endl;
			cout << "ending INNER_SCOPE_INPUT" << endl;
			cout << "beginning INNER_SCOPE_INPUT" << endl;

			this->state = STATE_INNER_SCOPE_INPUT;
			this->state_iter = 0;
			this->sum_error = 0.0;
			this->new_state_factor = 25;
		}
	} else {
		delete this->test_inner_input_network;
		this->test_inner_input_network = new FoldNetwork(this->curr_inner_input_network);
		this->test_inner_input_network->subfold_index++;

		if (this->inner_input_input_layer.size() > 0
				&& this->inner_input_input_layer.back() == this->test_inner_input_network->subfold_index) {
			this->inner_input_input_sizes.back()++;
			delete this->inner_input_input_networks.back();
			this->inner_input_input_networks.pop_back();
		} else {
			this->inner_input_input_layer.push_back(this->test_inner_input_network->subfold_index);
			this->inner_input_input_sizes.push_back(1);
		}
		this->inner_input_input_networks.push_back(new FoldNetwork(this->inner_input_input_sizes.back(),
																   this->curr_s_input_sizes[this->test_inner_input_network->subfold_index],
																   vector<int>{this->curr_scope_sizes[this->test_inner_input_network->subfold_index]},
																   50));
		// add to curr_s_input_sizes permanently 1 at a time
		this->curr_s_input_sizes[this->test_inner_input_network->subfold_index+1]++;
		this->test_inner_input_network->set_s_input_size(this->curr_s_input_sizes[
			this->test_inner_input_network->subfold_index+1]);

		cout << "INNER_SCOPE_INPUT fail" << endl;
		cout << "ending INNER_SCOPE_INPUT" << endl;
		cout << "beginning INNER_SCOPE_INPUT" << endl;

		this->state = STATE_INNER_SCOPE_INPUT;
		this->state_iter = 0;
		this->sum_error = 0.0;
		this->new_state_factor = 25;
	}
}
