#include "pattern.h"

#include "fixed_point.h"
#include "globals.h"
#include "world.h"

using namespace std;

Pattern::Pattern() {
	// do nothing
}

Pattern::Pattern(ifstream& input_file) {
	string num_fixed_points_line;
	getline(input_file, num_fixed_points_line);
	int num_fixed_points = stoi(num_fixed_points_line);
	for (int p_index = 0; p_index < num_fixed_points; p_index++) {
		string id_line;
		getline(input_file, id_line);
		this->fixed_points.push_back(world->fixed_points[stoi(id_line)]);
	}

	string success_likelihood_line;
	getline(input_file, success_likelihood_line);
	this->success_likelihood = stod(success_likelihood_line);
}

void Pattern::save(ofstream& output_file) {
	output_file << this->fixed_points.size() << endl;
	for (int p_index = 0; p_index < (int)this->fixed_points.size(); p_index++) {
		output_file << this->fixed_points[p_index]->id << endl;
	}

	output_file << this->success_likelihood << endl;
}
