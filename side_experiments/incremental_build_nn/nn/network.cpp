#include "network.h"

using namespace std;

const int NETWORK_INCREMENT_HIDDEN_SIZE = 10;

const int NETWORK_INPUT_MIN_IMPACT = 0.1;

Network::Network(int input_size) {

}

Network::Network(Network* original) {

}

Network::Network(ifstream& input_file) {

}

Network::~Network() {

}

void Network::activate(vector<vector<double>>& input_vals) {

}

void Network::backprop(double error) {

}

void Network::increment_side(int input_size) {

}

void Network::increment_above(int input_size) {

}

/**
 * TODO:
 * - finalize from outside
 */
void Network::finalize() {
	for (int i_index = (int)this->inputs.back().size(); i_index >= 0; i_index--) {
		double sum_impact = 0.0;
		for (int n_index = 0; n_index < NETWORK_INCREMENT_HIDDEN_SIZE; n_index++) {
			/**
			 * - if NETWORK_INCREMENT_TYPE_SIDE, then only 1 layer
			 * - if NETWORK_INCREMENT_TYPE_ABOVE, then last input layer is new
			 */
			sum_impact += abs(this->hiddens.back()->weights[n_index].back()[i_index]);
		}
		if (sum_impact < NETWORK_INPUT_MIN_IMPACT) {

		}
	}
	/**
	 * TODO: clean if all new inputs removed
	 */
}

void Network::save(ofstream& output_file) {

}
