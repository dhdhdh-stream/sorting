#include "transition.h"

using namespace std;

Transition::Transition() {
	// do nothing
}

Transition::Transition(ifstream& input_file) {
	string num_moves_line;
	getline(input_file, num_moves_line);
	int num_moves = stoi(num_moves_line);
	for (int m_index = 0; m_index < num_moves; m_index++) {
		string move_line;
		getline(input_file, move_line);
		this->moves.push_back(stoi(move_line));
	}

	string likelihood_calculated_line;
	getline(input_file, likelihood_calculated_line);
	this->likelihood_calculated = stoi(likelihood_calculated_line);

	string success_likelihood_line;
	getline(input_file, success_likelihood_line);
	this->success_likelihood = stod(success_likelihood_line);
}

void Transition::save(ofstream& output_file) {
	output_file << this->moves.size() << endl;
	for (int m_index = 0; m_index < (int)this->moves.size(); m_index++) {
		output_file << this->moves[m_index] << endl;
	}

	output_file << this->likelihood_calculated << endl;
	output_file << this->success_likelihood << endl;
}
