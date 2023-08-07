#include "layer.h"

using namespace std;

void Layer::score_hidden_finalize_new_state(int new_state_index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][0].push_back(this->weights[n_index][1][new_state_index]);
		this->weight_updates[n_index][0].push_back(0.0);
	}
}

void Layer::score_hidden_cleanup_new_state() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][1].clear();
		this->weight_updates[n_index][1].clear();
	}
}

void Layer::score_hidden_add_state() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][0].push_back(0.0);
		this->weight_updates[n_index][0].push_back(0.0);
	}
}
