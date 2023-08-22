#include "state_network.h"

using namespace std;

void StateNetwork::construct() {

}

StateNetwork::StateNetwork(int obs_size,
						   int hidden_size) {

}

StateNetwork::StateNetwork(StateNetwork* original) {

}

StateNetwork::StateNetwork(ifstream& input_file) {

}

StateNetwork::~StateNetwork() {

}

void StateNetwork::activate() {

}

void StateNetwork::activate(StateNetworkHistory* history) {

}

void StateNetwork::backprop(double target_max_update) {

}

void StateNetwork::backprop(double target_max_update,
							vector<double>& obs_history,
							StateNetworkHistory* history) {

}

void StateNetwork::lasso_backprop(double target_max_update) {

}

void StateNetwork::lasso_backprop(double target_max_update,
								  vector<double>& obs_history,
								  StateNetworkHistory* history) {

}

void StateNetwork::backprop_errors_with_no_weight_change() {

}

void StateNetwork::backprop_errors_with_no_weight_change(
		vector<double>& obs_history,
		StateNetworkHistory* history) {

}

void StateNetwork::save(ofstream& output_file) {

}

StateNetworkHistory::StateNetworkHistory(StateNetwork* network) {

}

void StateNetworkHistory::save_weights() {

}

void StateNetworkHistory::reset_weights() {

}
