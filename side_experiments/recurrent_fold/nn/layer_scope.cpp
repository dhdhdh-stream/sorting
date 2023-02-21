#include "layer.h"

#include "utilities.h"

using namespace std;

void Layer::hidden_add_input() {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][0].push_back((randuni()-0.5)*0.02);
		this->weight_updates[n_index][0].push_back(0.0);
	}
}

void Layer::hidden_remove_input(int index) {
	for (int n_index = 0; n_index < (int)this->acti_vals.size(); n_index++) {
		this->weights[n_index][0].erase(this->weights[n_index][0].begin()+index);
		this->weight_updates[n_index][0].erase(this->weight_updates[n_index][0].begin()+index);

	}
}
