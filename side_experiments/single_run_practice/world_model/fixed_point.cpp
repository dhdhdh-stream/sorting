#include "fixed_point.h"

#include "globals.h"
#include "world.h"

using namespace std;

void FixedPoint::save(ofstream& output_file) {
	output_file << this->val_average << endl;

	output_file << this->transitions.size() << endl;
	for (map<FixedPoint*, Transition>::iterator it = this->transitions.begin();
			it != this->transitions.end(); it++) {
		output_file << it->first->id << endl;

		it->second.save(output_file);
	}
}

void FixedPoint::load(ifstream& input_file) {
	string val_average_line;
	getline(input_file, val_average_line);
	this->val_average = stod(val_average_line);

	string num_transitions_line;
	getline(input_file, num_transitions_line);
	int num_transitions = stoi(num_transitions_line);
	for (int t_index = 0; t_index < num_transitions; t_index++) {
		string fixed_point_id_line;
		getline(input_file, fixed_point_id_line);
		FixedPoint* fixed_point = world->fixed_points[stoi(fixed_point_id_line)];

		Transition transition(input_file);

		this->transitions[fixed_point] = transition;
	}
}
