#include "pass_through_network.h"

using namespace std;

PassThroughNetwork::PassThroughNetwork(int front_state_index,
									   int back_state_index) {
	this->front_state_index = front_state_index;
	this->back_state_index = back_state_index;
}

PassThroughNetwork::PassThroughNetwork(PassThroughNetwork* original) {
	this->front_state_index = original->front_state_index;
	this->back_state_index = original->back_state_index;
}

PassThroughNetwork::PassThroughNetwork(std::ifstream& input_file) {
	string front_state_index_line;
	getline(input_file, front_state_index_line);
	this->front_state_index = stoi(front_state_index_line);

	string back_state_index_line;
	getline(input_file, back_state_index_line);
	this->back_state_index = stoi(back_state_index_line);
}

void PassThroughNetwork::save(std::ofstream& output_file) {
	output_file << this->front_state_index << endl;
	output_file << this->back_state_index << endl;
}
